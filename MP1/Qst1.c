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
//TODO implement special args array where pipes and redirections are stored
//TODO fix print flush error (works on correctly on gdb but not on stdout) : fixed
//TODO implement last command entry with '\033' '[' 'A/B/C/D'
char** parse(char* buff,int* size,int* flag)
{
    char* res;  
    int i=0;
    char** ret=(char**)malloc(sizeof(char*)*MAX);
    while(res=strsep(&buff," "))
    {
    	if(res[0]=='&')  // background
        {	
        	*flag=1;
        	 continue;
        }
        ret[i]=malloc(strlen(res)+1);
        strcpy(ret[i],res);
        
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
    		switch(fork())
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
    printf("%s %% ",cwd);
    while(fgets(buff,MAX,stdin)!= NULL)
    {

        buff[strlen(buff)-1]=0; // remove the '\n'
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
                	waitpid(pid,NULL,WUNTRACED);
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
