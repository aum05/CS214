#include "utils.h"

/*********************** Functions for initailizing the shell ***********************/

// Function to the get the present working directory where the shell program is run
void get_base_dir() {
    getcwd(opening_dir,  MAX_SIZE - 1); 
    strcpy(cwd, opening_dir);
}

// Function that modifies the PWD relative to assumed home directory
void change_cwd(char* cwd) {
    int i, j;

    for(i = 0; cwd[i]==opening_dir[i] && cwd[i]!='\0' && opening_dir[i] != '\0'; i++);

    if(opening_dir[i] == '\0'){
        cwd[0] = '~';
        for(j = 1; cwd[i]!='\0'; j++) {
            cwd[j] = cwd[i++];
        }
        cwd[j] = '\0';
    }
}
// Function that handles signals
void sig_handler(int sig) {
    if(sig == SIGINT) {
        signal(SIGINT,SIG_IGN);               // Ignores ^C
        signal(SIGINT, sig_handler);       // Resets the signal handler
    }
    else if(sig == SIGCHLD) {                  // Handling SIGCHILD
        int i, stat, term_pid;
        while((term_pid = waitpid(-1, &stat, WNOHANG)) > 0) {
            for(i = 0; i < job_counter; i++){
                if(job_list[i].active==0) 
                    continue;
                else if(job_list[i].pid == term_pid)
                    break;
            }
            if(i != job_counter) {
                if(WIFSIGNALED(stat)){ 
                    printf("\n[%d] %d terminated with signal %d\n", i, job_list[i].pid, WTERMSIG(stat));
                }
                job_list[i].active = 0;
            }
        }
    }
}

// Initialize the shell
void init() {
    shell = STDERR_FILENO;

    job_counter = 0;

    in_commands = malloc((sizeof(char)*MAX_SIZE)*MAX_SIZE);
    out_commands = malloc((sizeof(char)*MAX_SIZE)*MAX_SIZE);

    if(isatty(shell)) {                                             // Testing the stderr to check it refers to terminal
        while(tcgetpgrp(shell) != (pgid_shell = getpgrp())){      
            kill(pgid_shell, SIGTTIN);                              // SIGTTIN allows for terminal input while working with background processes
        }                                     
    }
    signal (SIGTTIN, SIG_IGN);                                   // Ignore background processes

    signal (SIGINT, SIG_IGN);                                    // Ignore ^C

    signal (SIGTSTP, SIG_IGN);                                   // Ignore ^Z

    signal (SIGQUIT, SIG_IGN);                                   // Ignore ^/

    signal (SIGTTOU, SIG_IGN);

    main_pid = main_pgid = getpid();
    setpgid(main_pid, main_pgid);
    tcsetpgrp(shell, main_pgid);                                   // Give the executable's process group the control of stderr
    
    get_base_dir();
    change_cwd(cwd);                                 // Update the cwd relative to the opening directory
}

/*********************** Parser functions ***********************/
// The first two functions handle input output redirections

// Function to open input file
int open_input() {
    int fp = open(input_file, O_RDONLY, S_IRWXU);
    if (fp < 0) {
        perror(input_file);  
    }
    dup2(fp, STDIN_FILENO);
    close(fp);
    return fp;
}

// Function to open output file
int open_output() {
    int fp = 0;
    if(last == 1){
        fp = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
    }
    else if(last == 2){
        fp = open(output_file, O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
    } 
    if(fp < 0) {
        perror(output_file);
    }
    dup2(fp, STDOUT_FILENO);
    close(fp);
    return fp;
}

char* read_input() {
    char *str = NULL;
    int c;
    size_t size = 0;

    while(EOF != (c=fgetc(stdin)) && c != '\n' ) {
        //Increase the space by 2 bytes, since one character represents 1 byte.
        str = (char*)realloc(str,  size + 2);
        str[size++] = c;
    }
    if(str)
        str[size] = '\0';
    return str;
}

int parse_input(char* cmd_input, char** commands) {
    int counter = 0;
    char* cmd = strtok(cmd_input, ";");
    while(cmd!=NULL) {
        commands[counter++] = cmd;
        cmd = strtok(NULL, ";");
    }
    return counter;
}

int parse_commands(char* command, char** cmd_vars) {
    int num_cmds = 0;
    char* cmd = strtok(command, CMD_DELIMS);
    while(cmd!=NULL) {
        cmd_vars[num_cmds++] = cmd;
        cmd = strtok(NULL, CMD_DELIMS);
    }
    return num_cmds;
}

/*********************** Implentation of Built-in Commands ***********************/

int cd_func(char** cmd_vars, char* cwd) {
    if(cmd_vars[1] == NULL || strcmp(cmd_vars[1], "~\0") == 0 || strcmp(cmd_vars[1], "~/\0") == 0) {
        chdir(getenv("HOME")); 
        strcpy(cwd, getenv("HOME"));
        change_cwd(cwd);
    }
    else if(chdir(cmd_vars[1]) == 0) {
        getcwd(cwd, MAX_SIZE);
        change_cwd(cwd);
        return 0;
    }
    else {
        perror("Error changing directory");
    }
}

void pwd(char** cmd_vars, char* command) {
    char curr_dir[MAX_SIZE];
    getcwd(curr_dir, MAX_SIZE - 1); 
    if(cmd_vars[1] == NULL) printf("%s\n", curr_dir);
    else run(cmd_vars, command);
}

void jobs() {
    int i;
    for(i = 0; i < job_counter ; i++) {
        if(job_list[i].active == 1) {
            if(job_list[i].bg == 1){
                strtok(job_list[i].command,"&");
                printf("[%d] %d %s %s &\n", i, job_list[i].pid, job_list[i].stat, job_list[i].command);
            }
            else{
                printf("[%d] %d %s %s\n", i, job_list[i].pid, job_list[i].stat, job_list[i].command);
            }    
        }
    }
}

void kill_func(int num_cmnds, char** cmd_vars) {
    if(num_cmnds != 2 || !strstr(cmd_vars[1],"%")) {
        fprintf(stderr, "Invalid usage of kill\n");
        return;
    }

    int job_num = atoi(&cmd_vars[1][1]);
    if(job_list[job_num].active == 1) {
        if(kill(job_list[job_num].pid, SIGTERM) < 0)
            fprintf(stderr, "Signal not sent!\n");
    }
    else fprintf(stderr, "Job not found\n");               
}

void bg (int num_cmnds, char** cmd_vars){
    if(num_cmnds != 2 || !strstr(cmd_vars[1],"%")) {
        fprintf(stderr, "Invalid usage of bg\n");
        return;
    }

    int job_num = atoi(&cmd_vars[1][1]);
    if(job_list[job_num].active == 0) {
        printf("Job does not exists\n");
        return;
    }

    if(job_list[job_num].active == 1 && strcmp(job_list[job_num].stat, "Stopped") == 0) {
        int pid = job_list[job_num].pid, pgid;
        // Obtain the pgid of the suspended jop
        pgid = getpgid(pid);  
        // Send SIGCONT signal to this pgid to resume the process
        if(killpg(pgid, SIGCONT) < 0)
            perror("Can't run in background!\n");
        // Set the background process variable
        job_list[job_num].bg = 1;
        strcpy(job_list[job_num].stat,"Running");
        // Transfer control of terminal back to the shell
        tcsetpgrp(shell, main_pid);
    }
    else fprintf(stderr, "No job found\n");
}

void fg(int num_cmnds, char** cmd_vars) {
    if(num_cmnds != 2 || !strstr(cmd_vars[1],"%")) {
        fprintf(stderr, "Invalid usage of fg\n");
        return;
    }

    int job_num = atoi(&cmd_vars[1][1]), stat;
    if(job_list[job_num].active == 0) {
        printf("No such job exists\n");
        return;
    }
    if(job_list[job_num].active == 1) {
        int pid = job_list[job_num].pid, pgid;
        // Obtain the pgid of the background/suspended jop
        pgid = getpgid(pid);
        // Transfer shell's control to the job
        tcsetpgrp(shell, pgid);
        pid_fg = pgid;

        // Send SIGCONT signal to this pgid to resume the process
        if(killpg(pgid, SIGCONT) < 0)
            perror("Can't get in foreground!\n");
        
        // Change from background process to foreground process
        job_list[job_num].bg = 0;
        waitpid(pid, &stat, WUNTRACED);

        // True when the child process was stopped because of a signal
        if(!WIFSTOPPED(stat)){
            job_list[job_num].active = 0;
            pid_fg = 0;
        }
        else{
            printf("\n");
            strcpy(job_list[job_num].stat,"Stopped");
        }
        // Transfer control of terminal back to the shell
        tcsetpgrp(shell, main_pid);
    }
    else fprintf(stderr, "No job found\n");
}

/*********************** Functions for running the commands ***********************/

void start_proc(int pid, char* name, char* command) {
    job_list[job_counter].pid = pid;
    job_list[job_counter].name = strdup(name);
    job_list[job_counter].command = strdup(command);
    job_list[job_counter].active = 1;
    strcpy(job_list[job_counter].stat, "Running");
    job_counter++;
}

void delete_proc(int pid) {
    int i;
    for(i = 0 ; i < job_counter; i++) {
        if(job_list[i].pid == pid) {
            job_list[i].active = 0;
            break;
        }
    }
}

int run(char** cmd_vars, char* command) {
    pid_t pid;
    pid = fork();
    if(pid < 0) {
        perror("Failure creating a Child Process\n");
        return -1; 
    }
    else if(pid==0) { 
        int file_in, file_out;
        setpgid(pid, pid);
        
        if(redirect_input) {
            file_in = open_input();
            if(file_in == -1){
                _exit(-1);
            }
        }
        if(redirect_output) {
            file_out = open_output();
            if(file_out == -1){
                _exit(-1);
            }
        }

        // Transfer control of terminal to this process if it is in the foreground
        if(if_bg == 0){
            tcsetpgrp(shell, getpid());
        }

        // Restore default signals in child process
        signal (SIGINT, SIG_DFL);                      
        signal (SIGTSTP, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
        signal (SIGTTIN, SIG_DFL);
        signal (SIGTTOU, SIG_DFL);
        signal (SIGQUIT, SIG_DFL); 
        
        int val;
        if((val = execvp(cmd_vars[0], cmd_vars)) < 0) {
            perror(cmd_vars[0]);
            _exit(-1);
        }
        _exit(0);
    }
    if(if_bg == 0) {
        // Transfer control from parent to child
        tcsetpgrp(shell, pid);
        start_proc(pid, cmd_vars[0], command);
        job_list[job_counter].bg = 0;
        int stat;
        pid_fg = pid;
        waitpid(pid, &stat, WUNTRACED);
        
        // Check if the child was stopped by a signal
        if(!WIFSTOPPED(stat)){
            delete_proc(pid);
        }
        else{
            printf("\n");
            strcpy(job_list[job_counter-1].stat,"Stopped");
        }
        // Transfer control of terminal back to the shell
        tcsetpgrp(shell, main_pgid);
    }
    else {
        job_list[job_counter].bg = 1;
        printf("[%d] %d\n", job_counter, pid);
        start_proc(pid, cmd_vars[0], command);
        return 0;
    }
}

void cmd_executer(int num_cmnds, char** cmd_vars, char* copy_command) {
    char* ampersand = &cmd_vars[num_cmnds-1][strlen(cmd_vars[num_cmnds-1])-1];
    if(num_cmnds > 0){
        if(strcmp(cmd_vars[0], "fg\0") == 0 ){
            fg(num_cmnds, cmd_vars);
        }
        else if(strcmp(cmd_vars[0], "bg\0") == 0 ){
            bg(num_cmnds, cmd_vars);
        }
        else if(strcmp(cmd_vars[0], "cd\0") == 0){
            cd_func(cmd_vars, cwd);
        }
        else if(strcmp(cmd_vars[0], "jobs\0") == 0){
            jobs();
        }
        else if(strcmp(cmd_vars[0], "kill\0") == 0){
            kill_func(num_cmnds, cmd_vars);
        }
        else if(strcmp(ampersand, "&") == 0){
            if(strcmp(cmd_vars[num_cmnds-1], "&\0") == 0){
                cmd_vars[num_cmnds-1] = NULL;
            }
            else{
                strtok(cmd_vars[num_cmnds-1], "&");
            }
            if_bg = 1;
            run(cmd_vars, copy_command);
        }
        else if(strcmp(cmd_vars[0], "pwd\0") == 0){
            pwd(cmd_vars, copy_command);
        }
        else if(strcmp(cmd_vars[0], "exit\0") == 0){
            _exit(0);
        }
        else if(isalpha(cmd_vars[0][0]) || strstr(cmd_vars[0], "./") || strcmp(&cmd_vars[0][0], "/")){
            run(cmd_vars, copy_command);
        }
    }
    free(cmd_vars);
}

/*********************** Main function for running the Shell ***********************/

int main(int argc, char** argv){
    //Basic Setup
    init();

    //Command Loop
    while(!feof(stdin)) {
        if(signal(SIGCHLD,sig_handler)==SIG_ERR)
            perror("Error catching SIGCHLD");
        if(signal(SIGINT,sig_handler)==SIG_ERR)
            perror("Error catching SIGINT");

        printf("> ");

        int i,j;

        //Commands seperated by semi-colon
        char** commands = malloc((sizeof(char)*MAX_SIZE)*MAX_SIZE);

        for(j = 0; j < MAX_SIZE; j++) commands[j] = '\0';

        // Stores the command line input
        char* cmd_input = read_input();
        // Stores number of commands
        int num_cmds = parse_input(cmd_input, commands);

        for(i = 0; i < num_cmds; i++) {
            input_file = output_file = NULL;
            if_bg = 0;
            char* copy_command = strdup(commands[i]);

            // Array containing command variables
            char** cmd_vars = malloc((sizeof(char)*MAX_SIZE)*MAX_SIZE);
            for(j = 0; j < MAX_SIZE; j++) cmd_vars[j] = '\0';
            
            int num_cmnds = parse_commands(strdup(commands[i]), cmd_vars);
            cmd_executer(num_cmnds, cmd_vars, copy_command);
        }
        if(commands){
            free(commands);
        }
        if(cmd_input){
            free(cmd_input);
        }
    }
    return 0;
}