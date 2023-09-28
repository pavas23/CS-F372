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

// function for handling request of type 1
void handle_response_type1(int msqid,int request_type,int client_id){
    struct my_msgbuf response_to_client;

    // set the data bytes for the response message 
    response_to_client.mtype = 1000*client_id+(4+request_type);
    strcpy(response_to_client.mtext,"Hello");

    // writing the response in the message queue
    if(msgsnd(msqid,&response_to_client,sizeof(response_to_client.mtext),0) == -1){
        perror("msgsnd");
        exit(1);
    }

    printf("Server: Hello sent to client with ID %d from Ping Server !!\n",client_id);
    return;
}

// function for handling request of type 2
void handle_response_type2(int msqid,int request_type,int client_id,struct my_msgbuf buf){
    printf("Sever: Request sent to File Search Server, from client with ID: %d\n",client_id);

    struct my_msgbuf response_to_client;

    // set the data type for the response message 
    response_to_client.mtype = 1000*client_id+(4+request_type);

    // filename given by user that we need to search
    char filename[1000];
    strcpy(filename,buf.mtext);

    int pfds[2]; // File descriptors for the pipe

    // create the pipe
    // creates a pipe with two file descriptos pipe_fds[0] for reading and pipe_fds[1] for writing
    if(pipe(pfds) == -1){
        perror("Error in pipe");
        exit(1);
    }

    // forking the second child process
    pid_t second_child_pid = fork();

    if(second_child_pid == -1){
        perror("Error creating second child process");
        exit(1);
    }

    if(second_child_pid == 0){
        // second child process to perform ls, and write it to the pipe

        close(pfds[0]);  // close the reading end of the pipe
        dup2(pfds[1],1); // redirect stdout to the writing end of the pipe
        close(pfds[1]);  // close the writing end of the pipe

        // executing the execlp command
        if(execlp("ls","ls",NULL) == -1){
            char error_message[] = "Error_in_execlp_function!!_Can't_give_the_word_count ";
            write(pfds[1],error_message,sizeof(error_message));
            perror("Error in execlp");
            exit(1);
        }

    }else{
        // first child process for searching the file in the output given by ls command
        wait(NULL); // will wait for child process

        close(pfds[1]);  // close the writing end of the pipe

        // filename_output will store string containing all the files found by "ls"
        char list_of_filenames[1000];

        int bytes_read = read(pfds[0],list_of_filenames,sizeof(list_of_filenames));

        if(bytes_read == -1){
            perror("Error in reading of the pipe");
            exit(1);
        }

        close(pfds[0]);  // close the reading end of the pipe

        // if File is not found
        strcpy(response_to_client.mtext,"File does not exist !!\n");

        // for searching the file among all the files in the given directory
        char* token = strtok(list_of_filenames,"\n");
        while(token != NULL){
            if(strcmp(token,filename) == 0){
                strcpy(response_to_client.mtext,"File exists in the current directory !!\n");
                break;
            }
            token = strtok(NULL,"\n");
        }

        // sending the response message to the client via message queue
        if(msgsnd(msqid,&response_to_client,sizeof(response_to_client.mtext),0) == -1){
            perror("msgsnd");
            exit(1);
        }
    }

    return;
}

void handle_response_type3(int msqid,int request_type,int client_id,struct my_msgbuf buf){
    printf("Sever: Request sent to File Word Count Server, from client with ID: %d\n",client_id);

    struct my_msgbuf response_to_client;

    // set the data type for the response message 
    response_to_client.mtype = 1000*client_id+(4+request_type);

    // filename given by user in which we need to count number of words
    char filename[1000];
    strcpy(filename,buf.mtext);

    int pfds[2];  // File descriptors for the pipe

    // create the pipe
    // creates a pipe with two file descriptos pipe_fds[0] for reading and pipe_fds[1] for writing
    if(pipe(pfds) == -1){
        perror("Error in pipe");
        exit(1);
    }

    // forking the second child process
    pid_t second_child_pid = fork();

    if(second_child_pid == -1){
        perror("Error creating second child process");
        exit(1);
    }

    if(second_child_pid == 0){
        // second child process to perform wc, and write it to the pipe

        close(pfds[0]);  // close the reading end of the pipe
        dup2(pfds[1],1); // redirect stdout to the writing end of the pipe
        close(pfds[1]);  // close the writing end of the pipe

        // executing the execlp command
        if(execlp("wc","wc","-w",filename,NULL) == -1){
            // if execlp doesn't work then write the error message in the writing end of the pipe
            char error_message[] = "Error_in_execlp_function!!_Can't_give_the_word_count ";
            write(pfds[1],error_message,sizeof(error_message));
            perror("Error in execlp");
            exit(1);
        }

    }else{
        // first child process for searching the file in the output given by ls command
        wait(NULL); // will wait for child process

        close(pfds[1]);  // close the writing end of the pipe

        // word_count_output will store the string containing word count
        char word_count_output[200];

        int bytes_read = read(pfds[0],word_count_output,sizeof(word_count_output));

        if(bytes_read == -1){
            perror("Error in reading of the pipe");
            exit(1);
        }

        if(bytes_read == 0){
            // file does not exist
            strcpy(response_to_client.mtext,"File does not exist !!");
        }else{
            word_count_output[bytes_read] = '\0';  // Null-terminate the string
            char *wordCount = strtok(word_count_output, " "); // parsing the word count from the string
            strcpy(response_to_client.mtext,wordCount);
        }

        close(pfds[0]);  // close the reading end of the pipe
      
        // sending the response message to the client via message queue
        if(msgsnd(msqid,&response_to_client,sizeof(response_to_client.mtext),0) == -1){
            perror("msgsnd");
            exit(1);
        }
    }

    return;
}

void handle_request_terminate(int msqid,struct my_msgbuf buf,int numChildProcesses){

    // it will wait for all child processes to finish, before exiting
    for(int i=0;i<numChildProcesses;i++){
        wait(NULL);
    }

    printf("Main Server Exiting...\n\n");

    // this will delete the message queue
    if(msgctl(msqid,IPC_RMID,NULL) == -1){
        perror("msgctl");
        exit(1);
    }

    return;
}

int main(int argc, char* argv[]){

    // making the buffer
    struct my_msgbuf buf;
    int msqid; // message queue id, id for the message queue we created in server
    int len;
    int s; // status variable for wait
    key_t key;

    // ftok() is used to generate a unique key,
    if((key=ftok("server.c",'B')) == -1){
        perror("ftok");
        exit(1);
    }

    // a new queue is created
    if((msqid=msgget(key,PERMS | IPC_CREAT)) == -1){
        perror("msgget");
        exit(1);
    }

    printf("Main Server Running...\n");
    int numChildProcesses = 0;

    // main server will loop forever, and serve the requests given by the clients
    while(1){
        // reading messages from the message queue
        if(msgrcv(msqid,&buf,sizeof(buf.mtext),0,0) == -1){
            perror("msgrcv");
            exit(1);
        }

        numChildProcesses++; // increment the child Process everytime a child is created

        // Fork a child process, to create a child server which will serve all the client requests
        pid_t child_pid = fork();

        if(child_pid == -1){
            perror("Error in child process creation");
            exit(1);
        }

        if(child_pid == 0){
            // Inside the Child Server, which will serve every client request
            int client_id = buf.mtype/1000;
            int request_type = buf.mtype%10;

            // if the server tries to read a request that is already processed, that will have request_type > 4
            if(request_type > 4){
                if(msgsnd(msqid,&buf,sizeof(buf.mtext),0) == -1){
                    perror("msgsnd");
                }
                sleep(1); // sleep for 1s, so that client can read the result from the queue for synchronization purpose
            }

            if(request_type == 1){
                // Ping Server
                handle_response_type1(msqid,request_type,client_id);
            }
            else if(request_type == 2){
                // File Search Server
                handle_response_type2(msqid,request_type,client_id,buf);
            }
            else if(request_type == 3){
                // File Word Count Server
                handle_response_type3(msqid,request_type,client_id,buf);
            }
            else if(request_type == 4){
                // Message that client process has been terminated
                printf("Server: Client Process with ID: %d terminated\n",client_id);
            }
            exit(0); // terminate the child process, after performing the action
        }
        else{
            // parent process
            if(buf.mtype/1000 == 0){
                // this means this is termination request, sent by cleanup process as its mtype was < 1000
                handle_request_terminate(msqid,buf,numChildProcesses);
                exit(1); // to terminate the server
            }
        }
    }

    return 0;
}
