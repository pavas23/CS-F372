#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/msg.h>
#include<string.h>

#define PERMS 0644

struct my_msgbuf{
    long mtype;
    char mtext[200];
};

int main(int argc, char* argv[]){
    struct my_msgbuf buf; // buf obj for sending the message to server
    struct my_msgbuf output; // buf obj for receiving the message from message queue
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

    // asking the user to enter client ID
    int client_id = -1;
    // make sure ID entered by client is a positive number
    while(client_id <= 0){
        printf("Enter the Client-ID\n");
        scanf("%d",&client_id);
        if(client_id <= 0){
            printf("Please enter a positive integer as client ID\n");
        }
    }
   
    // display the menu to user
    while(1){
        int input_option = 0;
        printf("1. Enter 1 to contact the Ping Server\n");
        printf("2. Enter 2 to contact the File Search Server\n");
        printf("3. Enter 3 to contact the File Word Count Server\n");
        printf("4. Enter 4 if this client wishes to exit\n\n");
        scanf("%d",&input_option);

        if(input_option == 1){
            // hashing the mtype using both menu option as well as client_id
            buf.mtype = input_option + 1000*client_id; 
            strcpy(buf.mtext,"Hi");

            // send "Hi" through message queue to server.c file
            if(msgsnd(msqid,&buf,sizeof(buf.mtext),0) == -1){
                perror("msgsnd");
            }

            // receiving the output through msgrcv from message queue
            if(msgrcv(msqid,&output,sizeof(output.mtext),(input_option+4)+1000*client_id,0) == -1){
                perror("msgrcv");
                exit(1);
            }
            printf("%s\n",output.mtext);
        }
        else if(input_option == 2){
            // taking filename from the user
            char filename[1000];
            fflush(stdin);

            printf("Enter the file name to be searched\n");
            scanf("%[^\n]s",filename);
            fflush(stdin);

            // hashing the mtype using both menu option as well as client_id
            buf.mtype = input_option + 1000*client_id; 
            strcpy(buf.mtext,filename);

            // sending the filename through message queue to server.c file
            if(msgsnd(msqid,&buf,sizeof(buf.mtext),0) == -1){
                perror("msgsnd");
            }

            // receiving the output through msgrcv from message queue
            if(msgrcv(msqid,&output,sizeof(output.mtext),(input_option+4)+1000*client_id,0) == -1){
                perror("msgrcv");
                exit(1);
            }

            // printing the output message
            printf("%s\n",output.mtext);

        }else if(input_option == 3){
            // taking filename from the user
            char filename[1000];
            fflush(stdin);

            printf("Enter the file name to get the word count, if the file exists !\n");
            scanf("%[^\n]s",filename);
            fflush(stdin);

            // hashing the mtype using both menu option as well as client_id
            buf.mtype = input_option + 1000*client_id; 
            strcpy(buf.mtext,filename);

            // sending the filename through message queue to server.c file
            if(msgsnd(msqid,&buf,sizeof(buf.mtext),0) == -1){
                perror("msgsnd");
            }

            // receiving the output through msgrcv from message queue
            if(msgrcv(msqid,&output,sizeof(output.mtext),(input_option+4)+1000*client_id,0) == -1){
                perror("msgrcv");
                exit(1);
            }

            // printing the output message
            printf("%s\n\n",output.mtext);
        }
        else if(input_option == 4){
            // if 4 is entered as input client will terminate

            // hashing the mtype using both menu option as well as client_id
            buf.mtype = input_option + 1000*client_id; 
            strcpy(buf.mtext,"Terminate the Client");

            // sending the message through message queue to server.c file
            if(msgsnd(msqid,&buf,sizeof(buf.mtext),0) == -1){
                perror("msgsnd");
            }

            // exiting
            printf("\nSuccessfully terminated the client process\n\n");
            exit(1);

        }else{
            printf("Please Enter a Valid Option\n");
        }
        
    }
    return 0;
}
