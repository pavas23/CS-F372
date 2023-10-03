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

	if((key = ftok("q02_writer.c",'B')) == -1){
		perror("ftok");
		exit(1);
	}

    if((msqid = msgget(key,PERMS)) == -1){	/* connect to the queue */
		perror("msgget");
		exit(1);
	}

	printf("Reader: ready to receive messages \n");

	for(int i=3;i>=1;i--){
		if(msgrcv(msqid,&buf,sizeof(buf.mtext),i,0) == -1){
			perror("msgrcv");
			exit(1);
		}
		printf("Reader: \"%s\"\n",buf.mtext);
	}
    
	return 0;
}
