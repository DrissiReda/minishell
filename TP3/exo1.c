#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
pthread_t* T;
void* Thread(void* args)
{
    printf("Thread num %ld\n",pthread_self());
}
int main(int argc, char* argv[])
{
    int V=atoi(argv[1]);
    T=malloc(sizeof(pthread_t)*V);
    int i=0;
    for(i=0;i<V;i++)
        pthread_create(&T[i], NULL, (void*)Thread,(void*) NULL);
    for(i=0;i<V;i++)
        pthread_join(T[i], NULL);
}
