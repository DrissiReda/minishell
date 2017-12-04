#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
pthread_t* T;
pthread_mutex_t mutex;
int counter=0;
void* Thread(void* args)
{
    pthread_mutex_lock(&mutex);
    printf("Thread num %d\n",++counter);
    pthread_mutex_unlock(&mutex);
}
int main(int argc, char* argv[])
{
    pthread_mutex_init(&mutex, NULL);
    int V=atoi(argv[1]);
    T=malloc(sizeof(pthread_t)*V);
    int i=0;
    for(i=0;i<V;i++)
        pthread_create(&T[i], NULL, (void*)Thread,(void*) NULL);
    for(i=0;i<V;i++)
        pthread_join(T[i], NULL);
}
