#include<stdlib.h>

int main()
{
    pid_t pid=fork();
    int status=0;
    if(pid)
    {
        int i=0;
        for(i=0;i<10000;i++)
            write(1,"a ",3);
        wait(&status);
    }
    else
    {
        int j=0;
        for(j=0;j<10000;j++)
            write(1,"b ",3);
        exit(0);
    }

}
