#include "utils.h"

/*********************** Functions for initailizing the shell ***********************/

// Function to the get the present working directory where the shell program is run
void get_base_dir() {
    getcwd(base_dir,  MAX_BUF_LEN - 1); 
    strcpy(cwd, base_dir);
}

void set_prompt() {
    printf("> ");
}

// Function that modifies the PWD relative to assumed home directory
void mod_cwd_rel(char* cwd) {
    int i, j;

    for(i = 0; cwd[i]==base_dir[i] && cwd[i]!='\0' && base_dir[i] != '\0'; i++);

    if(base_dir[i] == '\0'){
        cwd[0] = '~';
        for(j = 1; cwd[i]!='\0'; j++) {
            cwd[j] = cwd[i++];
        }
        cwd[j] = '\0';
    }
}
// Function that handles signals
void signal_handler(int signum) {
    if(signum == SIGINT) {
        signal(SIGINT,SIG_IGN);               /* For ignoring ctrl + c */
        signal(SIGINT, signal_handler);       /* For re-setting signal handler */
    }
    else if(signum == SIGCHLD) {                  /* For handling signal from child processes */
        int i, status, die_pid;
        while((die_pid = waitpid(-1, &status, WNOHANG)) > 0) {  /* Get id of the process which has terminated  */
            for(i = 0; i < num_jobs; i++){
                if(table[i].active==0) 
                    continue;
                else if(table[i].pid == die_pid)
                    break;
            }
            if(i != num_jobs) {
                if(WIFSIGNALED(status))   /* returns true if the child process was terminated by a signal */
                    fprintf(stdout, "\n%s with pid %d has exited with signal\n", table[i].name, table[i].pid);
                table[i].active = 0;
            }
        }
    }
}

// Initialize the shell
void initializer() {
    shell = STDERR_FILENO;                         /* FD for stderr */

    num_jobs = 0;

    input_cmd_tokens = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN);
    output_cmd_tokens = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN);            /* Initialisations and allocations */

    if(isatty(shell)) {                                             /* test whether a stderr refers to a terminal */
        while(tcgetpgrp(shell) != (shell_pgid = getpgrp())){        /* if it does, send signal to make process group or executable same as process group of stderr */
            kill(shell_pgid, SIGTTIN);                              /* SIGTTIN sets terminal input for background processes */
        }             
                                                                    
                                                 
    }

    signal (SIGINT, SIG_IGN);                                    /* To ignore Ctrl c */

    signal (SIGTSTP, SIG_IGN);                                   /* To ignore Ctrl z */

    signal (SIGQUIT, SIG_IGN);                                   /* To ignore Ctrl \ */

    signal (SIGTTIN, SIG_IGN);                                   /* To ignore background processes */
    
    signal (SIGTTOU, SIG_IGN);

    my_pid = my_pgid = getpid();                                 /* Set pgid of executable same as pid */
    setpgid(my_pid, my_pgid);
    tcsetpgrp(shell, my_pgid);                                   /* Give control of stderr to executable's process group */
    
    get_base_dir();
    mod_cwd_rel(cwd);                                 /* modify current working directory relative to assumed home directory */
}

/*********************** Parser functions ***********************/
// The first two functions handle input output redirections in piping

// Function to open input file
int open_infile() {
    int f = open(infile, O_RDONLY, S_IRWXU);
    if (f < 0) {
        perror(infile);  
    }
    dup2(f, STDIN_FILENO);
    close(f);
    return f;
}

// Function to open output file
int open_outfile() {
    int f;
    if(last == 1) f = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
    else if(last == 2) f = open(outfile, O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
    if(f < 0) {
            perror(outfile);
    }
    dup2(f, STDOUT_FILENO);
    close(f);
    return f;
}

char* read_cmdline() {
    int len=0,c;            
    char* cmd = malloc(sizeof(char)*MAX_BUF_LEN);
    while(1) {
        c = getchar();
        if(c == '\n') {
            cmd[len++] = '\0';
            break;
        }
        else
            cmd[len++] = c;
    }
    return cmd;
}

int parse_cmd_line(char* cmdline, char** cmds) {
    int num_cmds = 0;
    char* token = strtok(cmdline, ";");
    while(token!=NULL) {
        cmds[num_cmds++] = token;
        token = strtok(NULL, ";");
    }
    return num_cmds;
}

int parse_cmd(char* cmd, char** cmd_tokens) {
    int tok = 0;
    char* token = strtok(cmd, CMD_DELIMS);
    while(token!=NULL) {
        cmd_tokens[tok++] = token;
        token = strtok(NULL, CMD_DELIMS);
    }
    return tok;
}

int check_for_pipe(char* cmd) {
    int i;
    idxi = idxo = last = piping = input_redi = output_redi = 0;
    for( i = 0 ; cmd[i] ; i++) {
        if(cmd[i] == '|') {
            piping = 1;
        }
        if(cmd[i] == '<') {
            input_redi = 1;
            if(idxi == 0 ) idxi = i;
        }
        if(cmd[i] == '>') {
            output_redi = 1;
            if(last == 0) last = 1;
            if(idxo == 0 ) idxo = i;
        }
        if(cmd[i] == '>' && cmd[i+1] == '>') last = 2;
    }
    if(piping) return 1;
    else return -1;
}

void parse_for_piping(char* cmd) {
    char* copy = strdup(cmd);
    char* token;
    int tok = 0;
    token = strtok(copy, "|");
    while(token!= NULL) {
        pipe_cmds[tok++] = token;
        token = strtok(NULL, "|");
    }
    num_pipe = tok;
}

int parse_for_redi(char* cmd, char** cmd_tokens) {
    char* copy = strdup(cmd);
    idxi = idxo = last = input_redi = output_redi = 0;
    infile = outfile = NULL;
    int i, tok = 0;
    for( i = 0 ; cmd[i] ; i++) {
        if(cmd[i] == '<') {
                input_redi = 1;
                if(idxi == 0 ) idxi = i;
        }
        if(cmd[i] == '>') {
                output_redi = 1;
                if(last == 0) last = 1;
                if(idxo == 0 ) idxo = i;
        }
        if(cmd[i] == '>' && cmd[i+1] == '>') last = 2;
    }
    if(input_redi == 1 && output_redi == 1) {
        char* token;
        token = strtok(copy, " <>\t\n");
        while(token!=NULL) {
                cmd_tokens[tok++] = strdup(token);
                token = strtok(NULL, "<> \t\n");
        }
        if(idxi < idxo ) {
                infile = strdup(cmd_tokens[tok - 2]);
                outfile = strdup(cmd_tokens[tok - 1]);
        }
        else {
                infile = strdup(cmd_tokens[tok - 1]);
                outfile = strdup(cmd_tokens[tok - 2]);
        }
        cmd_tokens[tok - 2] = cmd_tokens[tok - 1] = NULL;
        
        return tok - 2;
    }
            
    if(input_redi == 1) {
        char* token;
        char* copy = strdup(cmd);

        char** input_redi_cmd = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN);
        token = strtok(copy, "<");
        while(token!=NULL) {
        input_redi_cmd[tok++] = token;
        token = strtok(NULL, "<");
        }
        copy = strdup(input_redi_cmd[tok - 1]);

        token = strtok(copy, "> |\t\n");
        infile = strdup(token);

        tok = 0;
        token = strtok(input_redi_cmd[0], CMD_DELIMS);
        while(token!=NULL) {
        cmd_tokens[tok++] = strdup(token);
        token = strtok(NULL, CMD_DELIMS);
        }

        cmd_tokens[tok] = NULL;

        free(input_redi_cmd);    
    }

    if(output_redi == 1) {
        char* copy = strdup(cmd);
        char* token;
        char** output_redi_cmd = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN);
        if(last == 1)
                token = strtok(copy, ">");
        else if(last == 2)
                token = strtok(copy, ">>");
        while(token!=NULL) {
                output_redi_cmd[tok++] = token;
                if(last == 1) token = strtok(NULL, ">");
                else if(last == 2) token = strtok(NULL, ">>");
        }

        copy = strdup(output_redi_cmd[tok - 1]);
        token = strtok(copy, "< |\t\n");
        outfile = strdup(token);

        tok = 0;
        token = strtok(output_redi_cmd[0], CMD_DELIMS);
        while(token!=NULL) {
                cmd_tokens[tok++] = token;
                token = strtok(NULL, CMD_DELIMS);
        }

        free(output_redi_cmd);    
    }
    if(input_redi == 0 && output_redi == 0 ) return parse_cmd(strdup(cmd), cmd_tokens);
    else return tok;
}

/*********************** Implentation of Built-in Commands ***********************/

int cd_cmd(char** cmd_tokens, char* cwd, char* base_dir) {
    if(cmd_tokens[1] == NULL || strcmp(cmd_tokens[1], "~\0") == 0 || strcmp(cmd_tokens[1], "~/\0") == 0) {
        chdir(getenv("HOME")); 
        strcpy(cwd, getenv("HOME"));
        mod_cwd_rel(cwd);
    }
    else if(chdir(cmd_tokens[1]) == 0) {
        getcwd(cwd, MAX_BUF_LEN);
        mod_cwd_rel(cwd);
        return 0;
    }
    else {
        perror("Error executing cd command");
    }
}

void echo(char** cmd_tokens, int tokens, char* cmd) {
    if(tokens > 1 && cmd_tokens[1][0] == '-') {
        run_cmd(cmd_tokens, cmd);
        return;
    }
    int i, len = 0, in_quote = 0, flag = 0;
    char buf[MAX_BUF_LEN] = "\0";
    for(i = 0; isspace(cmd[i]); i++);
    if(i == 0) i = 5;
    else i += 4;
    for(; cmd[i] != '\0' ; i++) {
        if(cmd[i] == '"') {
            in_quote = 1 - in_quote;
        }    
        else if(in_quote == 0 && (isspace(cmd[i])) && flag == 0) {
            flag = 1;
            if(len > 0) buf[len++] = ' ';
        }
        else if(in_quote == 1 || !isspace(cmd[i])) buf[len++] = cmd[i];
        if(!isspace(cmd[i]) && flag == 1) flag = 0;
    }
    if(in_quote == 1) {
        fprintf(stderr, "Missing quotes\n");
        return;
    }
    else printf("%s\n", buf);
}

void pwd(char** cmd_tokens, char* cmd) {
    char pwd_dir[MAX_BUF_LEN];
    getcwd(pwd_dir, MAX_BUF_LEN - 1); 
    if(cmd_tokens[1] == NULL) printf("%s\n", pwd_dir);
    else run_cmd(cmd_tokens, cmd);
}

void jobs() {
    int i;
    for(i = 0; i < num_jobs ; i++) {
        if(table[i].active == 1) {
            if(table[i].bg == 1){
                printf("[%d] %d %s %s &\n", i, table[i].pid, table[i].stat, table[i].name);
            }
            else{
                printf("[%d] %d %s %s\n", i, table[i].pid, table[i].stat, table[i].name);
            }    
        }
    }
}

void kjob(int tokens, char** cmd_tokens) {
    if(tokens != 2) {
        fprintf(stderr, "Invalid usage of kjob!\n");
        return;
    }
    int job_num = atoi(cmd_tokens[1]);
    if(table[job_num].active == 1) {
        if(kill(table[job_num].pid, SIGKILL) < 0)                 /* For sending signal mentioned to job mentioned */
            fprintf(stderr, "Signal not sent!\n");
    }
    else fprintf(stderr, "Job not found\n");               
}

/* TODO
 * void bg (int tokens, char** cmd_tokens)
 */

void fg(int tokens, char** cmd_tokens) {
    if(tokens != 2) {
        fprintf(stderr, "Invalid usage of fg");
        return;
    }       

    int i, job_num = atoi(cmd_tokens[1]), status;
    if(table[job_num].active == 0) {
        printf("No such job exists\n");
        return;
    }
    if(table[job_num].active == 1) {
        int pid = table[job_num].pid, pgid;
        pgid = getpgid(pid);                     /* get pgid of mentioned job */
        tcsetpgrp(shell, pgid);                  /* Give control of shell's terminal to this process */
        fgpid = pgid;                            /* Set this pgid as fg pgid */
        if(killpg(pgid, SIGCONT) < 0)            /* Send signal to thid pgid to continue if stopped */
            perror("Can't get in foreground!\n");
        table[job_num].bg = 0;                   /* Change from background process to foreground process */
        waitpid(pid, &status, WUNTRACED);        /* Wait for this process, return even if it has stopped without trace */
        if(!WIFSTOPPED(status)) {                /* returns true if the child process was stopped by delivery of a signal */
            table[job_num].active = 0;
            fgpid = 0;
        }
        else{
            strcpy(table[job_num].stat,"Stopped");
        }
        tcsetpgrp(shell, my_pid);                /* Give control of terminal back to the executable */
    }
    else fprintf(stderr, "No job found\n");
}

/*********************** Functions for running the commands ***********************/

void add_proc(int pid, char* name) {
    table[num_jobs].pid = pid;
    table[num_jobs].name = strdup(name);

    table[num_jobs].active = 1;
    strcpy(table[num_jobs].stat, "Running");
    num_jobs++;
}

void rem_proc(int pid) {
    int i;
    for(i = 0 ; i < num_jobs; i++) {
        if(table[i].pid == pid) {
            table[i].active = 0;
            break;
        }
    }
}

int run_cmd(char** cmd_tokens, char* cmd) {
    pid_t pid;
    pid = fork();
    if(pid < 0) {
        perror("Child Proc. not created\n");
        return -1; 
    }
    else if(pid==0) { 
        int fin, fout;
        setpgid(pid, pid);                               /* Assign pgid of process equal to its pid */
        
        if(input_redi) {
            fin = open_infile();
            if(fin == -1){
                _exit(-1);
            }
        }
        if(output_redi) {
            fout = open_outfile();
            if(fout == -1){
                _exit(-1);
            }
        }

        if(is_bg == 0) tcsetpgrp(shell, getpid());        /* Assign terminal to this process if it is not background */

        signal (SIGINT, SIG_DFL);                         /* Restore default signals in child process */
        signal (SIGQUIT, SIG_DFL);
        signal (SIGTSTP, SIG_DFL);
        signal (SIGTTIN, SIG_DFL);
        signal (SIGTTOU, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
        
        int ret;
        if((ret = execvp(cmd_tokens[0], cmd_tokens)) < 0) {
            perror("Error executing command!\n");
            _exit(-1);
        }
        _exit(0);
    }
    if(is_bg == 0) {
        tcsetpgrp(shell, pid);                              /* Make sure the parent also gives control to child */
        add_proc(pid, cmd_tokens[0]);
        table[num_jobs].bg = 0;
        int status;
        fgpid = pid;
        waitpid(pid, &status, WUNTRACED);               /* Wait for this process, return even if it has stopped without trace */
        
        if(!WIFSTOPPED(status)) rem_proc(pid);         /* returns true if the child process was stopped by delivery of a signal */
 
        else{
            strcpy(table[num_jobs-1].stat,"Stopped");
        }
        tcsetpgrp(shell, my_pgid);                     /* Give control of terminal back to the executable */
    }
    else {
        table[num_jobs].bg = 1;
        printf("\[%d] %d\n", num_jobs, pid);
        add_proc(pid, cmd_tokens[0]);
        return 0;
    }
}

void normal_cmd(int tokens, char** cmd_tokens, char* cmd_copy) {
    int check_bg = strlen(cmd_tokens[tokens-1]) - 1;
    if(tokens > 0){
        if(strcmp(cmd_tokens[0], "fg\0") == 0 ){
            fg(tokens, cmd_tokens);
        }
        else if(strcmp(cmd_tokens[0], "jobs\0") == 0){
            jobs();
        }
        else if(strcmp(cmd_tokens[0], "kill\0") == 0){
            kjob(tokens, cmd_tokens);
        }
        else if(strstr(cmd_tokens[tokens-1][check_bg], "&\0") == 0){
            cmd_tokens[tokens - 1] = NULL;
            is_bg = 1;
            run_cmd(cmd_tokens, cmd_copy);        // for running background process
        }
        else if(strcmp(cmd_tokens[0], "cd\0") == 0){
            cd_cmd(cmd_tokens, cwd, base_dir);
        }
        else if(strcmp(cmd_tokens[0], "pwd\0") == 0){
            pwd(cmd_tokens, cmd_copy);
        }
        else if(strcmp(cmd_tokens[0], "exit\0") == 0){
            _exit(0);
        }
        else if(strcmp(cmd_tokens[0], "echo\0") == 0){
            echo(cmd_tokens, tokens, cmd_copy);
        }
        else if(isalpha(cmd_tokens[0][0])){
            run_cmd(cmd_tokens, cmd_copy);
        }
    }
    free(cmd_tokens);
}

void redi_and_pipi_cmd(char* cmd) {
    int pid, pgid, fin, fout;

    num_pipe = 0;

    parse_for_piping(cmd);

    int* pipes = (int* )malloc(sizeof(int)*(2*(num_pipe - 1)));

    int i;

    for(i = 0; i < 2*num_pipe - 3; i += 2) {
        if(pipe(pipes + i) < 0 ) {             /* Create required number of pipes, each a combination of input and output fds */
            perror("Pipe not opened!\n");
            return;
        }
    }
    int status,j;
    for(i = 0; i < num_pipe ; i++) {
        char** cmd_tokens = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN); /* array of command tokens */
        int tokens = parse_for_redi(strdup(pipe_cmds[i]), cmd_tokens);
        is_bg = 0;               
        pid = fork();
        if(i < num_pipe - 1)
            add_proc(pid, cmd_tokens[0]);
        
        if(pid != 0 ) {
            if(i == 0 ) pgid = pid;
            setpgid(pid, pgid);                         /* Assign pgid of process equal to pgid of first pipe command pid */
        }
        if(pid < 0) {
            perror("Fork Error!\n");
        }
        else if(pid == 0) {
            signal (SIGINT, SIG_DFL);                              /* Restore default signals in child process */
            signal (SIGQUIT, SIG_DFL);
            signal (SIGTSTP, SIG_DFL);
            signal (SIGTTIN, SIG_DFL);
            signal (SIGTTOU, SIG_DFL);
            signal (SIGCHLD, SIG_DFL);

            if(output_redi) fout = open_outfile();
            else if(i < num_pipe - 1) dup2(pipes[2*i + 1], 1);

            if(input_redi) fin = open_infile();
            else if(i > 0 ) dup2(pipes[2*i -2], 0);
     
            int j;
            for(j = 0; j < 2*num_pipe - 2; j++) close(pipes[j]);

            if(execvp(cmd_tokens[0], cmd_tokens) < 0 ) {
                    perror("Execvp error!\n");
                    _exit(-1);
            }
        }
    }

    for(i = 0; i < 2*num_pipe - 2; i++) close(pipes[i]);

    if(is_bg == 0) {
        tcsetpgrp(shell, pgid);                  /* Assign terminal to pg of the pipe commands */

        for(i = 0; i < num_pipe ; i++) {
            int cpid = waitpid(-pgid, &status, WUNTRACED);
            
            /* Wait for this process, return even if it has stopped without trace */

            if(!WIFSTOPPED(status)) rem_proc(cpid);

            else{
                strcpy(table[num_jobs-1].stat,"Stopped");
            }
        }

        tcsetpgrp(shell, my_pgid);            /* Give control of terminal back to the executable */
    }
}

/*********************** Main function for running the Shell ***********************/

int main(){
    //Basic Setup
    initializer();

    //Command Loop
    while(1) {
        if(signal(SIGCHLD,signal_handler)==SIG_ERR)
            perror("can't catch SIGCHLD");
        if(signal(SIGINT,signal_handler)==SIG_ERR)
            perror("can't catch SIGINT!");

        set_prompt();

        int i,j;

        char** cmds = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN); // array of semi-colon separated commands

        for(j = 0; j < MAX_BUF_LEN; j++) cmds[j] = '\0';

        char* cmdline = read_cmdline(); // read command line
        int num_cmds = parse_cmd_line(cmdline, cmds); // parse command line

        for(i = 0; i < num_cmds; i++) {
            infile = outfile = NULL;
            is_bg = 0, num_pipe = 0;
            char* cmd_copy = strdup(cmds[i]);

            char** cmd_tokens = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN); // array of command tokens
            for(j = 0; j < MAX_BUF_LEN; j++) cmd_tokens[j] = '\0';

            if(check_for_pipe(strdup(cmds[i])) == -1) {
                    if(input_redi == 1 || output_redi == 1) normal_cmd(parse_for_redi(strdup(cmd_copy), cmd_tokens), cmd_tokens, cmd_copy);
                    else {
                            int tokens = parse_cmd(strdup(cmds[i]), cmd_tokens);
                            normal_cmd(tokens, cmd_tokens, cmd_copy);
                    }
            }
            else redi_and_pipi_cmd(cmds[i]);
        }
        if(cmds) free(cmds);
        if(cmdline) free(cmdline);
    }
    return 0;
}
