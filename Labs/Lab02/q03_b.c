#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

int main(int argc,char* argv[]){
    pid_t child_pid = fork();

    if(child_pid == 0){
        // child process
        printf("Child Process...\n");
        execlp("cat","read",argv[1],NULL); // code after this line won't work, as it gets replaced by the new image being executed by execlp
    }
    else{
        // parent process
        wait(NULL); // will wait for child process to finish
        printf("Parent Process...\n");
        execlp("/bin/ls","ls","-l",NULL);
    }

    return 0;
}

