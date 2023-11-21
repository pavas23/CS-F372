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
#define MAX_VERTICES 30
#define NUM_RESOURCES 21 // maximum number of files in graph database system

int *shmptr_reader_count;

// struct for transfering message queue object
struct my_msgbuf{
    long operation_number; // setting this field as long, because long datatype is taken as mtype
    int sequence_number;
    char graph_file_name[200];
};

// struct for dfs
struct my_graph_dfs{
    int startingVertex;
    int num_node_of_graph;
    int (*adjacency_matrix)[COLS + 1];
    int *visited;
    int *deepestVertices;
    int *count;
    sem_t *semaphore;
};

// struct for queue
struct Queue{
    int *array;
    int front, rear;
    int capacity;
};

// struct for bfs
struct my_graph_bfs{
    int startingVertex;
    int num_node_of_graph;
    int (*adjacency_matrix)[COLS + 1];
    int *visited;
    int *bfs_nodes;
    int *count;
    struct Queue *queue;
    sem_t *semaphore;
};

// function for creaing queue using circular array
struct Queue *createQueue(int capacity){
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = -1;
    queue->rear = -1;
    queue->array = (int *)malloc(queue->capacity * sizeof(int));
    return queue;
}

int isQueueEmpty(struct Queue *queue){
    if (queue->front == -1){
        return 1;
    }
    return 0;
}

int isQueueFull(struct Queue *queue){
    if ((queue->rear + 1) % queue->capacity == queue->front)
        return 1;
    return 0;
}

void enqueue(struct Queue *queue, int item){
    if (isQueueFull(queue)){
        printf("Queue is full\n");
        return;
    }
    if (isQueueEmpty(queue)){
        queue->front = 0;
        queue->rear = 0;
    }
    else{
        queue->rear = (queue->rear + 1) % queue->capacity;
    }

    queue->array[queue->rear] = item;
    return;
}

int dequeue(struct Queue *queue){
    if (isQueueEmpty(queue)){
        printf("Queue is empty\n");
        return -1;
    }

    int item = queue->array[queue->front];

    if (queue->front == queue->rear){
        queue->front = -1;
        queue->rear = -1;
    }
    else{
        queue->front = (queue->front + 1) % queue->capacity;
    }

    return item;
}

int queue_size(struct Queue *queue){
    return queue->rear - queue->front + 1;
}

// dfs helper function for each thread
void *dfsHelper(void *arg){
    struct my_graph_dfs *temp = (struct my_graph_dfs *)arg;
    // marking this vertex as visited
    temp->visited[temp->startingVertex] = 1;

    int num_neighbours = 0;
    int index_neighbour = 0;

    for (int i = 0; i < temp->num_node_of_graph; i++){
        if (temp->adjacency_matrix[temp->startingVertex - 1][i] != 0){
            num_neighbours++;
            index_neighbour = i + 1;
            if (num_neighbours > 1){
                break;
            }
        }
    }

    if (num_neighbours == 0){
        sem_wait(temp->semaphore);
        temp->deepestVertices[*(temp->count)] = temp->startingVertex;
        (*temp->count)++;
        sem_post(temp->semaphore);
        return NULL;
    }

    if (num_neighbours == 1 && temp->visited[index_neighbour] == 1){
        // deepest vertex
        sem_wait(temp->semaphore);
        temp->deepestVertices[*(temp->count)] = temp->startingVertex;
        (*temp->count)++;
        sem_post(temp->semaphore);
        return NULL;
    }

    // array to keep track of child thread id's
    pthread_t tid_array[100];
    pthread_attr_t thread_attr_array[100];
    int index = 0;

    // iterating over neighbors of this vertex
    for (int i = 0; i < temp->num_node_of_graph; i++){
        if (temp->adjacency_matrix[temp->startingVertex - 1][i] != 0 && !temp->visited[i + 1]){
            // means there is edge from vertex startingVertex to vertex i and ith vertex is unvisited
            // so we need to create a thread for this unvisited vertex

            // initialize the attributes with default values
            pthread_attr_init(&thread_attr_array[index]);

            struct my_graph_dfs *graph = malloc(sizeof(struct my_graph_dfs));
            graph->startingVertex = i + 1;
            graph->num_node_of_graph = temp->num_node_of_graph;
            graph->adjacency_matrix = temp->adjacency_matrix;
            graph->visited = temp->visited;
            graph->deepestVertices = temp->deepestVertices;
            graph->count = temp->count;
            graph->semaphore = temp->semaphore;

            int result = pthread_create(&tid_array[index], &thread_attr_array[index], dfsHelper, (void *)graph);
            index++;
            if (result != 0){
                perror("Error in thread creation\n");
            }
        }
    }

    // parent thread waiting for child threads to join
    for (int i = 0; i < index; i++){
        pthread_join(tid_array[i], NULL);
    }

    return NULL;
}

// fucntion for dfs traversal
void *start_routine_dfs(void *arg){
    struct my_msgbuf *temp = (struct my_msgbuf *)arg;
    int sequence_number = temp->sequence_number / 1000;
    int operation_number = temp->sequence_number % 1000;

    struct my_msgbuf output; // buf object for sending the output to client

    char graph_file_name[200];
    strcpy(graph_file_name, temp->graph_file_name);

    int semaphore_index = 0;
    if (strlen(graph_file_name) == 6){
        semaphore_index = graph_file_name[1] - '0';
    }
    else{
        semaphore_index = (graph_file_name[1] - '0') * 10 + (graph_file_name[2] - '0');
    }

    // opening the named semaphore corresponding to reader semaphore for this particular file
    char semaphore_name_reader[100];
    sprintf(semaphore_name_reader, "%s%d", "R", semaphore_index);
    sem_t *sem_reader = sem_open(semaphore_name_reader, 0);

    // opening writer semaphore corresponding to this particular file
    char semaphore_name_writer[100];
    sprintf(semaphore_name_writer, "%s%d", "W", semaphore_index);
    sem_t *sem_writer = sem_open(semaphore_name_writer, 0);

    // incrementing reader count for this reader
    sem_wait(sem_reader);
    shmptr_reader_count[semaphore_index]++;
    if (shmptr_reader_count[semaphore_index] == 1){
        sem_wait(sem_writer);
    }
    sem_post(sem_reader);

    // connecting to shared memory segement created by the client process
    key_t key;
    int shmid;
    int *shmptr;

    if ((key = ftok("client.c", sequence_number)) == -1){
        perror("Error in ftok");
        exit(-1);
    }

    // using semaphore so that it waits till user enters starting vertex, before reading from shared memory
    char semaphore_name[100];
    sprintf(semaphore_name, "%s%d", "S", sequence_number);
    sem_t *sem = sem_open(semaphore_name, 0); // initializing semaphore to 1

    sem_wait(sem); // locks the critical section
    shmid = shmget(key, sizeof(int) * (ROWS + 1) * (COLS + 1), PERMS);
    shmptr = shmat(shmid, NULL, 0); // pointer to shared memory
    if (shmptr == (int *)-1){
        perror("Error in shmptr");
        exit(1);
    }

    int starting_vertex = 0;
    starting_vertex = shmptr[0];
    sem_post(sem); // releases the lock

    int readerCount = shmptr_reader_count[semaphore_index]; // reading the reader count at index corresponding to file number

    if (starting_vertex <= 0){
        // error as starting vertex can not be less than or equal to 0
        strcpy(output.graph_file_name, "Error: Starting vertex can not be less than or equal to 0!!");
        output.operation_number = 1000 + sequence_number;
        if (msgsnd(temp->operation_number, &output, sizeof(output), 0) == -1)
        {
            perror("msgsnd");
        }
    }
    else if (starting_vertex > 30){
        // error as starting vertex can not be greater than 30
        strcpy(output.graph_file_name, "Error: Starting vertex can not be greater than 30!!");
        output.operation_number = 1000 + sequence_number;
        if (msgsnd(temp->operation_number, &output, sizeof(output), 0) == -1)
        {
            perror("msgsnd");
        }
    }
    else{
        // check if file exists or not
        if (access(graph_file_name, F_OK) != -1){
            // file already exists

            FILE *file = fopen(graph_file_name, "r");

            if (file == NULL){
                perror("Error opening file");
                exit(-1);
            }

            int num_node_of_graph = 0;
            char line[100];
            if (fgets(line, sizeof(line), file) != NULL){
                num_node_of_graph = atoi(line);
            }

            int adjacency_matrix[ROWS + 1][COLS + 1];
            int currRow = 0, currCol = 0;

            char ch;
            while ((ch = fgetc(file)) != EOF){
                if (ch == ' ' || ch == '\n'){
                    continue;
                }
                adjacency_matrix[currRow][currCol] = ch - '0';
                currCol++;
                if (currCol == num_node_of_graph){
                    currCol = 0;
                    currRow += 1;
                }
            }

            fclose(file);

            sem_t semaphore;
            sem_init(&semaphore, 0, 1);

            // defining an array for storing deepest vertex along each path
            int *deepestVertices = (int *)malloc(sizeof(int) * MAX_VERTICES);
            // visited array
            int *visited = (int *)malloc(sizeof(int) * (num_node_of_graph + 1));
            int count_of_deepest_vertices = 0;
            int *count = &count_of_deepest_vertices;

            // marking all vertices of graph as unvisited initially
            for (int i = 0; i <= num_node_of_graph; i++){
                visited[i] = 0;
            }

            pthread_t tid;              // thread id
            pthread_attr_t thread_attr; // pthread attribute structure

            // initialize the attributes with default values
            pthread_attr_init(&thread_attr);

            struct my_graph_dfs *graph = malloc(sizeof(struct my_graph_dfs));
            graph->startingVertex = starting_vertex;
            graph->num_node_of_graph = num_node_of_graph;
            graph->adjacency_matrix = adjacency_matrix;
            graph->visited = visited;
            graph->deepestVertices = deepestVertices;
            graph->count = count;
            graph->semaphore = &semaphore;

            // creating thread for starting vertex
            int result = pthread_create(&tid, &thread_attr, dfsHelper, (void *)graph);
            if (result != 0){
                perror("Error in thread creation\n");
            }

            void *status; // pointer to location where exit status of joined thread will be stored
            result = pthread_join(tid, &status);
            if (result != 0){
                perror("Thread join failed");
            }

            // destroy the mutex for this request
            sem_destroy(&semaphore);

            // after this deepest vertices, will contain list of desired vertices
            char dfsTraversal[100];
            int index = 0;

            for (int i = 0; i < count_of_deepest_vertices; i++){
                int val = deepestVertices[i];
                char tempStr[10];
                sprintf(tempStr, "%d", val);
                strcat(dfsTraversal, tempStr);
                strcat(dfsTraversal, " ");
            }

            // sending output to client via message queue
            strcpy(output.graph_file_name, dfsTraversal);
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

    // decrementing reader count for this particular file
    sem_wait(sem_reader);
    shmptr_reader_count[semaphore_index]--;
    if (shmptr_reader_count[semaphore_index] == 0){
        sem_post(sem_writer);
    }
    sem_post(sem_reader);

    printf("Request Served!!\n");
    pthread_exit(NULL);
    return NULL;
}

// bfs helper function for each thread
void *bfsHelper(void *arg)
{
    struct my_graph_bfs *temp = (struct my_graph_bfs *)arg;

    // iterate over all its neighbours
    for (int i = 0; i < temp->num_node_of_graph; i++){
        if (temp->adjacency_matrix[temp->startingVertex - 1][i] != 0 && !temp->visited[i + 1]){
            sem_wait(temp->semaphore);
            // start of critical section
            temp->visited[i + 1] = 1;
            temp->bfs_nodes[(*temp->count)] = i + 1;
            (*temp->count)++;
            enqueue(temp->queue, i + 1);
            // end of critical section
            sem_post(temp->semaphore);
        }
    }

    pthread_exit(NULL);
    return NULL;
}

// function for bfs traversal
void *start_routine_bfs(void *arg){
    struct my_msgbuf *temp = (struct my_msgbuf *)arg;
    int sequence_number = temp->sequence_number / 1000;
    int operation_number = temp->sequence_number % 1000;

    struct my_msgbuf output; // buf object for sending the output to client

    char graph_file_name[200];
    strcpy(graph_file_name, temp->graph_file_name);

    int semaphore_index = 0;
    if (strlen(graph_file_name) == 6){
        semaphore_index = graph_file_name[1] - '0';
    }
    else{
        semaphore_index = (graph_file_name[1] - '0') * 10 + (graph_file_name[2] - '0');
    }

    // opening the named semaphore corresponding to reader semaphore for this particular file
    char semaphore_name_reader[100];
    sprintf(semaphore_name_reader, "%s%d", "R", semaphore_index);
    sem_t *sem_reader = sem_open(semaphore_name_reader, 0);

    // opening writer semaphore corresponding to this particular file
    char semaphore_name_writer[100];
    sprintf(semaphore_name_writer, "%s%d", "W", semaphore_index);
    sem_t *sem_writer = sem_open(semaphore_name_writer, 0);

    // incrementing reader count for this reader
    sem_wait(sem_reader);
    shmptr_reader_count[semaphore_index]++;
    if (shmptr_reader_count[semaphore_index] == 1){
        sem_wait(sem_writer);
    }
    sem_post(sem_reader);

    // connecting to shared memory segement created by the client process
    key_t key;
    int shmid;
    int *shmptr;

    if ((key = ftok("client.c", sequence_number)) == -1){
        perror("Error in ftok");
        exit(-1);
    }

    // using semaphore so that it waits till user enters starting vertex, before reading from shared memory
    char semaphore_name[100];
    sprintf(semaphore_name, "%s%d", "S", sequence_number);
    sem_t *sem = sem_open(semaphore_name, 0); // initializing semaphore to 1

    sem_wait(sem); // locks the critical section
    shmid = shmget(key, sizeof(int) * (ROWS + 1) * (COLS + 1), PERMS);
    if (shmid == -1){
        perror("Error in shmid");
        exit(1);
    }
    shmptr = shmat(shmid, NULL, 0); // pointer to shared memory
    if (shmptr == (int *)-1){
        perror("Error in shmptr");
        exit(1);
    }

    int starting_vertex = 0;
    starting_vertex = shmptr[0];
    sem_post(sem); // releases the lock

    int readerCount = shmptr_reader_count[semaphore_index]; // reading the reader count at index corresponding to file number

    if (starting_vertex <= 0){
        // error as starting vertex can not be neagtive
        strcpy(output.graph_file_name, "Error: Starting vertex can not be less than or equal to 0!!");
        output.operation_number = 1000 + sequence_number;
        if (msgsnd(temp->operation_number, &output, sizeof(output), 0) == -1)
        {
            perror("msgsnd");
        }
    }
    else if (starting_vertex > 30){
        // error as starting vertex can not be greater than 30
        strcpy(output.graph_file_name, "Error: Starting vertex can not be greater than 30!!");
        output.operation_number = 1000 + sequence_number;
        if (msgsnd(temp->operation_number, &output, sizeof(output), 0) == -1)
        {
            perror("msgsnd");
        }
    }
    else{
        // check if file exists or not
        if (access(graph_file_name, F_OK) != -1){
            // file already exists

            // critical section
            FILE *file = fopen(graph_file_name, "r");

            if (file == NULL){
                perror("Error opening file");
                exit(-1);
            }

            int num_node_of_graph = 0;
            char line[100];
            if (fgets(line, sizeof(line), file) != NULL){
                num_node_of_graph = atoi(line);
            }

            int adjacency_matrix[ROWS + 1][COLS + 1];
            int currRow = 0, currCol = 0;

            char ch;
            while ((ch = fgetc(file)) != EOF){
                if (ch == ' ' || ch == '\n'){
                    continue;
                }
                adjacency_matrix[currRow][currCol] = ch - '0';
                currCol++;
                if (currCol == num_node_of_graph){
                    currCol = 0;
                    currRow += 1;
                }
            }

            fclose(file);

            sem_t semaphore;
            sem_init(&semaphore, 0, 1);

            struct Queue *queue = createQueue(num_node_of_graph);
            int *bfs_nodes = (int *)malloc(sizeof(int) * num_node_of_graph);     // defining an array for storing deepest vertex along each path
            int *visited = (int *)malloc(sizeof(int) * (num_node_of_graph + 1)); // visited array
            int count_of_bfs_nodes = 0;
            int *count = &count_of_bfs_nodes;

            // marking all vertices of graph as unvisited initially
            for (int i = 0; i <= num_node_of_graph; i++){
                visited[i] = 0;
            }

            visited[starting_vertex] = 1;
            bfs_nodes[*count] = starting_vertex;
            (*count)++;
            pthread_t tid;
            pthread_attr_t thread_attr;
            struct my_graph_bfs *graph = malloc(sizeof(struct my_graph_bfs));
            graph->startingVertex = starting_vertex;
            graph->num_node_of_graph = num_node_of_graph;
            graph->adjacency_matrix = adjacency_matrix;
            graph->visited = visited;
            graph->bfs_nodes = bfs_nodes;
            graph->count = count;
            graph->queue = queue;
            graph->semaphore = &semaphore;

            int result = pthread_create(&tid, NULL, bfsHelper, (void *)graph);
            if (result != 0){
                perror("Error in thread creation\n");
            }
            void *status;
            result = pthread_join(tid, &status);
            if (result != 0){
                perror("Thread join failed");
            }

            while (!isQueueEmpty(queue)){
                int num_nodes_level = queue_size(queue);
                pthread_t threads[num_nodes_level];
                pthread_attr_t thread_attrs[num_nodes_level];

                int currNode = 0;
                for (int i = 0; i < num_nodes_level; i++){
                    currNode = dequeue(queue);
                    // thread for processing the current node
                    struct my_graph_bfs *graph = malloc(sizeof(struct my_graph_bfs));
                    graph->startingVertex = currNode;
                    graph->num_node_of_graph = num_node_of_graph;
                    graph->adjacency_matrix = adjacency_matrix;
                    graph->visited = visited;
                    graph->bfs_nodes = bfs_nodes;
                    graph->count = count;
                    graph->queue = queue;
                    graph->semaphore = &semaphore;
                    int result = pthread_create(&threads[i], NULL, bfsHelper, (void *)graph);
                    if (result != 0){
                        perror("Error in thread creation\n");
                    }
                }

                // wait for all threads at same level to complete
                for (int i = 0; i < num_nodes_level; i++){
                    void *status; // pointer to location where exit status of joined thread will be stored
                    int result = pthread_join(threads[i], &status);
                    if (result != 0){
                        perror("Thread join failed");
                    }
                }
            }

            // free the queue memory
            free(queue->array);
            free(queue);

            // destroy the mutex for this request
            sem_destroy(&semaphore);

            // after this deepest vertices, will contain list of desired vertices
            char bfsTraversal[100];
            int index = 0;

            for (int i = 0; i < count_of_bfs_nodes; i++){
                int val = bfs_nodes[i];
                char tempStr[10];
                sprintf(tempStr, "%d", val);
                strcat(bfsTraversal, tempStr);
                strcat(bfsTraversal, " ");
            }

            // sending output to client via message queue
            strcpy(output.graph_file_name, bfsTraversal);
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

    // decrementing reader count for this particular file
    sem_wait(sem_reader);
    shmptr_reader_count[semaphore_index]--;
    if (shmptr_reader_count[semaphore_index] == 0){
        sem_post(sem_writer);
    }
    sem_post(sem_reader);

    printf("Request Served!!\n");
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[]){
    struct my_msgbuf buf; // buf object for receiving request from load balancer
    int msqid;            // id for the message queue, we created in load balancer
    int len;
    key_t key;
    key_t shm_key;

    // shared memory
    int shmid;

    if ((shm_key = ftok("secondary_server.c", 'S')) == -1){
        perror("Error in ftok");
        exit(1);
    }

    // creating or connecting to already existing shared memory segment
    // this shared memory segment is created for reader-writer problem, to store reader count for each shared resource i.e for each graph database
    shmid = shmget(shm_key, sizeof(int) * NUM_RESOURCES, PERMS | IPC_CREAT);
    if (shmid == -1){
        perror("Error in shmid");
        exit(1);
    }

    shmptr_reader_count = shmat(shmid, NULL, 0);
    if (shmptr_reader_count == (int *)-1){
        perror("Error in shmptr");
        exit(1);
    }

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

    // taking input from user, to decide whether it is secondary server 1 or secondary server 2
    char server_number_ch[200];

    printf("Enter 1 for running secondary server 1 and 2 for running secondary server 2\n");
    scanf("%[^\n]s", server_number_ch);
    while (strcmp(server_number_ch, "1") != 0 && strcmp(server_number_ch, "2") != 0){
        printf("Wrong input !!\n");
        printf("Enter 1 for running secondary server 1 and 2 for running secondary server 2\n");
        getchar();
        scanf("%[^\n]s", server_number_ch);
    }
    int server_number = (server_number_ch[0] - '0');

    printf("Secondary Server %d running...\n", server_number);

    // array to store tid of currently executing threads
    pthread_t tid_array[10000];
    pthread_attr_t thread_attr_array[10000];
    int index = 0;

    // secondary server will receive requests from load balancer and send them to clients
    while (1){
        if (server_number == 1){
            // secondary server 1 will read odd numbered read requests
            if (msgrcv(msqid, &buf, sizeof(buf), 120, 0) == -1){
                perror("msgrcv");
                exit(1);
            }
        }
        else{
            // secondary server 2 will read even numbered read requests
            if (msgrcv(msqid, &buf, sizeof(buf), 130, 0) == -1){
                perror("msgrcv");
                exit(1);
            }
        }

        // incrementing the reader count for given graph file
        int file_number = 0;
        if (strlen(buf.graph_file_name) == 6){
            file_number = buf.graph_file_name[1] - '0';
        }
        else{
            file_number = (buf.graph_file_name[1] - '0') * 10 + (buf.graph_file_name[2] - '0');
        }

        if (buf.sequence_number == 999){
            // cleanup request
            // waiting for existing threads to terminate, i.e complete all currently executing requests
            for (int i = 0; i < index; i++){
                pthread_join(tid_array[i], NULL);
            }

            // terminating the secondary server
            printf("\nSecondary Server %d terminating...\n", server_number);
            exit(0);
        }

        printf("Request received from load balancer with sequence number %d and operation number %d\n", buf.sequence_number / 1000, buf.sequence_number % 1000);

        // creating a thread to handle the request sent to this secondary server
        // initialize the attributes with default values
        pthread_attr_init(&thread_attr_array[index]);

        int operation_number = buf.sequence_number % 1000;
        int result = 0;

        if (operation_number == 3){
            // dfs operation
            buf.operation_number = msqid;
            result = pthread_create(&tid_array[index], &thread_attr_array[index], start_routine_dfs, (void *)&buf);
            index++;
        }
        else if (operation_number == 4){
            // bfs operation
            buf.operation_number = msqid;
            result = pthread_create(&tid_array[index], &thread_attr_array[index], start_routine_bfs, (void *)&buf);
            index++;
        }

        if (result != 0){
            perror("Error in thread creation\n");
        }
    }

    return 0;
}
