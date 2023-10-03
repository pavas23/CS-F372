#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<errno.h>

#define PERMS 0644

struct my_msgbuf{
    long mtype;
    char mtext[200];
};

int main(int argc, char* argv[]){
    struct my_msgbuf buf;
    int msqid; // message queue id, id for the message queue we created
    int len;
    key_t key;

    // ftok() is used to generate a unique key
    if((key=ftok("q01_writer.c",'B')) == -1){
        perror("ftok");
        exit(1);
    }

    // a new queue is created or an existing queue is opened by msgget
    if((msqid=msgget(key,PERMS | IPC_CREAT)) == -1){
        perror("msgget");
        exit(1);
    }

    printf("Writer ready to send\n");
    printf("Enter lines of text, ^D to quit\n");
    buf.mtype = 1; // we don't really care in this case

    // writing to the message queue
    while(fgets(buf.mtext,sizeof(buf.mtext),stdin) != NULL){
        len = strlen(buf.mtext);

        // remove new line at the end if it exists
        if(buf.mtext[len-1] == '\n') buf.mtext[len-1] = '\0';

        // to send message to message queue
        if(msgsnd(msqid,&buf,len+1,0) == -1){
            // +1 for '\0'
            perror("msgsnd");
        }
    }

    if(msgctl(msqid,IPC_RMID,NULL) == -1){
        perror("msgctl");
        exit(1);
    }

    return 0;
}
