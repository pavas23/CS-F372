#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<string.h>

int factorial(int n){
    if(n == 0 || n == 1){
        return 1;
    }
    return n*factorial(n-1);
}

int main(int argc,char* argv[]){

    // taking input from the user
    int a,b,c;
    scanf("%d %d %d",&a,&b,&c);

    int pfds[2]; // creating file descriptors for the pipe
    int s; // status variable for wait

    // creating the pipe
    if(pipe(pfds) == -1){
        perror("Error in pipe creation");
        return 1;
    }

    // array for reading the output from pipe 
    int output_array[3];

    pid_t first_child_pid = fork();

    if(first_child_pid == 0){
        // first child process

        // finding factorial of first number by reading number from reading end of the pipe
        read(pfds[0],output_array,sizeof(output_array));
        output_array[0] = factorial(output_array[0]);

        // writing the factorial to the writing end of the pipe
        write(pfds[1],output_array,sizeof(output_array));

        close(pfds[0]);
        close(pfds[1]);
    }
    else{
        pid_t second_child_pid = fork();

        if(second_child_pid == 0){
            // second child process

            // finding factorial of second number by reading number from reading end of the pipe
            read(pfds[0],output_array,sizeof(output_array));
            output_array[1] = factorial(output_array[1]);

            // writing the factorial to the writing end of the pipe
            write(pfds[1],output_array,sizeof(output_array));

            close(pfds[0]);
            close(pfds[1]);
        }else{
            pid_t third_child_pid = fork();

            if(third_child_pid == 0){
                // third child process

                // finding factorial of third number by reading number from reading end of the pipe
                read(pfds[0],output_array,sizeof(output_array));
                output_array[2] = factorial(output_array[2]);

                // writing the factorial to the writing end of the pipe
                write(pfds[1],output_array,sizeof(output_array));

                close(pfds[0]);
                close(pfds[1]);
            }
            else{
                // parent process

                int input[3] = {a,b,c};

                // writing the input to writing end of the pipe
                write(pfds[1],input,sizeof(input));

                // waiting for all 3 child processes to finish
                wait(&s);
                wait(&s);
                wait(&s);

                int ans[3];

                // reading the factorial's from reading end of the pipe
                read(pfds[0],ans,sizeof(ans));

                for(int i=0;i<3;i++){
                    printf("%d\n",ans[i]);
                }
                printf("\n");

                close(pfds[0]);
                close(pfds[1]);
            }
        }
    }

    return 0;
}

