#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>

#define PERMS 0644

struct my_msgbuf{
    long mtype;
    char mtext[200];
};

int main(void){
    struct my_msgbuf buf;
    int msqid;
    int len;
    key_t key;

    // get the key corresponding the message queue, created by file writer.c
    if((key=ftok("q01_writer.c",'B')) == -1){
        perror("ftok");
        exit(1);
    }

    // connect to the queue
    if((msqid=msgget(key,PERMS)) == -1){
        perror("msgget");
        exit(1);
    }

    printf("Reader: ready to receive messages \n");

    while(1){
        if(msgrcv(msqid,&buf,sizeof(buf.mtext),0,0) == -1){
            perror("msgrcv");
            exit(1);
        }
        printf("Reader: \"%s\"\n",buf.mtext);
    }

    return 0;
}
