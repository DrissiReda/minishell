#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
extern int errno;
//TODO implement special args array where pipes and redirections are stored
char** parse(char* buff)
{
    char* res;  
    int i=0;
    char** ret=(char**)malloc(sizeof(char*)*255);
    while(res=strsep(&buff," "))
    {
        ret[i]=malloc(strlen(res)+1);
        strcpy(ret[i],res);
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
int find_redir(char** args)
{
    int i=0;
    while(args[i])
    {
        if(!strcmp(args[i],"<")||!strcmp(args[i],">")||!strcmp(args[i],"2>"))
            return i;
        i++;
    }
    return -1;
}
void def_cmd(char** args)
{
    int i=find_redir(args);
    int fd;
    if(i>0)
    {
        switch(args[i][0])
        {
            case '<' :
                if((fd=open(args[i+1], O_RDONLY)) < 0)
                {
                    perror(args[i+1]);
                    exit(1);
                }
                if(fd)
                {
                    dup2(fd,0);
                    close(fd);
                }
                break;
            case '>' :
                 if((fd=open(args[i+1], O_WRONLY)) < 0)
                {
                    perror(args[i+1]);
                    exit(1);
                }
                 if(fd!=1)
                {
                    dup2(fd, 1);
                    close(fd);
                }
                 break;
            case '2' :
                 if((fd=open(args[i+1], O_WRONLY)) <0)
                {
                    perror(args[i+1]);
                    exit(1);
                }
                 if(fd!=2)
                {
                    dup2(fd,2);
                    close(fd);
                }
                 break;
            }
        //cleaning
        int j=i;
        while(args[j++]);
        j--;
        while(j>=i)
        {
            free(args[j--]);
        }
    }
    execvp(args[0],args);
    perror(args[0]);
    exit(0);
}
int main(int argc,char* argv[])
{
    char* buff=malloc(255);
    char* cwd=malloc(255);
    char** args;
    cwd=getcwd(cwd,255);
    printf("%s %% ",cwd);
    while(fgets(buff,255,stdin)!= NULL)
    {

        buff[strlen(buff)-1]=0; // remove the '\n'
        args=parse(buff);
        if(args==NULL || !strcmp(args[0],""))
        {
            printf("%s %% ",cwd);
            continue;
        }
        if(!strcmp(args[0],"exit"))
        {
            printf("\n");
            break;
        }
        if(!strcmp(args[0],"cd"))
        {
            cd(args[1]);
            cwd=getcwd(cwd,255);
            printf("%s %% ",cwd);
            continue;
        }
        else
        {
        pid_t pid;
        switch(pid=fork())
        {
            case -1 : 
                perror("fork");
                errno=0;
                break;
            case 0 :
                def_cmd(args);
            case 1 :
                wait(NULL);
                //printf("%s %%",cwd);
                break;
        }
        printf("%s %%\n",getcwd(cwd,255));
        }
    }
    printf("\n");
}
