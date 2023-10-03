#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>

// this program basically implements ls | wc -l

// 0 and 1 are default file descriptors for stdin and stdout

int main(int argc,char* argv[]){
    pid_t pid;
    int pfds[2]; // file descriptors for the pipe
    int s; // status variable for wait

    // create a pipe with two file descriptors pfds[0] for reading and pfds[1] for writing
    if(pipe(pfds) == -1){
        perror("Error in creating pipe");
        return 1;
    }

    // fork a child process
    pid = fork();

    if(pid == 0){
        // child process 1: execute "ls" command and redirect its output to child process 2
        dup2(pfds[1],1); // redirect stdout to writing end of the pipe
        close(pfds[0]);  // close the reading end of the pipe in this child
        close(pfds[1]);  // close the writing end of the pipe in this child

        if(execlp("ls","ls",NULL) == -1){
            perror("Error in execlp");
            return 1;
        }
    }
    else{
        // Parent Process
        if(fork() == 0){
            // child process 2: execute "wc -l" command and read from the pipe

            dup2(pfds[0],0); // redirect stdin to the reading end of the pipe 
            close(pfds[0]);  // close the reading end of the pipe in this child
            close(pfds[1]);  // close the writing end of the pipe in this child

            if(execlp("wc","wc","-l",NULL) == -1){
                perror("Error in execlp");
                return 1;
            }
        }
        else{
            // closes both ends of the pipe and waits for child processes
            close(pfds[0]);
            close(pfds[1]);
            wait(&s); 	// wait for first child to finish
			wait(&s); 	// wait for second child to finish
        }
    }

    return 0;
}