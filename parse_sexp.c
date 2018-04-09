/*
parse_sexp.c
*/
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "command.h"

#include "parse_sexp.h"

static char temp_line[128];

char*
read_line(int fd)
{
    int ii;
    for (ii = 0; ii < 100; ++ii) {
        int rv = read(fd, temp_line + ii, 1);
        if (rv == 0) {
            return 0;
        }
        if (temp_line[ii] == '\n') {
            temp_line[ii] = 0;
            break;
        }
    }
    return temp_line;
}

char*
get_first(char* line) {
    int i = 0;
    int j = 0;
    while (line[i] != '\"') {
        ++i;
    }
    ++i;
    char* first = malloc(100*sizeof(char));
    while (line[i] != '\"') {
        first[j] = line[i];
        ++i;
        ++j;
    }
    first[j] = '\0';
    return first;
}

char*
get_rest(char* line, int begin, int len) {
    char* rest = malloc(100*sizeof(char));
    int i = 0;
    while (begin < len-1) {
        rest[i] = line[begin];
        ++begin;
        ++i;
    }
    rest[i] = '\0';
    return rest;
}

char*
get_first_shellcmd(char* input, int begin) {
    int left_parens = 0, right_parens = 0;
    while (input[begin] != '(') {
        ++begin;
    }
    char* shellcmd = malloc(100*sizeof(char));
    shellcmd[0] = '(';
    ++left_parens;
    ++begin;
    int j = 1;
    int quotes = 0;
    while (left_parens != right_parens) {
        if (input[begin] == '(') {
            ++left_parens;
        } else if ((input[begin] == ')') && (quotes % 2 == 0)) {
            ++right_parens;
        } else if (input[begin] == '\"') {
            ++quotes;
        }
        shellcmd[j] = input[begin];
        ++begin;
        ++j;
    }
    shellcmd[j] = '\0';
    return shellcmd;
}

command*
make_command_tree(char* line) {
    int len = strlen(line);
    char* first = get_first(line);
    char rest[100];
    if (line[strlen(first) + 3] == ')') {
        strcpy(rest,"");
    } else {
        char* get_rest_intermediate = get_rest(line, strlen(first) + 4, len);

        strcpy(rest,get_rest_intermediate);
        free(get_rest_intermediate);
    }

    command* cc;
    if (is_redir(first)) {
        char* operation = get_first_shellcmd(rest,0);
        command* commander = make_command_tree(operation);
        char* file = malloc(100*sizeof(char));
        int file_idx = 0;
        int n = strlen(operation) + 1;
        free(operation);
        while (n < strlen(rest)) {
            file[file_idx] = rest[n];
            ++n;
            ++file_idx;
        }
        file[file_idx] = '\0';
        cc = make_redir(first,commander,file);
    } else if (is_combin(first)) {
        char* first_arg = get_first_shellcmd(rest,0);
        if (streq(first,"&")) {
            command* left_command = make_command_tree(first_arg);
            cc = make_combin(first,left_command,NULL);
        } else {
            command* left_command = make_command_tree(first_arg);
            if (strlen(rest) == strlen(first_arg)+strlen(first)+5) {
                // if there is only one arg to this combin op
                cc = make_combin(first,left_command,NULL);
            } else {
                if (strlen(rest) < strlen(first_arg)+1) {
                    cc = make_combin(first,left_command,NULL);
                } else {
                    char* second_arg = get_first_shellcmd(rest,strlen(first_arg)+1);
                    command* right_command = make_command_tree(second_arg);
                    cc = make_combin(first,left_command,right_command);
                }
            }
        }

    } else {
        if (streq(rest,"")) {
            // this command has no args
            // empty args array
            char** empty_args = malloc(sizeof(char*));
            empty_args[0] = " ";
            cc = make_simple(first,empty_args);
        } else {
            char args_rest[100];
            char** cmd_args = malloc(100*sizeof(char*));
            int k = 0;
            while (!streq(rest,"")) {
                cmd_args[k] = get_first(rest);
                if (rest[strlen(cmd_args[k]) + 2] == ')') {
                    strcpy(rest,"");
                } else {
                    char* get_rest_intermediate_2 = get_rest(rest, strlen(cmd_args[k]) + 3, strlen(rest)+1);
                    strcpy(rest,get_rest_intermediate_2);
                    free(get_rest_intermediate_2);
                }
                k++;
            }
            cmd_args[k] = " ";
            cc = make_simple(first,cmd_args);
        }
    }
    return cc;
}
