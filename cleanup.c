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

int main(int argc,char* argv[]){
    struct my_msgbuf buf;
    struct my_msgbuf output;
    int msqid; // message queue id, id for the message queue we created in server
    int len;
    key_t key;

    // get the key corresponding to the message queue, created by file server.c
    if((key=ftok("server.c",'B')) == -1){
        perror("ftok");
        exit(1);
    }

    // connect to the queue
    if((msqid=msgget(key,PERMS)) == -1){
        perror("msgget");
        exit(1);
    }
    
    int count = 0;
    // displaying the menu option to the user for cleanup
    while(1){
        printf("Do you want the server to terminate? Press Y for Yes and N for No\n");
        char ch[100];
        
        if(count!=0)
        {
        getchar();	
        }
        count++;
        
        scanf("%[^\n]s",ch);

        if(strcmp(ch,"Y")==0){
            // terminate the main server
            buf.mtype = 999; // as all other requests sent to the main server will be > 1000
            strcpy(buf.mtext,"Terminate the Main Server!!");
            
            // send terminate through message queue to server.c file
            if(msgsnd(msqid,&buf,sizeof(buf.mtext),0) == -1){
                perror("msgsnd");
            }

            // after terminating the main server, this process will also terminate
            exit(1);
        }
        else if(strcmp(ch,"N")==0){
            continue;
        }
        else{
            printf("Enter a valid character\n");
        }
    }

    return 0;
}

