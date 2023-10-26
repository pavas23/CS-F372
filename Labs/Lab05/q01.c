#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#define SHM_KEY 0x1234
#define BUF_SIZE 8
#define PERMS 0644 // file permission

int main(int argc,char* argv[]){
    int shmid;
    int* shmptr;

    if(fork() == 0){
        // child process
        sleep(5);
        shmid = shmget(SHM_KEY,BUF_SIZE,PERMS);
        if(shmid == -1){
            perror("SHMID Error");
            exit(1);
        }

        shmptr = shmat(shmid,NULL,0);

        if(shmptr == (void*)-1){
            perror("SHMPTR Error");
            exit(1);
        }

        printf("Child Reading shared memeory....\n");
        if(shmptr[0] == getppid()){
            printf("The ID of parent process is %d\n",shmptr[0]);
        }

        printf("Child reading done, detaching from shared memory\n");
        if(shmdt(shmptr) == -1){
            perror("Detaching error!!");
            exit(1);
        }
    }else{
        // parent process
        // shmget is used to create a new memory segment or locate an existing one with the key
        shmid = shmget(SHM_KEY,BUF_SIZE,PERMS | IPC_CREAT);
        if(shmid == -1){
            perror("SHMID Error!!");
            exit(1);
        }

        shmptr = shmat(shmid,NULL,0);

        if(shmptr == (void*)-1){
            perror("SHMPTR Error!!");
            exit(1);
        }

        printf("Parent %d writing to shared memory....\n",getpid());

        shmptr[0] = getpid(); // writing to shared memory
        printf("Parent writing done, detaching from shared memory\n");

        if(shmdt(shmptr) == -1){
            perror("SHMDT Error");
            exit(1);
        }

        wait(NULL); // will wait for child process to read from memory before deleting the shared memory

        if(shmctl(shmid,IPC_RMID,0) == -1){
            perror("SHMCTL");
            exit(1);
        }
    }

    return 0;
}
