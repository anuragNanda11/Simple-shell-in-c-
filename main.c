//
//  main.c
//  csc 415 HW 3
//
//  Created by Anurag Nanda on 3/4/16.


#include <stdio.h>
#include "errno.h"
#include <string.h>
#include    <err.h>
#include  <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>

static void handle_redirection(char **argv, int op_index) {
    FILE *fp = NULL;
    if(strcmp(argv[op_index], ">") == 0) {
        fp = fopen(argv[op_index+1], "w");
        dup2(fileno(fp), STDOUT_FILENO);
    } else if (strcmp(argv[op_index], "<") == 0) {
        fp = fopen(argv[op_index+1], "r");
        dup2(fileno(fp), STDIN_FILENO);
    } else if (strcmp(argv[op_index], ">>") == 0) {
        fp = fopen(argv[op_index+1], "a");
        dup2(fileno(fp), STDOUT_FILENO);
    } else if (strcmp(argv[op_index], "2>") == 0) {
        fp = fopen(argv[op_index+1], "w");
        dup2(fileno(fp), STDERR_FILENO);
    } else if (strcmp(argv[op_index], "2>>") == 0) {
        fp = fopen(argv[op_index+1], "a");
        dup2(fileno(fp), STDERR_FILENO);
    }

    
    argv[op_index] = NULL;
    
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    fclose(fp);
    execvp(*argv,argv);
}


static void execute(char **argv, int tok_count){
    
    bool is_pipe = false;
    char *arg1[tok_count];
    int index = 0;
    
    for (; index < tok_count-1; index++) {
        
        if(strcmp(argv[index], "|") == 0) {
            arg1[index] = NULL;
            is_pipe = true;
            break;
        } else if (strcmp(argv[index], ">")==0
                   ||strcmp(argv[index], "<") == 0
                   ||(strcmp(argv[index], ">>") == 0)
                   ||(strcmp(argv[index], "2>") == 0)
                   ||(strcmp(argv[index], "2>>") == 0)) {
            handle_redirection(argv, index);
            return;
        }
        arg1[index] = argv[index];
    }
    
    
        int pipefd[2];
        pipe(pipefd);
        pid_t childpid = fork();
    

    
        if(childpid == 0) {
            if (dup2(pipefd[1], 1) < 0) {
                fprintf(stderr, "Error in duping: %s\n", strerror(errno));
                exit(1);
            }
            close(pipefd[0]);
            close(pipefd[1]);
            execvp(*arg1, arg1);
            exit (127);
        }
    
        if (dup2(pipefd[0],0) < 0) {
            fprintf(stderr, "Error in duping: %s\n", strerror(errno));
            exit(1);
        }
        close(pipefd[1]);
        close(pipefd[0]);
    
    
    if (is_pipe) {
        execvp(argv[index+1], &argv[index+1]);
        printf("failed to exec command 2");
    } else {
        execvp(*argv,argv);
        printf("failed to exec command");
    }
    
    fprintf(stderr, "Can't execute Command %s: %s\n",argv[0], strerror(errno));
    exit(127);

}


//returns true if the user requested the process to be run in background.
static bool bgrnd_ps(char *str) {
    if (strcmp(str, "&") == 0) return true;
    return false;
}



static int parse(char *input, char **argv, int max_args) {
    char *token;
    int tokenCount = 0;
    
    token = strtok(input, " \t\n");
    
    while (token != NULL) {
        if(tokenCount >= max_args) {
            printf("Tooooo Many Args\n");
            return -1;
        }
        argv[tokenCount]=token;
        token = strtok(NULL, " \t\n");
        tokenCount++;
    }
    argv[tokenCount] = NULL;
    return tokenCount+1;
}

int main(void) {
    char input[1024];
    int max_args = 10;
    char *argv[max_args];
    
    while (1) {
        printf("good_shell==> ");
        if (fgets(input,sizeof(input),stdin) == NULL) {
            if(feof(stdin))
                return(0);
            fprintf(stderr, "Commands Can't Be Read: %s\n", strerror(errno));
            return(1);
        }
        int tok_count = parse(input, argv, max_args);
        if (argv[0] == NULL) {
            printf("");
            continue;
        }
        if(strcmp(argv[0],"exit")==0){exit(0);}
        
        bool bgrnd  = bgrnd_ps(argv[tok_count-2]);
        
        if (bgrnd) {
            argv[tok_count-2] = NULL;
        }
        
        
        
        pid_t pid;
        pid = fork();
        
        if (pid == -1) {
            fprintf(stderr, "Failed to fork: %s\n",strerror(errno));
            continue;
        } else if (pid == 0) {
            if (tok_count > 1) {execute(argv, tok_count);}
        }
        
        if (!bgrnd) wait(NULL);
    }
}