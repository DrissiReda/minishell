#include<stdlib.h>

int main()
{
    pid_t pid=fork();
    int status=0;
    if(pid)
    {
        write(1,"I am your father\n",17);
        wait(&status);
    }
    else
    {
        write(1,"I am his son\n",13);
        exit(0);
    }

}
