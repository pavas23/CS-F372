#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> // for execlp()
#include<sys/types.h>
#include<sys/wait.h>

int main(int argc,char* argv[]){
    execlp("cat","read",argv[1],NULL); 
    // This will execute the command specified
    // execlp will replace the whole code after this line with a new image of the file to be executed given in the command execlp
    printf("After displaying file contents\n"); 
    // this won't be displayed as code gets replaced with new image
    return 0;
}