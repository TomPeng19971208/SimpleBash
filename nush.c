#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "svec.h"
#include "tokenize.h"
#include <fcntl.h>
#include <sys/stat.h>

int execute(char* cmd);

//command without operator
int normal_command(svec* input) {
    // printf("\n--%s\n", extract(input, 0, 2));
    int cpid = fork();
    int status;
    if (cpid<0) {perror("fork failed");}
    if (cpid==0) {//child process
        execvp(input->data[0], input->data);
        //    printf("command not found\n");
    }
    else {
        wait(&status);
        free_svec(input);
        //printf("status is: %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
     exit(0);
}
//redirect command
int out_command(svec* input)
{
    int last = input->size-1;
    int status;
    int cpid = fork();
    char* temp = extract(input, 0, last-1);
    svec* input2 = tokenize(temp);
    if (cpid==0) {
        int file = open(input->data[last], O_RDWR|O_CREAT, S_IRWXU);
        dup2(file, 1);
        close(file);
        execvp(input2->data[0], input2->data);
    }
    else {
        waitpid(cpid, &status,0);
        free(temp);
        free_svec(input2);
        return WEXITSTATUS(status);
    }
    
}

//redirect input command
int in_command(svec* input)
{
    int last = input->size-1;
    int status,cpid;
    char* temp = extract(input, 0, last-1);
    svec* input2 = tokenize(temp);
    //"<" redirect input
    cpid = fork();
    if (cpid==0) {
        int file = open(input->data[last], O_RDWR);
        dup2(file, 0);
        close(file);
       // execvp(input2->data[0], input2->data);
        execute(temp);
        exit(0);
    }
    else {
        waitpid(cpid,&status,0);
        free(temp);
        free_svec(input2);
        return(WEXITSTATUS(status));
        //exit(0);
    }
}

int pipe_command(svec *store) {
    int idx = checkop(store, "|");
    char *cmd1 = extract(store, 0, idx);
    char *rest = extract(store, idx+1, store->size);
    svec* p1 = tokenize(cmd1);
   // printf("%s\n", cmd1);
    p1->data[p1->size]=0;
    svec* p2 = tokenize(rest);
    // printf("%s\n", rest);
    p2->data[p2->size]=0;
    
    int status;
    int cpid;
    cpid = fork();
    //child
    if (cpid == 0) {
        int pipes[2];
        int status2;
        int cpid2;
        pipe(pipes);
        cpid2= fork();
        if (cpid2 ==0) {//child
            close(pipes[0]);
            dup2(pipes[1],1);
            exit(execute(cmd1));
        }
        else {
            waitpid(cpid2, &status2, 0);
            close(pipes[1]);
            dup2(pipes[0],0);
            execute(rest);
            exit(status2);
        }
    }
    //parent
    else {
        waitpid(cpid,&status, 0);
        free(cmd1);
        free(rest);
        free_svec(p1);
        free_svec(p2);
        return WEXITSTATUS(status);
    }
}


int
execute(char* cmd)
{
    svec* store = tokenize(cmd);
    int status;
    //svec_push_back(store, NULL);
    
    //check for semicolon;
    if (checkop(store, ";")>=0) {
        int div = checkop(store, ";");
        char* p1 = extract(store, 0, div);
        char* p2 = extract(store, div+1, store->size);
        for (int i=0;i<2;i++) {
            if (i==0) {
               int i= execute(p1);
                free(p1);
            }
            else {
                int i=execute(p2);
                free(p2);
            }
        }
        return 0;
        
    }    //check for pipe;
    if (checkop(store, "|")>=0) {
        status =  pipe_command(store);
        return status;
    }
    //check for output redirection
    if (checkop(store, ">")>=0) {
        status = out_command(store);
        return status;
    }
    if(checkop(store, "<")>=0) {
        status = in_command(store);
        return status;
    }
    //check for "&" background
    if (checkop(store, "&")>=0) {
        free_svec(store);
        system(cmd);
        return 0;
    }
    //check for &&;
    if (checkop(store, "&&")>=0) {
        int idx = checkop(store, "&&");
        char *p1 = extract(store, 0, idx);
        int cpid,status;
        char *rest = extract(store, idx+1, store->size);
        //printf("rest = %s\n", rest);
        //int result = execute(p1);
        int result = normal_command(tokenize(p1));
        //printf("result ==%d\n", result);
        if (result==0 && strcmp(rest, "exit")==0) {
            free(p1);
            free(rest);
            exit(0);
        }
        if (result==0) {
            //    printf("complete");
            svec* temp = tokenize(rest);
            temp->data[temp->size] = 0;
            cpid = fork();
            if (cpid==0) {
                execvp(temp->data[0],temp->data);
            }
            else {
                waitpid(cpid, &status, 0);
                free_svec(temp);
            }
        }
        free(p1);
        free(rest);
        return WEXITSTATUS(status);
    }
    //check for ||;
    if (checkop(store, "||")>=0) {
        int idx = checkop(store, "||");
        int cpid, status;
        char *p1 = extract(store, 0, idx);
        char *rest = extract(store, idx+1, store->size);
        int result = normal_command(tokenize(p1));
        svec* temp = tokenize(rest);
        temp->data[temp->size]=0;
        if (result && strcmp(rest, "exit")==0) {
            free(p1);
            free(rest);
            exit(0);
        }
        if (result) {
            int status;
            int cpid = fork();
            if (cpid==0) {
               execvp(temp->data[0], temp->data);
            }
            else {
                waitpid(cpid, &status,0);
                free_svec(temp);
            }
        }
        free(p1);
        free(rest);
        return WEXITSTATUS(status);
    }
    
   
    //no operator found
    else {
        if (checkop(store, "exit")>=0) {
        free_svec(store);
        exit(0);
    }
        //check for cd
        if (checkop(store, "cd")>=0) {
            int i = checkop(store, "cd");
            int cpid, status;
            chdir(store->data[i+1]);
            return 0;
        }
        normal_command(store);
    }
    return 0;
}

int
main(int argc, char* argv[])
{
    char cmd[256];
    //read from stdin
    if (argc == 1) {
        printf("nush$ ");
        fflush(stdout);
        while(fgets(cmd, 256, stdin)) {
            execute(cmd);
            printf("nush$ ");
            fflush(stdout);
        }
    }
    
    //read from a file
    if (argc ==2) {
        FILE* input = fopen(argv[1], "r");
        while(fgets(cmd, 256, input)) {
            execute(cmd);
        }
        fclose(input);
    }
    else {
        printf("number of arguments can only be 1 or 2\n");
        return -1;
    }
    return 0;
}













