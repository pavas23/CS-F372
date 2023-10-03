#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/wait.h>

int main(int argc,char* argv[]){
    pid_t child_pid = fork();

    if(child_pid < 0){
        perror("Error in creating child process\n");
        return 1;
    }
    if(child_pid == 0){
        // child process
        printf("Child process started\n");

        FILE* input = fopen("input.txt","r");
        FILE* output = fopen("output.txt","w+");
        char c = fgetc(input);

        while(c != EOF){
            fputc(c,output);
            c = fgetc(input);
        }
        fclose(input);
        fclose(output);

        printf("End of child process...\n");
    }else{
        // parent process
        wait(NULL); // wait for child process to write input.txt to output.txt
        printf("Parent process started...\n");
        printf("Content of input.txt \n");

        FILE* input = fopen("output.txt","r");
        char c;
        while((c=fgetc(input)) != EOF){
            printf("%c",c);
        }
        printf("\n");
        fclose(input);

        printf("End of Parent process...\n");
    }

    return 0;
}