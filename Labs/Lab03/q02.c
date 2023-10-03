#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

#define ARRAY_SIZE 5

int main(int argc,char* argv[]){

    // 0 and 1 are default file descriptors for stdin and stdout
    int pipe_fds[2]; // file descriptors for the pipe

    // creates a pipe with two file descriptos pipe_fds[0] for reading and pipe_fds[1] for writing
    if(pipe(pipe_fds) == -1){
        perror("Pipe Creation Failed\n");
        return 1;
    }

    // fork a child process
    pid_t child_pid = fork();
    if(child_pid == -1){
        perror("Child Process Creation Failed\n");
        return 1;
    }

    if(child_pid == 0){
        // child process
        close(pipe_fds[0]); // close the read end of the pipe
        int child_array[ARRAY_SIZE] = {1,2,3,4,5};
        // write to the pipe
        write(pipe_fds[1],child_array,sizeof(child_array));
        close(pipe_fds[1]); // close the write end of the pipe
    }
    else{
        // parent process
        close(pipe_fds[1]); // close the write end of the pipe
        int parent_array[ARRAY_SIZE];

        // read from the pipe after child process is done with writing
        read(pipe_fds[0],parent_array,sizeof(parent_array));

        int sum = 0;
        for(int i=0;i<ARRAY_SIZE;i++){
            printf("%d ",parent_array[i]);
            sum += parent_array[i];
        }
        printf("\n");
        printf("The sum of integers read from the pipe is %d\n",sum);

        close(pipe_fds[0]); // close the read end of the pipe
        wait(NULL);
    }

    return 0;
}

