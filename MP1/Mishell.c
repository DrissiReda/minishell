
#include "Mishell.h"

char* buffer;
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
            case '\0' : // replace empty strings with pwd for ls
            	if(i==1 && !strcmp(current->args[0],"ls")) //only works for first argument
            	{										   // to avoid bugs
            		current->args[i]=calloc(1,2);
                	strcpy(current->args[i],".");
                }
                else // ignore
                {
                	i--;
                }
                break;
            case '&' :    // background
                if(*flag!=2) *flag=1;  // only set the flag if no other flag in other pipes
                continue; // since we can only add this token to the end
            case '~':  // home variable
                current->args[i]=calloc(1,strlen(getenv("HOME")+1));
                strcpy(current->args[i],getenv("HOME"));
                current->args[i+1]=NULL; //last arg is a null
                break;
            case '$': //env variable
            	if(getenv(res+1)==NULL) //proper behavior for undefined variables
            	{	
            		i--;
            		break;
            	}
            	else // normal behavior
            	{
                	current->args[i]=calloc(1,strlen(getenv(res+1))+1);
                	strcpy(current->args[i],getenv(res+1));
                	current->args[i+1]=NULL;
                }
                break;
            case '|': // pipes, create a new node in our linked list
            	i=-1; //reset should be -1 cuz i will increment and go back to 0
            	current->next=calloc(1,sizeof(piped)); //linked list manipulation
            	current=current->next;
            	current->args=(char**)calloc(1,sizeof(char*)*MAX);
            	current->args[0]=NULL;
            	current->next = NULL;
            	if(*flag==1)*flag=2; // should produce an error if not last piped command
            	break;
            case ' ': //ignore
            	i--;
            	break;
            case '"' :
            	res++; //get rid of the leading "
            	if(res[strlen(res)-1]=='\"') // one word
            	{
            		res[strlen(res)-1]=0;
            	}
            	else
            		sprintf(res,"%s %s",res,strsep(&buff,"\"")); 
            	//no need to break we need the default
            default : // general arguments case
                current->args[i]=calloc(1,strlen(res)+1);
                strcpy(current->args[i],res);
                current->args[i+1]=NULL;
                break;
            }
        i++;
    }
    return ret;
}
void cd(char* dir,char** prev) // executes cd command
{
	char* pwd=calloc(1,MAX);
	if(dir != NULL)
	{
		if(!strcmp(dir,getcwd(pwd,MAX)))//no changing of pwd if dir is the same
		{
			printf("Done");
			free(pwd);
			return;
		}
		else 
    	{	
    		if(!strcmp(dir,"-")) // restore previous pwd
    		{
    			if(*prev==NULL)
    				fprintf(stderr,"Mishell: cd: OLDPWD not set\n");	
    			else
    			{
    				char* current=getcwd(current,MAX);
    				chdir(*prev);
    				*prev=current;
    			}
    		}
    		else // general cd case
    		{
    			*prev=getcwd(*prev,MAX);
        		if(chdir(dir)<0)
        		{
        			perror("cd ");
        			errno=0;
        		}
        	}
    	}
	}
	
    if(dir==NULL || !strcmp(dir,"") || !strcmp(dir,"~"))//cd to home var
    {
    	if(!strcmp(getenv("HOME"),getcwd(pwd,MAX)))
    	{// no changing to home if we're already there
    		free(pwd);
    		return;
    	}
    	*prev=getcwd(*prev,MAX);
        if(chdir(getenv("HOME"))<0) 
        {
        	perror("cd ");
        	errno=0;
        }
    }
    free(pwd);
}
void hist()
{
	FILE *h;
    char c;
    h=fopen(".history","rt");
    //printing char by char, so that fgets doesn'ts miss the last line
    while((c=fgetc(h))!=EOF){
        printf("%c",c);
    }

    fclose(h);	
}
void add_hist(char* cmd,int* index)
{
	FILE* h;
	if(!(h=fopen(".history",(*index < HISTMAX)?"a" : "w")))
	{ // overwrite if history longer than histmax
		perror("history_add ");
		errno=0;
	}
	time_t current;
	struct tm* local;
	current=time(NULL);
	char buffer[100];
	local= localtime(&current);
	*index=(*index<HISTMAX)?*index+1:1;
	strftime(buffer,100,"%d/%m/%Y %T",local);
	fprintf(h,"%d %s %s\n",*index,buffer,cmd);
	fclose(h);
}
node* find_redir(char** args) // searchs for all redirections
{
    node* ret=NULL;
    node* current=NULL;
    ret=calloc(1,sizeof(node));
    ret->i=0;
    current=ret;
    int i=0;
    while(args[i]) //while args is not null
    {
        if(!strcmp(args[i],"<")||!strcmp(args[i],">")||!strcmp(args[i],"2>")  
        	|| !strcmp(args[i],">>") )
        {
        	current->i=i;
        	current->next=calloc(1,sizeof(node));
        	current=current->next;
        }
        i++;
    }
    return ret;
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
      if(execvp (args[0], args)<0) // displays error if not successful
      {
      	perror(args[0]);
      	exit(1);
      }
}
// takes care of redirections/forking
int spawn_exec (int input, int output, char** args)
{
  pid_t pid;
  node* redirections=find_redir(args);
  node* head=redirections;
  int fd;
    while(redirections->i && redirections->next != NULL) // redirection
    {
    	if(args[redirections->i + 1]== NULL)
    	{
    		fprintf(stderr,"Mishell: syntax error near unexpected token `newline'\n");
    		return -1;
    	}
    	if(args[redirections->i+1][0]=='2' || args[redirections->i+1][0]=='<' || 
    		args[redirections->i+1][0]=='>' )
    	{
    		fprintf(stderr,"Mishell: syntax error near unexpected token `%c'\n",
    			args[redirections->i+1][0]);
    		return -1;
    	}
        switch(args[redirections->i][0])// each case has a different first char  
        {                 // so switch makes sense
        case '<' : // stdin
            if((fd=open(args[redirections->i+1], O_RDONLY)) < 0)
            {
                perror(args[redirections->i+1]);
                exit(1);
            }
            redir(fd,STDIN_FILENO);
            break;
        case '>' : // stdout
            if((fd=open(args[redirections->i+1],(args[redirections->i][1]=='>')
            	?(O_WRONLY | O_APPEND):(O_WRONLY | O_CREAT | O_TRUNC) , 0644)) < 0)
                if((fd=creat(args[redirections->i+1], O_WRONLY | O_TRUNC)) < 0)
                {
                    perror(args[redirections->i+1]);
                    exit(1);
                }
            redir(fd,STDOUT_FILENO);
            break;
        case '2' : // stderr
            if((fd=open(args[redirections->i+1], O_WRONLY | O_TRUNC)) <0)
                if((fd=creat(args[redirections->i+1], O_WRONLY | O_TRUNC)) < 0)
                {
                    perror(args[redirections->i+1]);
                    exit(1);
                }
            redir(fd,STDERR_FILENO);
            break;
        }
        args[redirections->i]=NULL; // no need to remove every string, since all loops break at NULLs
  		redirections=redirections->next;
  	}                 // global cleaning will be done at the end of main's while loop iteration
  switch(pid = fork ()) // fork
  {
  	case -1 : //error
  		perror("fork");
  		errno=0;
  		break;
  	case  0 : // child executes cmd
      	do_exec(input,output,args);
      	exit(1);
    default :
    	waitpid(pid,NULL,0);
    }
  clean_node(&head); // cleaning
  return pid;
}
/* manages forking, executes all piped
 * commands
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
int def_cmd(piped* cmds,int* flag)
{
	if(*flag==2)
	{
		fprintf(stderr,"Mishell: syntax error near unexpected token `|'\n");
		return -1;
	}
	else
    	return fork_pipes(cmds);
}
void clean_piped(piped** cmds)
{
		piped* current;
    	int i=0; // used to iterate through args
    	while(*cmds && (*cmds)->args!=NULL)
    	{
    		current=*cmds;
    		while(current->args[i]) // cleaning the array of strings
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

void clean_node(node** list)
{
		node* current;
    	while(*list)
    	{
    		current=*list;
    		*list=(*list)->next;
    		free(current);
    	}
    	free(*list);
}
int getcount()
{
	char buff[MAX];
	int count=0;
	FILE* h=fopen(".history","r");
	if(h==NULL)
		return 0;
	while(fgets(buff,MAX,h) !=NULL)
		count++;
	fclose(h);
	return count;
}
char* match_generator (const char *uncomplete, int state)
{
    static int i, size;
    const char *current;

    if (!state)
    {
        i = 0;
        size = strlen (uncomplete);
    }
    while (current=dict[i])
    {
        i++;
        //strncmp is so we only recover the written part of the uncomplete word
        if (strncmp (current, uncomplete, size) == 0) return strdup (current);
    }
    // If no words from the dictionary matched, then return NULL.
    return NULL;
}

// Custom completion function
static char **completion (const char *text, int start, int end)
{
    // This prevents appending space to the end of the matching word
    rl_completion_append_character = '\0';

    char **matches = (char **) NULL;
    if (start == 0)
    {
        matches = rl_completion_matches ((char *) text, &match_generator);
    }
    return matches;
}
static char* last_command(int count, int key)
{
	FILE* f=fopen(".history","r");
	fseek(f, -1, SEEK_END);// char before last one (EOF)
	char c;
	int i=0,size=1; // get size of the line
	char* line=calloc(1,MAX);
	for(i=0;i<count;i++)
	while (c!= '\n') // find the beginning of the line
	{
		fseek(f, -2, SEEK_CUR);
		c= fgetc(f);
		size++;
	}
	fgets(line, size, f);
	strsep(&line," ");//skip command number
	strsep(&line," ");//skip date
	strsep(&line," ");//skip seconds
	if(line)
	{
		printf("%s",line);
	} 
	return NULL;
	
}	
int main(int argc,char* argv[])
{
    
	int index=getcount();
    char* cwd=calloc(1,MAX);
    char* prevcd=NULL; 
    piped* cmds=calloc(1,sizeof(piped));
    cmds->args=NULL;
    int flag=0; // existence of '&'
    cwd=getcwd(cwd,MAX);
    sprintf(cwd,"%s %% ",cwd);
    rl_attempted_completion_function = completion;
    while(buffer=readline(cwd))
    {
    	if(strcmp(buffer,""))
    		add_hist(buffer, &index);
    	flag=0;
        cmds=parse(buffer,&flag);
        if(cmds->args[0]==NULL || !strcmp(cmds->args[0],""))
        {
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
            cd(cmds->args[1],&prevcd);

            sprintf(cwd,"%s %% ",getcwd(cwd,MAX));
            continue;
        }
        else
        if(!strcmp(cmds->args[0],"help"))
        {
        	helper();
            continue;
        }
        else
        if(!strcmp(cmds->args[0],"history"))
        {
        	hist();
        	continue;
        }
        {
            pid_t pid;
            switch(pid=fork())
            {
            case -1 :
                perror("fork");
                errno=0;
                break;
            case 0 :
                def_cmd(cmds,&flag);
                exit(1);
            default :
                if(!flag)
                    waitpid(pid,NULL,0);
                else if (flag==1)
                	printf("Mishel: background pid :%d\n",pid);
                break;
            }
        }
        clean_piped(&cmds);
        free(buffer);
    }
    printf("\n");
    free(buffer);
    free(cwd);
}
