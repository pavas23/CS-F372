#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>
#include<sys/wait.h>

int main(void){
    pid_t child_pid = fork();

    if(child_pid == -1){
        perror("Error creating child process\n");
        return 0;
    }

    if(child_pid == 0){

        // child process
        printf("Child Process....\n");
        pid_t grandchild_pid = fork();

        if(grandchild_pid == -1){
            perror("Error creating grandchild process\n");
            return 1;
        }

        if(grandchild_pid == 0){
            // grandchild process
            printf("Grandchild Process....\n");
            printf("Grandchild Process id is %d\n",getpid());
        }

        else{
            // child process
            wait(NULL); // It will wait for it's child process to finish i.e will wait for grandchild
            printf("Child Process....\n");
            printf("The PID returned to child process is %d\n",grandchild_pid);
            printf("Child Process id is %d\n",getpid());
        }
    }
    else{
        // parent process
        waitpid(child_pid,NULL,0); // It will wait for it's child process to finish
        printf("Parent Process....\n");
        printf("The PID returned to parent process is %d\n",child_pid);
        printf("Parent process id is %d\n",getpid());
    }

    // waitpid(pid,NULL)
    // if pid < -1, meaning wait for any child process whose process group ID is equal to absolute value of pid
    // -1, wait for any child process
    // 0, meaning wait for any child process whose process group ID is equal to that of calling process
    // >0, meaning wait for child process whose PID is equal to value pid passed

    return 0;
}