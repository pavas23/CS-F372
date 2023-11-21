#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

#define PERMS 0644 // file permission
#define ROWS 30
#define COLS 30

struct my_msgbuf{
    long operation_number; // setting this field as long, because long datatype is taken as mtype
    int sequence_number;
    char graph_file_name[200];
};

int main(int argc, char *argv[]){
    struct my_msgbuf buf;    // buf object for sending message to load balancer
    struct my_msgbuf output; // buf object for receiving the message from server
    int msqid;               // id for the message queue, we created in load balacer
    int len;
    key_t key;
    key_t shm_key;

    // shared memory
    int shmid;
    int *shmptr;

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

    // display menu to user
    while (1){
        printf("1. Add a new graph to the database\n");
        printf("2. Modify an existing graph of the database\n");
        printf("3. Perform DFS on an existing graph of the database\n");
        printf("4. Perform BFS on an existing graph of the database\n");

        // prompt the user to enter input
        int sequence_number = 0;
        int operation_number = 0;
        char graph_file_name[200];

        printf("\nEnter Sequence Number\n");
        scanf("%d", &sequence_number);
        getchar(); // to read the '\n' character
        printf("Enter Operation Number\n");
        scanf("%d", &operation_number);
        getchar(); // to read the '\n' character
        printf("Enter Graph File Name\n");
        scanf("%[^\n]s", graph_file_name);
        fflush(stdin);

        // semaphore to synchronize write operation in shared memory and read operation by thread from same shared memory
        // as this shared memory will be created for every request, i.e for each sequence number
        char semaphore_name[100];
        sprintf(semaphore_name, "%s%d", "S", sequence_number);
        sem_unlink(semaphore_name);
        // initializing the semaphore to 0, so that client can write in shared memory and reader i.e. thread can't read until write operation by client is completed
        sem_t *sem = sem_open(semaphore_name, O_CREAT | O_EXCL, PERMS, 0);

        // send request to load balancer via message queue
        buf.sequence_number = sequence_number;
        buf.operation_number = operation_number;
        strcpy(buf.graph_file_name, graph_file_name);
        if (msgsnd(msqid, &buf, sizeof(buf), 0) == -1){
            perror("Error in msgsnd");
        }

        // generating shared memory key, for this request
        if ((shm_key = ftok("client.c", sequence_number)) == -1){
            perror("Error in ftok");
            exit(1);
        }

        // creating a shared memory
        shmid = shmget(shm_key, sizeof(int) * (ROWS + 1) * (COLS + 1), PERMS | IPC_CREAT);
        if (shmid == -1){
            perror("Error in shmid");
            exit(1);
        }

        shmptr = shmat(shmid, NULL, 0);
        if (shmptr == (int *)-1){
            perror("Error in shmptr");
            exit(1);
        }

        // taking input from user, to write to the shared memory segment for write operations
        if (operation_number == 1 || operation_number == 2){
            // as we initialized semaphore value to 0, so primary server thread can not access critical section (i.e the shared memory) unless and until it is release by sem_post i.e only once the client enters the input

            int num_nodes_of_graph = 0;

            // taking number of nodes in graph as input
            printf("Enter number of nodes of the graph\n");
            scanf("%d", &num_nodes_of_graph);

            if (num_nodes_of_graph == 0){
                // graph is empty
                printf("\n\nEmpty Graph: Invalid\n\n");
                continue;
            }

            // intializing shared memory matrix to -1
            for (int i = 0; i < num_nodes_of_graph + 1; i++){
                for (int j = 0; j < num_nodes_of_graph + 1; j++){
                    shmptr[i * num_nodes_of_graph + j] = -1; // n*i+j based indexing
                }
            }

            // writing the number of nodes to shared memory
            shmptr[0] = num_nodes_of_graph;

            // writing adjaceny matrix to shared memory
            printf("Enter adjacency matrix, each row on a separate line and elements of a single row separated by whitespace characters\n");
            for (int i = 1; i < num_nodes_of_graph + 1; i++){
                for (int j = 1; j < num_nodes_of_graph + 1; j++){
                    scanf("%d", &shmptr[i * num_nodes_of_graph + j]);
                }
            }

            sem_post(sem); // releasing the lock, this will make lock value as 1
        }
        else if (operation_number == 3 || operation_number == 4){

            // taking input from user, to write to shared memory segment for read operations
            int starting_vertex = 0;
            printf("Enter starting vertex\n");
            scanf("%d", &starting_vertex);

            // writing starting vertex to shared memory
            shmptr[0] = starting_vertex;
            sem_post(sem);
        }
        else{
            printf("Enter a valid operation number\n");
            continue;
        }

        // receiving the message through message queue
        if (msgrcv(msqid, &output, sizeof(output), 1000 + sequence_number, 0) == -1){
            perror("msgrcv");
            exit(1);
        }

        // displaying the output received by server via message queue
        if (operation_number == 1 || operation_number == 2){
            printf("\n\n%s\n\n", output.graph_file_name);
        }
        else{
            printf("\n\n");
            char *token = strtok(output.graph_file_name, " "); // this will extract the first token
            while (token != NULL){ // loop through string to extract all other tokens also
                printf("%s ", token);
                token = strtok(NULL, "_");
            }
            printf("\n\n");
        }

        // destroying the semaphore
        sem_close(sem); // closing the semaphore
        if (sem_unlink(semaphore_name) == -1){
            perror("Error in sem_unlink");
        }

        // deleting the shared memory segment created for this request
        if (shmdt(shmptr) == -1){
            perror("shmdt");
            exit(-1);
        }

        if (shmctl(shmid, IPC_RMID, 0) == -1){
            perror("Error in shmctl");
            exit(1);
        }
    }

    return 0;
}
