#include<stdio.h>
#include<pthread.h>

pthread_mutex_t write;
pthread_mutex_t read;
int cnt = 1;
int numreader = 0;

void* writer(void* wno){
	pthread_mutex_lock(&write);
	cnt = cnt*2;
	printf("Writer %d modified cnt to %d\n",(*((int*)wno)),cnt);
	pthread_mutex_unlock(&write);
}

void* reader(void* rno){
	pthread_mutex_lock(&read);
	numreader++; // increase the reader count
	if(numreader == 1){
		pthread_mutex_lock(&write); // stop the writer from writing
	}
	pthread_mutex_unlock(&read);
	
	printf("Reader %d: read cnt as %d\n",*((int*)rno),cnt);
	
	pthread_mutex_lock(&read);
	numreader--;
	if(numreader == 0){
		pthread_mutex_unlock(&write); // release the writer lock
	}
	pthread_mutex_unlock(&read);
}

int main(void){
	pthread_t readerThreads[5], writerThreads[5];
	pthread_mutex_init(&read,NULL);
	pthread_mutex_init(&write,NULL);
	
	int a[5] = {1,2,3,4,5}; // reader and writer numbers
	
	for(int i=0;i<5;i++){
		pthread_create(&readerThreads[i], NULL, (void*)reader, (void*)&a[i]);
		pthread_create(&writerThreads[i], NULL, (void*)writer, (void*)&a[i]);
	}
	
	for(int i=0;i<5;i++){
		pthread_join(readerThreads[i], NULL);
		pthread_join(writerThreads[i], NULL);
	}
	
	pthread_mutex_destroy(&read);
	pthread_mutex_destroy(&write);
	
	return 0;
}
