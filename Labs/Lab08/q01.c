#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>

#define NUM_THREADS 5
#define SHARED_RESOURCE_SIZE 2 // we have 2 instances of the resource

int sharedResource[SHARED_RESOURCE_SIZE] = {0,0};
sem_t semaphore;

void* thread_function(void* arg){
	int thread_id = *((int*)arg);
	int index = thread_id % SHARED_RESOURCE_SIZE;
	
	sem_wait(&semaphore); // wait for semaphore to access the
	
	// Modify the shared resource
	sharedResource[index]++;
	printf("Thread %d modified sharedResource[%d] : %d\n",thread_id, index, sharedResource[index]);
	
	// Simulate work by sleeping for 2 seconds
	sleep(2);
	
	sem_post(&semaphore); // release semaphore 
	
	return NULL;
}

int main(int argc,char* argv[]){
	pthread_t threads[NUM_THREADS];
	int thread_ids[NUM_THREADS];
	
	// initialize the semaphore
	sem_init(&semaphore,0,SHARED_RESOURCE_SIZE);
	
	// create threads
	for(int i=0;i<NUM_THREADS;i++){
		thread_ids[i] = i;
		pthread_create(&threads[i],NULL,thread_function,(void*)&thread_ids[i]);
	}
	
	// join threads
	for(int i=0;i<NUM_THREADS;i++){
		pthread_join(threads[i],NULL);
	}
	
	// destroy the semaphore
	sem_destroy(&semaphore);
	return 0;
}
