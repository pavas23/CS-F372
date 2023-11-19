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

struct my_msgbuf{
    long operation_number; // setting this field as long, because long datatype is taken as mtype
    int sequence_number;
    char graph_file_name[200];
};

int main(int argc, char *argv[]){
    struct my_msgbuf buf; // buf object for sending message to load balancer

    int msqid; // id for the message queue, we created in load balacer
    int len;
    key_t key;
    key_t shm_key;

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

    int count = 0;
    // display menu to user
    while (1){
        char ch[100];
        printf("Want to terminate the application? Press Y (Yes) or N (No)\n");

        if (count != 0){
            getchar();
        }
        count++;
        scanf("%[^\n]s", ch);

        if (strcmp(ch, "Y") == 0){
            // inform load balancer to terminate the process
            buf.operation_number = 5; // as cleanup will send mtype as 5
            buf.sequence_number = 5;
            strcpy(buf.graph_file_name, "Cleanup");

            if (msgsnd(msqid, &buf, sizeof(buf), 0) == -1)
            {
                perror("Error in msgsnd");
            }

            // after informing load balancer, cleanup process will also terminate
            printf("Terminating...\n");
            exit(0);
        }
        else if (strcmp(ch, "N") == 0){
            continue;
        }
        else{
            printf("Enter a valid character\n");
        }
    }

    return 0;
}
