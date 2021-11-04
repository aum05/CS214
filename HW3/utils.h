/*****************************Header Files & Macros*****************************************/

#include<signal.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<ctype.h>
#define MAX_SIZE 1024
#define CMD_DELIMS " \t\n"
#define FNAME_LEN 20

/**********************************Function Declarations************************************/

void start_proc(int pid, char* name, char* command);
void delete_proc(int pid);
char* read_input();
int parse_input(char* command, char** cmds);
int parse_commands(char* command, char** cmd_vars);
void get_base_dir();
int run(char** cmd_vars, char* command);
void change_cwd(char* cwd);
int cd_func(char** cmd_vars, char* cwd);
void pwd(char** cmd_vars, char* command);
int open_input();
int open_output();
void kill_func(int num_cmnds, char** cmd_vars);
void init();
void sig_handler(int sig);
void fg(int num_cmnds, char** cmd_vars);
void bg(int num_cmnds, char** cmd_vars);
void cmd_executer(int num_cmnds, char** cmd_vars, char* cmd_copy);
void jobs();

/*******************************Variable And Struct Declarations******************************/

struct proc_info {
        int pid, pgid;
        char* name;
        char* command;
        int bg;
        int active;
        char stat[10];
};

typedef struct proc_info proc_info;

int job_counter;

proc_info job_list[MAX_SIZE];

char opening_dir[MAX_SIZE];
char cwd[MAX_SIZE]; /* current working directory */

char* input_file;
char* output_file;

int shell, pgid_shell;

int last;

int redirect_input, redirect_output;

char** in_commands;
char** out_commands;

pid_t main_pid, main_pgid, pid_fg;

int if_bg;
