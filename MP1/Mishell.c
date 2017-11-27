#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define MAX 100
typedef struct piped
{
	char** args;
	struct piped* next;
} piped;
extern int errno;
char* prev=NULL; 
//TODO implement special args array where pipes and redirections are stored : fixed
//TODO fix print flush error (works on correctly on gdb but not on stdout)  : fixed
//TODO implement last command entry with '\033' '[' 'A/B/C/D'  
//TODO fix pipes, error management and opening/closing streams : fixed 
void helper()
{
	printf("Welcome to Mishell 0.0.1\n");
	printf("If you need any help why don't you ask man, he's the man for the job\n");
	printf("Usage : man <command>\n");
	printf("With Mishell you can use environment variables\n");
	printf("Warning, you must separate all tokens with a space (especially pipes)\n");
}
piped* parse(char* buff,int* flag) // takes care of parsing
{
    char* res=NULL;
    int i=0;
    piped* ret=(piped*)calloc(1,sizeof(piped));
    ret->args=(char**)calloc(1,sizeof(char*)*MAX);
    ret->next=NULL;
    piped* current=ret;
    
    
    
    while(buff!=NULL && strlen(buff) && (res=strsep(&buff," ")))
    {
            switch(res[0])
            {
            case '\0' :
            	current->args[i]=calloc(1,2);
                strcpy(current->args[i],".");
                break;
            case '&' :    // background
                *flag=1;  // no need for a flag for each pipe
                continue; // since we can only add this token to the end
            case '~':  // home variable
                current->args[i]=calloc(1,strlen(getenv("HOME")+1));
                strcpy(current->args[i],getenv("HOME"));
                break;
            case '$': //env variable
                current->args[i]=calloc(1,strlen(getenv(res+1)+1));
                strcpy(current->args[i],getenv(res+1));
                break;
            case '|': // pipes, create a new node in our linked list
            	i=-1; //reset should be -1 cuz i will increment and go back to 0
            	current->next=calloc(1,sizeof(piped));
            	current=current->next;
            	current->args=(char**)calloc(1,sizeof(char*)*MAX);
            	current->next = NULL;
            	break;
            default :
                current->args[i]=calloc(1,strlen(res)+1);
                strcpy(current->args[i],res);
                break;
            }
        i++;
    }
    return ret;
}
void cd(char* dir) // executes cd command
{
    if(dir==NULL || !strcmp(dir,"") || !strcmp(dir,"~"))
    {
    	prev=getcwd(prev,MAX);
        if(chdir(getenv("HOME"))<0)
        {
        	perror("cd ");
        	errno=0;
        }
    }
    else
    {
    	if(!strcmp(dir,"-")) // restore previous pwd
    	{
    		char* current=getcwd(current,MAX);
    		chdir(prev);
    		prev=current;
    	}
    	else
    	{
    		prev=getcwd(prev,MAX);
        	if(chdir(dir)<0)
        	{
        		perror("cd ");
        		errno=0;
        	}
        }
    }
}
int find_redir(char** args) // searchs for any redirections
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
void redir(int old,int new) // redirects file descriptors
{
	if(old != new)
	{
		dup2(old,new);
		close(old);
	}
}
void do_exec(int input,int output,char** args) // executes function with custom input
{                                              // and output
	  redir(input,STDIN_FILENO);
      redir(output,STDOUT_FILENO);
      if(execvp (args[0], args)<0)
      {
      	perror(args[0]);
      	exit(1);
      }
}
// takes care of redirections/forking
int spawn_exec (int input, int output, char** args)
{
  pid_t pid;
  int i=find_redir(args);
  int fd;
    if(i>0) // redirection
    {
        switch(args[i][0])
        {
        case '<' :
            if((fd=open(args[i+1], O_RDONLY)) < 0)
            {
                perror(args[i+1]);
                exit(1);
            }
            redir(fd,STDIN_FILENO);
            break;
        case '>' :
            if((fd=open(args[i+1], O_WRONLY)) < 0)
                if((fd=creat(args[i+1], O_WRONLY)) < 0)
                {
                    perror(args[i+1]);
                    exit(1);
                }
            redir(fd,STDOUT_FILENO);
            break;
        case '2' :
            if((fd=open(args[i+1], O_WRONLY)) <0)
                if((fd=creat(args[i+1], O_WRONLY)) < 0)
                {
                    perror(args[i+1]);
                    exit(1);
                }
            redir(fd,STDERR_FILENO);
            break;
        }
        //cleaning
        /*
        int j=i;
        while(args[j++]);
        j--;
        while(j>=i)
        {
            free(args[j--]);
        }*/
        args[i]=NULL; // no need to remove every string, since all loops break at NULLs
  	}                 // global cleaning will be done at the end of main's while loop iteration
  switch(pid = fork ()) // fork
  {
  	case -1 : 
  		perror("fork");
  		errno=0;
  		break;
  	case  0 :
      	do_exec(input,output,args);
      	exit(1);
    default :
    	waitpid(pid,NULL,0);
    }
  return pid;
}
/* manages forking, executes all piped
 * commands but the last, since it outputs
 * to stdout 
 */ 
int fork_pipes (piped* cmds)
{
  int input= STDIN_FILENO; //first reads from stdin
  piped* current=cmds;
  
  while(current->next)
    {
      	int fds[2];
      	pid_t pid;
      	pipe (fds);
      	/*input is carried from previous command
      	 *output is carried to next command
      	 */
      	switch(pid=fork())
      	{
      		case -1 :
      			perror("fork");
      			exit(1);
      		case 0 :
      			close(fds[0]);
      			spawn_exec(input, fds[1], current->args);
      			exit(1);
      		default :
      			waitpid(pid,NULL,0);
      			close (fds[1]);
      			close (input);
      			input = fds[0];
      	}
      current=current->next;
    }
   return spawn_exec(input,STDOUT_FILENO, current->args);
}
void def_cmd(piped* cmds)
{
    fork_pipes(cmds);
}
void clean(piped** cmds)
{
		piped* current;
    	int i=0; // used to iterate through args
    	while(*cmds)
    	{
    		current=*cmds;
    		while(current->args[i]) // cleaning memory
    		{
        		free(current->args[i]);
        		i++;
    		}
    		free(current->args);
    		*cmds=(*cmds)->next;
    		free(current);
    	}
    	free(*cmds);
}
int main(int argc,char* argv[])
{
    char* buff=calloc(1,MAX);
    char* cwd=calloc(1,MAX);
    piped* cmds;
    int flag=0; // existence of '&'
    cwd=getcwd(cwd,MAX);
    prev=getcwd(prev,MAX);
    printf("%s %% ",cwd);
    while(fgets(buff,MAX,stdin)!= NULL)
    {
    	flag=0;
		if(strlen(buff)>0)
        	buff[strlen(buff)-1]=0; // remove the '\n'
        //printf("string is %d || %d || %d\n",(int)buff[0],(int)buff[1],(int)buff[2]);
        cmds=parse(buff,&flag);
        if(cmds->args[0]==NULL || !strcmp(cmds->args[0],""))
        {
            printf("%s %% ",cwd);
            continue;
        }
        else
        if(!strcmp(cmds->args[0],"exit"))
        {
            return 0;
        }
        else
        if(!strcmp(cmds->args[0],"cd"))
        {
            cd(cmds->args[1]);
            cwd=getcwd(cwd,MAX);
            printf("%s %% ",cwd);
            continue;
        }
        else
        if(!strcmp(cmds->args[0],"help"))
        {
        	helper();
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
                def_cmd(cmds);
                exit(1);
            default :
                if(!flag)
                    waitpid(pid,NULL,0);
                //printf("%s %%",cwd);
                break;
            }
            printf("%s %% ",getcwd(cwd,MAX));
        }
        clean(&cmds);
    }
    printf("\n");
    clean(&cmds);
    free(buff);
    free(cwd);
}
