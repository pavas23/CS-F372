#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/msg.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>

#define PERMS 0644
#define ROWS 30
#define COLS 30
#define NUM_RESOURCES 21 // maximum number of files in graph database system

struct my_msgbuf{
    long operation_number; // setting this field as long, because long datatype is taken as mtype
    int sequence_number;
    char graph_file_name[200];
};

void *start_routine(void *arg){
    struct my_msgbuf *temp = (struct my_msgbuf *)arg;
    int sequence_number = temp->sequence_number / 1000;
    int operation_number = temp->sequence_number % 1000;

    struct my_msgbuf output; // buf object for sending the output to client

    // finding the name of semaphore corresponding to the file name user entered
    char graph_file_name[200];
    strcpy(graph_file_name, temp->graph_file_name);
    int semaphore_index = 0;
    if (strlen(graph_file_name) == 6){
        semaphore_index = graph_file_name[1] - '0';
    }
    else{
        semaphore_index = (graph_file_name[1] - '0') * 10 + (graph_file_name[2] - '0');
    }

    // this semaphore is for synchronization of shared memory between client write operation and thread read operation
    char semaphore_name[100];
    sprintf(semaphore_name, "%s%d", "S", sequence_number);
    sem_t *sem = sem_open(semaphore_name, 0); // initializing semaphore to 1

    // connecting to shared memory segement created by the client process
    key_t key;
    int shmid;
    int *shmptr;

    if ((key = ftok("client.c", sequence_number)) == -1){
        perror("Error in ftok");
        exit(-1);
    }

    sem_wait(sem); // this will wait until semaphore is released, i.e. once the client writes data to shared memory segment

    shmid = shmget(key, sizeof(int) * (ROWS + 1) * (COLS + 1), PERMS);
    shmptr = shmat(shmid, NULL, 0); // pointer to shared memory
    if (shmptr == (int *)-1){
        perror("Error in shmptr");
        exit(1);
    }

    int num_node_of_graph = shmptr[0];
    int adjacency_matrix[num_node_of_graph][num_node_of_graph];

    for (int i = 1; i < num_node_of_graph + 1; i++){
        for (int j = 1; j < num_node_of_graph + 1; j++){
            adjacency_matrix[i - 1][j - 1] = shmptr[i * num_node_of_graph + j];
        }
    }

    sem_post(sem); // releasing the lock

    // opening the named semaphore corresponding to writer semaphore for this particular file
    char semaphore_name_writer[100];
    sprintf(semaphore_name_writer, "%s%d", "W", semaphore_index);
    sem_t *sem_writer = sem_open(semaphore_name_writer, 0);

    // checking operation number
    if (operation_number == 1){
        // checking if file already exists
        if (access(graph_file_name, F_OK) != -1){
            // file already exists, return error
            // sending message back to client
            strcpy(output.graph_file_name, "File already exists");
            output.operation_number = 1000 + sequence_number;
            if (msgsnd(temp->operation_number, &output, sizeof(output), 0) == -1){
                perror("msgsnd");
            }
        }
        else{
            // critical section
            sem_wait(sem_writer);

            FILE *file = fopen(graph_file_name, "w");
            if (file == NULL){
                perror("Error opening file");
                exit(-1);
            }

            fprintf(file, "%d\n", num_node_of_graph);
            for (int i = 0; i < num_node_of_graph; i++){
                for (int j = 0; j < num_node_of_graph; j++){
                    fprintf(file, "%d ", adjacency_matrix[i][j]);
                }
                fprintf(file, "\n");
            }

            fclose(file);

            // releasing the lock
            sem_post(sem_writer);

            // sending message back to client
            strcpy(output.graph_file_name, "File Successfully added");
            output.operation_number = 1000 + sequence_number;
            if (msgsnd(temp->operation_number, &output, sizeof(output), 0) == -1){
                perror("msgsnd");
            }
        }
    }
    else if (operation_number == 2){
        if (access(graph_file_name, F_OK) != -1){
            // critical section
            sem_wait(sem_writer);

            FILE *file = fopen(graph_file_name, "w");
            if (file == NULL){
                perror("Error opening file");
                exit(-1);
            }

            fprintf(file, "%d\n", num_node_of_graph);
            for (int i = 0; i < num_node_of_graph; i++){
                for (int j = 0; j < num_node_of_graph; j++){
                    fprintf(file, "%d ", adjacency_matrix[i][j]);
                }
                fprintf(file, "\n");
            }

            fclose(file);

            // releasing the lock
            sem_post(sem_writer);

            // sending message back to client
            strcpy(output.graph_file_name, "File Successfully modified");
            output.operation_number = 1000 + sequence_number;
            if (msgsnd(temp->operation_number, &output, sizeof(output), 0) == -1){
                perror("msgsnd");
            }
        }
        else{
            // file doesn't exist, return error
            strcpy(output.graph_file_name, "File doesn't exists");
            output.operation_number = 1000 + sequence_number;
            if (msgsnd(temp->operation_number, &output, sizeof(output), 0) == -1){
                perror("msgsnd");
            }
        }
    }
    printf("Request Served!!\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    struct my_msgbuf buf; // buf object for receiving request from load balancer
    int msqid;            // id for the message queue, we created in load balancer
    int len;
    key_t key;
    key_t shm_key;

    // initializing read & write semaphore, one for each shared resource, i.e one semaphore for each graph database
    sem_t *writer_semaphore[NUM_RESOURCES];
    sem_t *reader_semaphore[NUM_RESOURCES];
    for (int i = 0; i < NUM_RESOURCES; i++){
        char semaphore_name_write[100];
        sprintf(semaphore_name_write, "%s%d", "W", i);

        char semaphore_name_read[100];
        sprintf(semaphore_name_read, "%s%d", "R", i);

        // removes named semaphore from system, to delete any existing semaphore
        sem_unlink(semaphore_name_write);
        sem_unlink(semaphore_name_read);

        writer_semaphore[i] = sem_open(semaphore_name_write, O_CREAT | O_EXCL, PERMS, 1);
        reader_semaphore[i] = sem_open(semaphore_name_read, O_CREAT | O_EXCL, PERMS, 1);

        if (writer_semaphore[i] == SEM_FAILED){
            perror("Error in sem_open");
            exit(-1);
        }

        if (reader_semaphore[i] == SEM_FAILED){
            perror("Error in sem_open");
            exit(-1);
        }
    }

    // shared memory
    int shmid;
    int *shmptr;

    printf("Primary Server running...\n");

    // get the key corresponding to the message queue created by load balancer
    if ((key = ftok("load_balancer.c", 'B')) == -1){
        perror("Error in ftok");
        exit(1);
    }

    // connect to the queue
    if ((msqid = msgget(key, PERMS)) == -1){
        perror("Error in msgget");
        exit(1);
    }

    // array to store tid of currently executing threads
    pthread_t tid_array[10000];
    pthread_attr_t thread_attr_array[10000];
    int index = 0;

    // primary server will receive requests from load balancer and send them to clients
    while (1){
        // reading messages from the message queue, it will read all the write requests
        if (msgrcv(msqid, &buf, sizeof(buf), 110, 0) == -1){
            perror("msgrcv");
            exit(1);
        }

        if (buf.sequence_number == 999){
            // cleanup request
            // waiting for existing threads to terminate, i.e complete all currently executing requests
            for (int i = 0; i < index; i++){
                pthread_join(tid_array[i], NULL);
            }

            // performing cleanup activites, i.e destorying reader and writer semaphores
            for (int i = 0; i < NUM_RESOURCES; i++){
                char semaphore_name_write[100];
                sprintf(semaphore_name_write, "%s%d", "W", i);

                char semaphore_name_read[100];
                sprintf(semaphore_name_read, "%s%d", "R", i);

                sem_close(writer_semaphore[i]); // closing write semaphores
                if (sem_unlink(semaphore_name_write) == -1){
                    perror("Error in sem_unlink");
                }

                sem_close(reader_semaphore[i]); // closing read semaphores
                if (sem_unlink(semaphore_name_read) == -1){
                    perror("Error in sem_unlink");
                }
            }

            // terminating the primary server
            printf("\nPrimary Server terminating...\n");
            exit(0);
        }

        printf("Request received from load balancer with sequence number %d and operation number %d\n", buf.sequence_number / 1000, buf.sequence_number % 1000);

        // passing the msqid to thread
        buf.operation_number = msqid;

        // creating a thread
        // initialize the attributes with default values
        pthread_attr_init(&thread_attr_array[index]);

        int result = pthread_create(&tid_array[index], &thread_attr_array[index], start_routine, (void *)&buf);
        index++;
        if (result != 0){
            perror("Error in thread creation\n");
        }
    }

    return 0;
}