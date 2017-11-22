#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define MAX 10000
extern int errno;
char* prev;
//TODO implement special args array where pipes and redirections are stored
//TODO fix print flush error (works on correctly on gdb but not on stdout) : fixed
//TODO implement last command entry with '\033' '[' 'A/B/C/D'
void helper()
{
	printf("Welcome to Mishell 0.0.1\n");
	printf("If you need any help why don't you ask man, he's the man for the job\n");
	printf("Usage : man <command>\n");
	printf("With Mishell you can use environment variables");
}
char** parse(char* buff,int* size,int* flag)
{
    char* res;
    int i=0;
    char** ret=(char**)malloc(sizeof(char*)*MAX);
    while(res=strsep(&buff," "))
    {
            switch(res[0])
            {
            case '&' : // background
                *flag=1;
                continue;
            case '~':  // home variable
                ret[i]=malloc(strlen(getenv("HOME")+1));
                strcpy(ret[i],getenv("HOME"));
                break;
            case '$': //env variable
                ret[i]=malloc(strlen(getenv(res+1)+1));
                strcpy(ret[i],getenv(res+1));
                break;
            default :
                ret[i]=malloc(strlen(res)+1);
                strcpy(ret[i],res);
                break;
            }

        i++;
    }
    *size=i;
    if(i>0 && !strcmp(ret[i-1],""))
        ret[i-1]=NULL;
    return ret;
}
void cd(char* dir)
{
    if(dir==NULL || !strcmp(dir,"") || !strcmp(dir,"~"))
    {
    	prev=getcwd(prev,MAX);
        chdir(getenv("HOME"));
    }
    else
    {
    	if(!strcmp(dir,"-")) // restore previous pwd
    	{
    		char* tmp=getcwd(tmp,MAX);
    		chdir(prev);
    		prev=tmp;
    	}
    	else
    	{
    		prev=getcwd(prev,MAX);
        	chdir(dir);
        }
    }
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
void def_cmd(char** args,int* size)
{
    int i=find_redir(args);
    *size=i;
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
            perror(args[i+1]);
            errno=0;
            if(fd)
            {
                dup2(fd,0);
                close(fd);
            }
            break;
        case '>' :
            if((fd=open(args[i+1], O_WRONLY)) < 0)
                if((fd=creat(args[i+1], O_WRONLY)) < 0)
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
                if((fd=creat(args[i+1], O_WRONLY)) < 0)
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
        /*
        int j=i;
        while(args[j++]);
        j--;
        while(j>=i)
        {
            args[j--]=NULL;
        }*/
        args[i]=NULL; // no need to remove every string, since all loops break at NULLs
    }
    i=0;
    int fds[2];
    while(args[i])
    {
        if(args[i][0]=='|')
        {
            pipe(fds);
            pid_t W;
            switch(W=fork())
            {
            case -1 :
                perror("fork");
                errno=0;
                break;
            case 0  :
                dup2(fds[0],0);
                close(fds[1]);
                close(fds[0]);
                *size -= i;
                def_cmd(args+i+1, size);
                break;
            default :
                dup2(fds[1],1);
                close(fds[1]);
                close(fds[0]);
                args[i]= NULL;
                //waitpid(W,NULL,WUNTRACED);
                break;
            }
        }
        i++;
    }
    execvp(args[0],args);
    perror(args[0]);
    exit(0);
}
int main(int argc,char* argv[])
{
    char* buff=malloc(MAX);
    char* cwd=malloc(MAX);
    char** args;
    int args_size;
    int flag=0; // existence of '&'
    cwd=getcwd(cwd,MAX);
    prev=getcwd(prev,MAX);
    printf("%s %% ",cwd);
    while(fgets(buff,MAX,stdin)!= NULL)
    {

        buff[strlen(buff)-1]=0; // remove the '\n'
        printf("string is %d || %d || %d\n",(int)buff[0],(int)buff[1],(int)buff[2]);
        args=parse(buff,&args_size,&flag);
        if(args[0]==NULL || !strcmp(args[0],""))
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
            cwd=getcwd(cwd,MAX);
            printf("%s %% ",cwd);
            continue;
        }
        if(!strcmp(args[0],"help"))
        	helper();
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
                def_cmd(args,&args_size);
            default :
                if(!flag)
                    waitpid(pid,NULL,0);
                //printf("%s %%",cwd);
                break;
            }
            printf("%s %% ",getcwd(cwd,MAX));
        }
    }
    printf("\n");
    flag=0;
    while(args[flag]) // cleaning memory
    {
        free(args[flag]);
        flag++;
    }
    flag=0;
    free(args);
}
