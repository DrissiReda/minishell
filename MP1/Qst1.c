#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>

int main(int argc,char* argv[])
{
    char buffer[255]={0};
    while(fgets(buffer,255,stdin)!= NULL)
    {
        printf("% ");
        printf("%s\n",buff);
    }
