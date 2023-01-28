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
    fprintf(stderr,"%s", complete_message);
    fprintf(stderr, "+ completed '%s' [0]\n", command);
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

    //printf("%s\n", ret);
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
    char** no_space = (char **)calloc(512/32, 32);
    size_t i, size, location, position = 0;
    for (size_t j = 0; j < strlen(command); j++) {
        char word[MAX_ARG];
        size = 0;
        for (i = j; i < strlen(command); i++) {
            //if (command[i] == ' ' || command[i] == '|' || command == '>') {
            if(command[i] == ' ') {
                i--;
                break;
            }
            word[i - j] = command[i];
            size++;
        }
        
        j = i + 1;
        //if(strlen(word))

        word[size] = '\0';
        if (!(word[0] == '\0')) {
            location = position++;
            no_space[location] = (char*)malloc(strlen(word) + 1);
            strcpy(no_space[location], word);
            (*count)++;
        }

        memset(word, '\0', sizeof(word));
        
    }
    
    return no_space;
}
// this function is used after all the space is removed
void SpaceHandler(char** res, char** input, int count, int currentcmd) {
    if(count == 1){
        res[currentcmd*2] = (char*)malloc(sizeof(input[0]));
        strcpy(res[currentcmd * 2], input[0]);
        //res[currentcmd*2+1] = (char*)malloc(0);
        res[currentcmd*2+1] = NULL;
        return;
    }
    if(count == 2){
        res[currentcmd*2] = (char*)malloc(sizeof(input[0]));
        strcpy(res[currentcmd * 2], input[0]);
        res[currentcmd*2+1] = (char*)malloc(sizeof(input[0]));
        strcpy(res[currentcmd*2+1], input[1]);
        return;
    }
    else{ // a lot of shit 
        res[currentcmd*2] = (char*)malloc(sizeof(input[0]));
        strcpy(res[currentcmd * 2], input[0]);
        char finalArg[32] = "";
        strcat(finalArg, input[1]);
        
        for(int i = 2 ; i < count; i++){
            strcat(finalArg, " ");
            strcat(finalArg,input[i]);
        }
        res[currentcmd*2+1] = (char*)malloc(sizeof(finalArg));
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
    for(int j = 0 ; j < commandCount; j++){
        //printf("%s\n", allCommandSets[j]);
    }
    // now we have split the entire user input based off of character |
    char** finalCommandSets = (char **)calloc(512/32, 32);
    char** individualSet;
    
    for(int i = 0; i < commandCount;i++){
        int command_plus_args_count = 0;
        individualSet = Space_Remover(allCommandSets[i], &command_plus_args_count);
        SpaceHandler(finalCommandSets,individualSet,command_plus_args_count, i);
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
    //printf("%d,%d,%d,%d,%d,%d\n", fd[0][0],fd[0][1],fd[1][0],fd[1][1],fd[2][0],fd[2][1]);
    // deal with the redirect file directory
    int fd2;
    if(redirect){
        if(trn){
            fd2 = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
        }
        else{
            fd2 = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
        }
        
        //fprintf(stderr,"redirecting file dir: %d\n",fd2);
    }
    for(int i = 0; i < processCount; i ++){
        //fprintf(stderr,"running process %d\n",i);
        pid = fork();
        if(pid == 0){
            //fprintf(stderr,"child accessed\n");
            // child
            if(i != 0){ // not the first process, so we need to read
                //fprintf(stderr,"reading\n");
                dup2(fd[i-1][0], STDIN_FILENO);
                //fprintf(stderr,"read\n");
            }
            if (i != processCount - 1){ // not the last process, so we need to write
                //fprintf(stderr,"writing\n");
                dup2(fd[i][1], STDOUT_FILENO);
                //fprintf(stderr,"written\n");
            }
            // at the last proces sand we don't need to redirect
            // so output needs to be put into a string 
            if(i == processCount - 1 && redirect == 0){
                //fprintf(stderr,"writing\n");
                // set stdout to a pipe, later will be read by the main parent process
                // also need to set stderr
                dup2(fd[i][1],STDOUT_FILENO);
                dup2(fd[i][1],STDERR_FILENO);
                //fprintf(stderr,"written\n");
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
            //fprintf(stderr,"executing command %s\n",finalCommandSets[i * 2]);
            //fprintf(stderr,"command args %s\n",finalCommandSets[i * 2+1]);
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
    //printf("Final output is : \n%s\n", bufferOutput);
    //fclose(stdout);
    for (int i = 0 ; i < processCount; i ++){
        int status;
        waitpid(-1,&status,0);
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            //fprintf(stderr,"process exited with status %d\n", exit_status);
            processResults[i] = exit_status;
        }
    }
    char completionOutput[100];
    // first add the complete which command part
    sprintf(completionOutput,"+ completed \'%s\' ", origCommand);
    //fprintf(stderr, "+ completed \'%s\' ", origCommand);
    char eachCommandStatusOutput[processCount][100];
    for(int i = 0 ; i < processCount; i++){
        sprintf(eachCommandStatusOutput[i],"[%d]",processResults[i]);
        //fprintf(stderr, "[%d]",processResults[i]);
        strcat(completionOutput,eachCommandStatusOutput[i]);
    }
    //fprintf(stderr,"\n");
    strcat(bufferOutput,completionOutput);
    //printf("the final string is: \n%s\n",bufferOutput);
    strcpy(resultString,bufferOutput);
    return;
}

int main(void) {
    char command[MAX_LINE];
    pid_t pid;
    while (1) {
        printf("sshell@ucd$ ");
        fgets(command, sizeof(command), stdin);
        // get the command 
        command[strlen(command) - 1] = '\0';
        char origCommand[512];
        // copy for later printing result purposes
        strcpy(origCommand, command);
        int count = 0;
        char** splitted_command = Space_Remover(command, &count);
        if (!strcmp(command, "exit")) {
            my_exit(command);
        }
        else if (strstr(command, "cd") != NULL) {
            command_runner(splitted_command[0], splitted_command[1]);
        }
        else{
            
            pid = fork();
            if(pid == 0) {
                if (strstr(command, ">") != NULL) { // redirection
                    my_redirection(command);
                }
                
                else 
                {
                    int command_count = 0;
                    char** allCommands = Command_Parser(command, &command_count);
                    char resultString[100];
                    strcpy(resultString, "");
                    ProcessRunner(command_count,origCommand,allCommands,0,0,"output.txt", resultString);
                    char *firstLine;
                    char *secondLine;

                    firstLine = strtok(resultString, "\n");
                    secondLine = strtok(NULL, "\n");

                    fprintf(stdout, "%s\n", firstLine);
                    fprintf(stderr, "%s\n", secondLine);
                } 
                free(splitted_command);
                exit(0);
            }
            
        }
        int status;
        wait(&status);
        
    }

    return 0;
}