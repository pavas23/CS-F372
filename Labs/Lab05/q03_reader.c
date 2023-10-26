#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#define BUF_SIZE 200
#define PERMS 0666

int main(int argc,char* argv[]){
    key_t key;
    int shmid;
    char* shmptr;

    char buffer[BUF_SIZE];

    if((key=ftok(".",10)) == -1){
        perror("ftok");
        exit(1);
    }

    shmid = shmget(key,sizeof(char[BUF_SIZE]),PERMS);
    if(shmid == -1){
        perror("SHMID Error");
        exit(1);
    }

    shmptr = (char*)shmat(shmid,NULL,0);
    if(shmptr == (void*)-1){
        perror("SHMPTR Error");
        exit(1);
    }

    printf("Reader reading from shared memory\n");
    while(1){
        if(shmptr[0] != '\0'){
            strcpy(buffer,shmptr);
            printf("Reader: %s\n",buffer);
            memset(shmptr,'\0',sizeof(shmptr));
        }
    }

    printf("Reader detaching from shared memory\n");
    if(shmdt(shmptr) == -1){
        perror("Detaching error");
        exit(1);
    }

    if(shmctl(shmid,IPC_RMID,0) == -1){
        perror("SHMCTL");
        exit(1);
    }

    return 0;
}
