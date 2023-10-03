#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> // for fork()
#include<sys/types.h> // for pid_t
#include<sys/wait.h> // for wait()

int main(){

    // fork() function returns a negative value when it fails to create a child process
    // It returns 0 to child process and PID of child process is returned to parent
    pid_t pid = fork();

    // After forking, the child process will have its own copy of the variables and code, independent of the parent process

    if(pid == 0){
        // child process
        for(int i=0;i<100;i++){
            printf("%d\n",i);
        }
        printf("The child process id is %d\n",getpid());
        printf("The parent process id is %d\n",getppid());
    }else{
        // parent process
        for(int i=100;i<200;i++){
            printf("%d\n",i);
        }
        printf("The parent process id is %d\n",getpid());
        printf("The parent's parent process id is %d\n",getppid());
    }

    /* It can be noticed that the order of statements printed by the child
    and the parent is random. This is because there is no restriction or specification that controls this order, and depends simply on which process is serviced first by the OS */

    return 0;
}
