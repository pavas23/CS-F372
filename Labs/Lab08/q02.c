#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 3
#define TOTAL_ITEMS (NUM_PRODUCERS * NUM_CONSUMERS)

int buffer[BUFFER_SIZE];
int itemCount = 0;
sem_t empty, full, mutex;

void* producer(void* arg){
	int producedItems = 0;
	while(producedItems < TOTAL_ITEMS / NUM_PRODUCERS){
		// produce item
		int item = rand() % 100 + 1;
		
		sem_wait(&empty);	// wait if buffer is full
		sem_wait(&mutex);	// enter critical section
		
		// add item to buffer
		buffer[itemCount] = item;
		itemCount++;
		printf("Produced item: %d\n",item);
		
		sem_post(&mutex);	// exit critical section
		sem_post(&full);	// signal that buffer is no longer empty
		
		producedItems++;
		sleep(1); // siumulate production time
	}
	pthread_exit(NULL);
}

void* consumer(void* arg){
	int consumedItems = 0;
	while(consumedItems < TOTAL_ITEMS / NUM_CONSUMERS){
		
		sem_wait(&full);	// wait if buffer is empty
		sem_wait(&mutex);	// enter critical section
		
		// consume items from the buffer if there are items
		int item = buffer[itemCount-1];
		itemCount--;
		printf("Consumed item: %d\n", item);
		
		sem_post(&mutex);	// exit critical section
		sem_post(&empty);	// signal that buffer is no longer full
		
		consumedItems++;
		sleep(2);	// simulate consumption time
	}
	pthread_exit(NULL);
} 

int main(int argc,char* argv[]){
	pthread_t producers[NUM_PRODUCERS];
	pthread_t consumers[NUM_CONSUMERS];
	
	sem_init(&empty,0,BUFFER_SIZE);	// initialize empty semaphore to buffer size
	sem_init(&full,0,0);	// initialize full semaphore to 0
	sem_init(&mutex,0,0);	// intialize mutex semaphore to 1
	
	// create producrer threads
	for(int i=0;i<NUM_PRODUCERS;i++){
		pthread_create(&producers[i],NULL,producer,NULL);
	}
	
	// create consumer threads
	for(int i=0;i<NUM_CONSUMERS;i++){
		pthread_create(&consumers[i],NULL,consumer,NULL);
	}
	
	// join producer threads
	for(int i=0;i<NUM_PRODUCERS;i++){
		pthread_join(producers[i],NULL);
	}
	
	// join consumer threads
	for(int i=0;i<NUM_CONSUMERS;i++){
        pthread_join(consumers[i],NULL);
    }

    sem_destroy(&full);
    sem_destroy(&empty);
    sem_destroy(&mutex);	

    return 0;
}

