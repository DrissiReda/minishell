#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
typedef struct node
{
    char* data;
    struct node* next;
} node;
node* parse(char* buff)
{
    char* res;
    node* ret;
    node* tmp;
    int i=0;
    ret=(node*)malloc(sizeof(node));
    tmp=ret;
    while(res=strsep(&buff," "))
    {
        tmp->data=malloc(strlen(res)+1);
        strcpy(tmp->data,res);
        tmp->next=malloc(sizeof(node));
        tmp=tmp->next;
        i++;
    }
    return ret;
}
void cd(char* dir)
{
    if(dir==NULL || !strcmp(dir,"") || !strcmp(dir,"~"))
        chdir(getenv("HOME"));
    else
        chdir(dir);
}
int main(int argc,char* argv[])
{
    char* buff=malloc(255);
    char* cwd=malloc(255);
    node* args;
    cwd=getcwd(cwd,255);
    printf("\n%s %% ",cwd);
    while(fgets(buff,255,stdin)!= NULL)
    {
        args=parse(buff);
        int i=0;
        node* tmp=args;
        while(tmp != NULL){
            printf("%s\n",tmp->data);
            tmp=tmp->next;
        }
        if(args==NULL || !strcmp(args->data,""))
        {
            printf("\n%s %% ",cwd);
            continue;
        }
        if(!strcmp(args->data,"exit"))
        {
            printf("\n");
            break;
        }
        if(!strcmp(args->data,"cd"))
        {
            cd(args->next->data);
            cwd=getcwd(cwd,255);
            printf("\n%s %% ",cwd);
            continue;
        }
        printf("\n %s %% ",cwd);
    }
}
