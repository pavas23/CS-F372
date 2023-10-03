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

    if((msqid = msgget(key,PERMS | IPC_CREAT)) == -1){
		perror("msgget");
		exit(1);
	}

	printf("Writer: ready to send messages.\n");

	for(int i=1;i<=3;i++){
		buf.mtype = i;
		len = sprintf(buf.mtext, "message%d",i);
		if(msgsnd(msqid,&buf,len+1,0) == -1)  // +1 for '\0'
			perror("msgsnd");
	}

	return 0;
}