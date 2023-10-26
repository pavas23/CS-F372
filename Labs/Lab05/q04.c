#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<errno.h>

#define SHM_KEY 0x1234
#define BUF_SIZE 250
#define PERMS 0644 // file permission

int main(int argc,char* argv[]){
    char* firstFileName = argv[1];
    char* secondFileName = argv[2];
    char* thirdFileName = argv[3];

    // defining shared memory id and shared memory pointer
    int shmid;
    char* shmptr;
    char buffer[BUF_SIZE];

    FILE* ptr;

    /* allocating space of BUF_SIZE*2 to create partition to store content of two files
    i.e first file content in first BUF_SIZE bytes and second file content starting from shmptr +BUF_SIZE, in the second half */

    shmid = shmget(SHM_KEY,sizeof(char[BUF_SIZE*2]),PERMS | IPC_CREAT);
    if(shmid == -1){
        perror("SHMID error!!");
        exit(1);
    }

    shmptr = (char*)shmat(shmid,NULL,0);
    if(shmptr == (void*)-1){
        perror("SHMPTR error!!");
        exit(1);
    }

    pid_t pid = fork();
    if(pid == -1){
        perror("fork");
        return 1;
    }

    if(pid == 0){
        // first child process
        printf("First child process writing contents of %s into shared memory...\n",firstFileName);
        shmptr[0] = '\0';

        ptr = fopen(firstFileName,"r");
        if(ptr == NULL){
            perror("first file can't be open!!");
            exit(1);
        }

        while (fgets(buffer,sizeof(buffer),ptr) != NULL){
            strcat(shmptr,buffer);
        }
        fclose(ptr);

        printf("First child process detaching from shared memory...\n");
        if(shmdt(shmptr) == -1){
            perror("Detaching error!!");
            exit(1);
        }
    }else{
        // parent process
        if(fork() == 0){
            sleep(1);
            // second child process
            printf("Second child process writing contents of %s into shared memory...\n",secondFileName);

            ptr = fopen(secondFileName,"r");
            if(ptr == NULL){
                perror("second file can't be open!!");
                exit(1);
            }

            // writing contents in shared memory starting from memory location pointed by shmptr+BUF_SIZE pointer
            while (fgets(buffer,sizeof(buffer),ptr) != NULL){
                strcat(shmptr+BUF_SIZE,buffer);
            }
            fclose(ptr);

            printf("Second child process detaching from shared memory...\n");
            if(shmdt(shmptr) == -1){
                perror("Detaching error!!");
                exit(1);
            }

        }else{
            // parent process
            // writing content of shared memory in third file
            wait(NULL); // wait for first child process to finish
            wait(NULL); // wait for second child process to finish

            printf("Parent process writing the contents into the third file...\n");
            ptr = fopen(thirdFileName,"w");

            if(ptr == NULL){
                perror("The file is not opened!!");
                exit(1);
            }

            fputs("Contents of first file are:\n",ptr);
            fputs(shmptr,ptr);
            fputs("\n\nContents of second file are:\n",ptr);
            fputs(shmptr+BUF_SIZE,ptr);
            fclose(ptr);

            printf("File Written Successfully !!\n");

            // delete the shared memory segment
            if(shmctl(shmid,IPC_RMID,0) == -1){
                perror("SHMCTL");
                exit(1);
            }
        }
    }
    return 0;
}

