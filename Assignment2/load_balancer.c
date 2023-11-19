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

#define PERMS 0644

struct my_msgbuf{
    long operation_number; // setting this field as long, because long datatype is taken as mtype
    int sequence_number;
    char graph_file_name[200];
};

int main(int argc, char *argv[]){
    struct my_msgbuf buf;   // buf object for sending the message to primary and secondary server
    struct my_msgbuf input; // buf object for receiving the message from client
    int msqid;              // message queue id, id for message queue that will be created by load balancer
    int len;
    key_t key;

    // ftok() is used to generate a unique key,
    if ((key = ftok("load_balancer.c", 'B')) == -1){
        perror("Error in ftok");
        exit(1);
    }

    // a new queue is created
    if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1){
        perror("Error in msgget");
        exit(1);
    }

    printf("Load Balancer running...\n");

    // load balancer will receive requests from clients and send them to primary and secondary servers.
    while (1){
        // reading messages from the message queue, it will read all the messages with mtype between 1 and 100
        if (msgrcv(msqid, &input, sizeof(input), -100, 0) == -1){
            perror("msgrcv");
            exit(1);
        }

        // if the request sent by load balancer is read again
        if (input.operation_number > 100){
            if (msgsnd(msqid, &input, sizeof(input), 0) == -1){
                perror("Error in msgsnd");
            }
        }
        else if (input.operation_number == 5){
            // check if msg is received from cleanup process, as cleanup will send mtype as 5
            // inform all three servers

            // sending message to primary server
            printf("Redirecting cleanup request to primary server...\n");
            input.sequence_number = 999;  // to inform cleanup request
            input.operation_number = 110; // so that request with mtype as 110 will be received by primary server
            if (msgsnd(msqid, &input, sizeof(input), 0) == -1){
                perror("Error in msgsnd");
            }

            // sending message to secondary server 1
            printf("Redirecting cleanup request to secondary server 1...\n");
            input.sequence_number = 999;  // to inform cleanup request
            input.operation_number = 120; // so that request with mtype as 120 will be received by secondary server 1
            if (msgsnd(msqid, &input, sizeof(input), 0) == -1){
                perror("Error in msgsnd");
            }

            // sending message to secondary server 2
            printf("Redirecting cleanup request to secondary server 2...\n");
            input.sequence_number = 999;  // to inform cleanup request
            input.operation_number = 130; // so that request with mtype as 130 will be received by secondary server 2
            if (msgsnd(msqid, &input, sizeof(input), 0) == -1){
                perror("Error in msgsnd");
            }

            // sleeps for 5 seconds
            sleep(5);

            // delete the message queue
            if (msgctl(msqid, IPC_RMID, NULL) == -1){
                perror("Error in msgctl");
                exit(1);
            }

            printf("Message queue deleted successfully and load balancer terminating...\n");
            exit(0);
        }
        else{
            // messages are received from clients, becuase operation number is from 1 to 4
            // hashing operation number with sequence number

            if (input.operation_number == 1 || input.operation_number == 2){
                printf("Redirecting request with sequence number %d and operation number %ld to primary server...\n", input.sequence_number, input.operation_number);

                input.sequence_number = input.sequence_number * 1000 + input.operation_number;
                // it is a write operation, send this request to primary server
                input.operation_number = 110; // so that request with mtype as 110 will be received by primary server

                if (msgsnd(msqid, &input, sizeof(input), 0) == -1){
                    perror("Error in msgsnd");
                }
            }
            else{
                // it is a read operation, send this request to secondary servers
                if (input.sequence_number % 2 != 0){
                    printf("Redirecting request with sequence number %d and operation number %ld to secondary server 1...\n", input.sequence_number, input.operation_number);

                    input.sequence_number = input.sequence_number * 1000 + input.operation_number;
                    // odd numbered read request, send this to secondary server 1
                    input.operation_number = 120; // so that request with mtype as 120 will be received by secondary server 1

                    if (msgsnd(msqid, &input, sizeof(input), 0) == -1){
                        perror("Error in msgsnd");
                    }
                }
                else{
                    printf("Redirecting request with sequence number %d and operation number %ld to secondary server 2...\n", input.sequence_number, input.operation_number);

                    input.sequence_number = input.sequence_number * 1000 + input.operation_number;
                    // even numbered read request, send this to secondary server 2
                    input.operation_number = 130; // so that request with mtype as 130 will be received by secondary server 2

                    if (msgsnd(msqid, &input, sizeof(input), 0) == -1){
                        perror("Error in msgsnd");
                    }
                }
            }
        }
    }

    return 0;
}