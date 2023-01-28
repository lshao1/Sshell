#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>

#define MAX_LINE 512
#define MAX_ARG 32

// finds how many spaces is in a string
int SpaceFinder(char* input){
    int spaceCount = 0;
    for(int i = 0; input[i]!='\0';i++){
        if(input[i] == ' '){
            spaceCount++;
        }
    }
    return spaceCount;
}

// find the last occuring index of a character in a string
int CharFindLast(char* input, char find){
    int pos = -1;
    for(size_t i = 0; i < strlen(input);i++){
        if(input[i] == find){
            pos = i;
        }
    }
    return pos;
}
// find the first occuring index of a character in a string 
int CharFindTop(char* input, char find){
    int pos = -1;
    for(size_t i = 0; i < strlen(input);i++){
        if(input[i] == find){
            return i;
        }
    }
    return pos;
}

// this function counts how many individual character c is in string s
int count_char(const char* s, char c) {
    int count = 0;
    const char* p = s;
    while ((p = strchr(p, c)) != NULL) {
        count++;
        p++;
    }
    return count;
}

// check if ampersand is in the right place
int CheckAndPersand(char* input){
    int fail = 0;
    int andpersandPlace = -1;
    for(int i = 0; input[i]!='\0';i++){
        if(andpersandPlace != -1 && input[i] != ' ') {
            fail = 1;
            break;
        }
        if(input[i]=='&'){
            andpersandPlace = i;
        }
    }
    return fail;
}

void my_exit(char *command) {
    char complete_message[] = "Bye...\n";
    write(STDERR_FILENO, complete_message, sizeof(complete_message) - 1);

    printf("+ completed '%s'\n", command);
    exit(0);
}

void my_pwd() {
    char *directory = (char*)malloc(MAX_LINE);
    getcwd(directory, MAX_LINE);
    printf("%s\n", directory);
    fprintf(stderr, "+ completed \'pwd\' [0]\n");
    free(directory);
}


void my_cd(char* args) {
    
    if (chdir(args) == -1) {
        fprintf(stderr,"Error: cannot cd into directory\n");
        return;
    }
}

char* command_runner(char* command, char* arguments) {
    

    // remember to free heap
    char* ret = NULL;
    if (!strcmp(command, "cd")) {
        my_cd(arguments);
        return ret;
    }
    else {
        char cmd[MAX_ARG] = "";
        //combine command
        strcat(cmd, command);
        // create args for execv
        char* args[] = {cmd, arguments, NULL};
        int res =  execvp(cmd, args);
        // failed
        if(res == -1){
            fprintf(stderr,"Error: command not found\n");
            exit(1);
        }
    }
    
    
    return ret;
}

void my_redirection(char* command) {
    // check whether include space
    char* symbol_location = strstr(command, ">");
    char* space_location = strstr(command, " ");
    // command argument length
    int input_len = symbol_location - space_location - 1;

    char* file_name;
    if ((symbol_location + 1)[0] == ' ') {
        file_name = symbol_location + 2;
    } 
    else {
        file_name = symbol_location + 1;
    }
    char* test = file_name;


    int fd;

    fd = open(file_name, O_WRONLY | O_CREAT, 0644);
    dup2(fd, STDOUT_FILENO);
    close(fd);

    char* cmd = strtok(command, " ");

    // command argument = args
    char input[MAX_ARG];
    strncpy(input, space_location + 1, input_len);
    input[input_len] = '\0';
    // create args for execv
    char* ret = command_runner(cmd, input);

    printf("%s\n", ret);
    free(ret);
}


char* get_cmd(char* command) {
    char* cmd = (char*)malloc(MAX_ARG); 
    for (size_t i = 0; i < strlen(command); i++) {
        if (command[i] == ' ') {
            break;
        }
        cmd[i] = command[i];
    }
    return cmd;
}

char** Space_Remover(char* command, int* count) {

    char* word_sets;
    char** all_word = (char **)calloc(12, MAX_ARG);
    word_sets = strtok(command, " ");
    int wordCount = 0;
    while(word_sets != NULL) {
        all_word[wordCount] = word_sets;
        wordCount++;
        word_sets = strtok(NULL, " ");
    }
    *count = wordCount;
    return all_word;
}
// this function is used after all the space is removed
void SpaceHandler(char** res, char** input, int count, int currentcmd) {
    if(count == 1){
        res[currentcmd*2] = (char*)malloc(MAX_ARG);
        strcpy(res[currentcmd * 2], input[0]);
        res[currentcmd*2+1] = NULL;
        return;
    }
    if(count == 2){
        res[currentcmd*2] = (char*)malloc(sizeof(MAX_ARG));
        strcpy(res[currentcmd * 2], input[0]);
        res[currentcmd*2+1] = (char*)malloc(sizeof(MAX_ARG));
        strcpy(res[currentcmd*2+1], input[1]);
        return;
    }
    else{ // a lot of shit 
        res[currentcmd*2] = (char*)malloc(sizeof(MAX_ARG));
        strcpy(res[currentcmd * 2], input[0]);
        char finalArg[32] = "";
        strcat(finalArg, input[1]);
        
        for(int i = 2 ; i < count; i++){
            strcat(finalArg, " ");
            strcat(finalArg,input[i]);
        }
        res[currentcmd*2+1] = (char*)malloc(MAX_ARG);
        strcpy(res[currentcmd*2+1], finalArg);
    }
    return;
}
char** Command_Parser(char* userInput, int* command_num) {
    
    char* commandSet;
    char* allCommandSets[4]; // the max amount of commands is int64_t
    commandSet = strtok(userInput, "|");
    int commandCount = 0; // how many commands we have
    while(commandSet != NULL) {
        allCommandSets[commandCount] = commandSet;
        commandCount++;
        commandSet = strtok(NULL,"|");
    }

    // now we have split the entire user input based off of character |
    char** finalCommandSets = (char **)malloc(commandCount * MAX_ARG);
    //initial
    for (size_t i = 0; i < 12; i++) {
        finalCommandSets[i] = NULL;
    }

    char** individualSet;

    for(int i = 0; i < commandCount;i++){
        int command_plus_args_count = 0;
        individualSet = Space_Remover(allCommandSets[i], &command_plus_args_count);
        SpaceHandler(finalCommandSets,individualSet,command_plus_args_count, i);
        free(individualSet);
    }
    (*command_num) = commandCount;
    return finalCommandSets;
}
void ProcessRunner (int processCount, char* origCommand, char** finalCommandSets, int redirect, int trn, char* filename, char *resultString) {
    // used to represent the result of the processes
    int processResults[processCount]; 
    int allProcesses = processCount;
    pid_t pid;
    int fd[processCount][2];
    // creating multiple pipes
    for (int i = 0; i < processCount; i++) {
        if (pipe(fd[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    int fd2;
    if(redirect){
        if(trn){
            fd2 = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
        }
        else{
            fd2 = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
        }
        

    }
    for(int i = 0; i < processCount; i ++){

        pid = fork();
        if(pid == 0){

            // child
            if(i != 0){ // not the first process, so we need to read

                dup2(fd[i-1][0], STDIN_FILENO);

            }
            if (i != processCount - 1){ // not the last process, so we need to write

                dup2(fd[i][1], STDOUT_FILENO);

            }
            // at the last proces sand we don't need to redirect
            // so output needs to be put into a string 
            if(i == processCount - 1 && redirect == 0){

                // set stdout to a pipe, later will be read by the main parent process
                // also need to set stderr
                dup2(fd[i][1],STDOUT_FILENO);
                dup2(fd[i][1],STDERR_FILENO);

            }
            // at the last process and we need to redirect
            if(i ==  processCount - 1 && redirect == 1){
                dup2(fd2,STDOUT_FILENO);
                close(fd2);
            }
            for(int j = 0 ; j < processCount; j++){ // close all
                close(fd[i][0]);
                close(fd[i][1]);
            }

            // execute the command 
            char* res = command_runner(finalCommandSets[i * 2],finalCommandSets[i * 2 + 1]);
            exit(1);
        }
        else{ // parent just closes a couple pipes that have been opened
            if(i != 0){
                close(fd[i-1][0]);
            }
            if(i != processCount-1){
                close(fd[i][1]);
            }
        }
    }
    // try to read the result that was put into final fd [1]
    char bufferOutput[100];
    if(!redirect){
        int readRes = read(fd[processCount - 1][0], bufferOutput, sizeof(bufferOutput));
        bufferOutput[readRes] = '\0';   
    }


    for (int i = 0 ; i < processCount; i ++){
        int status;
        waitpid(-1,&status,0);
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);

            processResults[i] = exit_status;
        }
    }
    char lastChar;
    for(int i = 0 ; bufferOutput[i]!='\0';i++){
        lastChar = bufferOutput[i];
    }
    if(lastChar != '\n'){
        strcat(bufferOutput,"\n");
    }
    char completionOutput[100];
    // first add the complete which command part
    sprintf(completionOutput,"+ completed \'%s\' ", origCommand);

    char eachCommandStatusOutput[processCount][100];
    for(int i = 0 ; i < processCount; i++){
        sprintf(eachCommandStatusOutput[i],"[%d]",processResults[i]);

        strcat(completionOutput,eachCommandStatusOutput[i]);
    }

    strcat(bufferOutput,completionOutput);

    strcpy(resultString,bufferOutput);
    return;
}

int special_char(char* location) {
    char bad_symbol[] = "/*?<>|:\"\'";
    size_t i = 0;
    while(1) {
        if (location[i] == '\0') {
            break;
        }
        if (strchr(bad_symbol, location[i++]) != NULL) {
            return 1;
        }
    }
    return 0;
}

void prepare_red (char** final, char* input) {
    // make a copy
    char* user_input = (char*)malloc(MAX_LINE);
    strcpy(user_input, input);

    char* split_line = strtok(user_input, ">>");
    char* all_info[2];
    size_t chunk = 0;
    while (split_line != NULL) {
        all_info[chunk++] = split_line;
        split_line = strtok(NULL, ">>");
    }
    // now we have split the user_input based off >>
    // store these back to res
    // info before ">>"

    final[0] = (char*)malloc(sizeof(all_info[0]));
    strcpy(final[0], all_info[0]);
    int len = strlen(all_info[0]);
    if (all_info[0][len - 1] == ' ') {
        memmove(all_info[0] + len - 1, all_info[0], 1);
    }
    // info after ">>" (file name)
    if (all_info[1][0] == ' ') {
        memmove(all_info[1], all_info[1] + 1, strlen(all_info[0]));
    }
    if (final[1] != NULL) {
        final[1] = (char*)malloc(sizeof(all_info[1]));
        strcpy(final[1], all_info[1]);
    }
    

}

void background_process(char* res, char* command,char* origCommand, char** bef_n_aft, int redirect, int trn, char* file_name) {
    // need to handle all commands

    int command_count = 0;
    char** allCommands = Command_Parser(bef_n_aft[0], &command_count);
    char* command_cpy = command;

    int final_result;
    char* final_result_ptr;

    final_result = shmget(IPC_PRIVATE, 10, IPC_CREAT | 0666);
    final_result_ptr = shmat(final_result, NULL, 0);
    char test[] = "hello from parent\n";
    strcpy(final_result_ptr, test);
    printf("working?\n");
    printf(" I am in the function!!!!\n");
    pid_t pid = fork();
    if (pid == 0) {
        printf(" I am in the last child!!!!\n");
        char resultString[100];
        strcpy(resultString, "");
        ProcessRunner(command_count,origCommand,allCommands,redirect,trn,file_name, resultString);
        printf(" last child DONE!!!!\n");

    }
    else {
        wait(NULL);
        // dump final_result_ptr to the last process;
        res = (char*)malloc(MAX_ARG);
        strcpy(res, final_result_ptr);
        printf("first child final result: %s\n", res);
    }

}

int main(void) {
    char command[MAX_LINE];
    int back_status;
    pid_t pid;
    
    int shmid; 
    char* shmptr;
    shmid = shmget(IPC_PRIVATE, 10, IPC_CREAT | 0666);
    shmptr = shmat(shmid, NULL, 0);
    shmptr = NULL;
    while (1) {
        printf("sshell@ucd$ ");
        fgets(command, sizeof(command), stdin);
        // get the command 
        command[strlen(command) - 1] = '\0';
        char origCommand[512];
        // copy for later printing result purposes
        
        strcpy(origCommand, command);
        int redirect = 0;
        int trn = 1;
        int background_job = 0;
        
        if(shmptr != NULL) {
            printf("%s\n", shmptr);
            shmptr = NULL;
        }
        
        int length = sizeof(command) / sizeof(command[0]) - 1;
        char symbol_to_remove = '&';
        int i, j = 0;
        for (i = 0; i < length; i++) {
            if (command[i] != symbol_to_remove) {
                command[j++] = command[i];
            }
        }
        command[j] = '\0';
        
        char origCommand_to_split[512];
        strcpy(origCommand_to_split, command);
        int count = 0;
        char** splitted_command = Space_Remover(origCommand_to_split, &count);

        char** bef_n_aft = (char **)calloc(2, MAX_LINE);
        prepare_red(bef_n_aft, command);
        char* file_name = bef_n_aft[1];


        if (!strcmp(command, "exit")) {
            my_exit(command);
        }
        else if (strstr(command, "pwd") != NULL) {
            my_pwd();
        }
        else if (strstr(origCommand_to_split, "cd") != NULL) {
            my_cd(splitted_command[1]);
        }
        else{

            pid = fork();
            if(pid == 0) {
                int command_count = 0;
                char** allCommands = Command_Parser(bef_n_aft[0], &command_count);
                int firstOccuringRed = CharFindTop(origCommand,'>');
                int lastOccuringPipe = CharFindLast(origCommand,'|');
                // command count has to be the amount of '|' + 1
                int pipesAmount = count_char(origCommand,'|');
                if(command_count != pipesAmount + 1){
                    fprintf(stderr,"Error: missing command\n");
                    exit(0);
                }
                if(firstOccuringRed < lastOccuringPipe){
                    fprintf(stderr, "Error: mislocated output redirection\n");
                    exit(0);
                }
                if(CheckAndPersand(origCommand)){
                        fprintf(stderr,"Error: mislocated background sign\n");
                        exit(0);
                }
                for(int i = 1; i < command_count * 2; i+=2){
                        if(SpaceFinder(allCommands[i]) > 11) {
                            fprintf(stderr,"Error: too many process arguments\n");
                            exit(0);
                        }
                    }
                if (strstr(origCommand, "&") != NULL) {
                    background_job = 1;
                    printf("yes have back ground job!!!!\n");
                }
                if (strstr(command, ">") != NULL) { // redirection
                    redirect = 1;
                    if (!strcmp(splitted_command[0], ">")) {
                        fprintf(stderr, "Error: missing command\n");
                        exit(1);
                    } 
                    else if (!strcmp(splitted_command[count - 1], ">")) {
                        fprintf(stderr, "Error: no output file\n");
                        exit(1);
                    }
                    else if (special_char(splitted_command[count-1])) {
                        fprintf(stderr, "Error: cannot open output file\n");
                        exit(1);
                    }
                    if (strstr(command, ">>") != NULL) {
                        // append
                        trn = 0;
                    }
                }
                
                
                if (background_job) {

                    int back_shmid;
                    char* back_shmptr;
                    pid_t back_pid;
                    back_shmid = shmget(IPC_PRIVATE, 10, IPC_CREAT | 0666);

                    back_shmptr = shmat(back_shmid, NULL, 0);

                    command[strlen(command) - 1] = '\0';

                    back_shmptr = (char*)malloc(MAX_ARG);
                    back_pid = fork();
                    if (back_pid == 0) {
                        
                        background_process(back_shmptr, command, origCommand, bef_n_aft, redirect, trn, file_name);

                    }
                    else {
                        wait(NULL);

                        shmptr = (char*)malloc(MAX_ARG);
                        strcpy(shmptr, back_shmptr);
                        
                        free(back_shmptr);
                    }   
                } 
                else {
                    
                    char resultString[MAX_LINE];
                    strcpy(resultString, "");
                    ProcessRunner(command_count,origCommand,allCommands,redirect,trn,file_name, resultString);
                    printf("%s\n", resultString);
                }
        
            
                

                exit(0);
            }
            
        }
        if (background_job) {
            waitpid(pid, &back_status, WNOHANG);
        }
        else {
            wait(NULL);
        }
        free(bef_n_aft);
        free(splitted_command);
        
    }
    return 0;
}