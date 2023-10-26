#include<pthread.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>

pthread_mutex_t lock;
int arr[5];
int ind = 0;

void* solve(void* arg){
	pthread_mutex_lock(&lock);
	ind++;
    // sleep makes the current thread to suspend execution for a specified period, this is efficient way of making processor time available for other threads, that's why pthread_join returns after sleep and other threads execute.
	sleep(1);
	arr[ind-1] = ind;
	pthread_mutex_unlock(&lock);
}

int main(void){
	pthread_t threads[5];
	pthread_mutex_init(&lock,NULL);
	
	int a[5] = {1,2,3,4,5}; // Thread Number
	
    // creating threads
	for(int i=0;i<5;i++){
		pthread_create(&threads[i],NULL,(void*)solve,(void*)&a[i]);
	}
	
    // pthread_join will make the main thread wait until the target thread completed its execution
	for(int i=0;i<5;i++){
		pthread_join(threads[i],NULL);
	}
	
	pthread_mutex_destroy(&lock);
	
	// printing the values of the global array arr
	for(int i=0;i<5;i++){
		printf("%d ",arr[i]);
	}
	
	printf("\n");	
	return 0;
}