#include<stdio.h>
#include<stdlib.h>
#include<time.h>
int main(int argc,char* argv[])
{
    int n=(int)atoi(argv[1]); 
    srand(time(NULL));
    int* T=(int)malloc(sizeof(int)*n);
    int status=0;
    pid_t pid=fork();
    if(pid)
    {
        int i;
        for(i=n/2;i<n;i++)
        {
            T[i]=rand()%1001;
            if(T[i]==42)
                printf("%d ",i);
        }
        wait(&status);
    }
    else
    {
        int j;
        for(j=0;j<n/2;j++)
        {    
            T[j]=rand()%1001;
            if(T[j]==42)
                printf("%d ",j);
        }
        printf("\n");
        exit(0);
    }
}

