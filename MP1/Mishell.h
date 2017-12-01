#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <readline/readline.h>

#define MAX 1000
#define HISTMAX 500
typedef struct piped
{
	char** args;
	struct piped* next;
} piped;
typedef struct node
{
	int i;
	struct node* next;
}node ;
extern int errno;
const char *dict[] = {"ls", "grep", "rm", "mkdir", "gcc", "rmdir", "touch", "cd",
					   "make", "vim", "help","exit","gdb", "sed", "bash", "history"};
void helper();
void cd(char* directory, char** old_directory);
void hist();
void add_hist(char* command, int* index);


piped* parse(char* buffer, int* flag);


node* find_redir(char** arguments);
void redir(int oldfd, int newfd);
void do_exec(int input_fd, int output_fd, char** arguments);
int spawn_exec(int input_fd,int output_fd, char** arguments);
int fork_pipes(piped* commands);
int def_cmd(piped* commands, int* flag);


void clean_piped(piped** commands);
void clean_node(node** list);

