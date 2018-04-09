/*
command.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"

command*
make_simple(char* comm, char** cmd_args) {
    simple* s = malloc(sizeof(simple));
    s->kind = KIND_SIMPLE;
    s->cmd = comm;
    s->args = cmd_args;
    return (command*) s;
}

command*
make_combin(char* oper, command* l, command* r) {
    combin* c = malloc(sizeof(combin));
    c->kind = KIND_COMBIN;
    c->op = oper;
    c->left = l;
    c->right = r;
    return (command*) c;
}

command*
make_redir(char* oper, command* f, char* fi) {
    redir* r = malloc(sizeof(redir));
    r->kind = KIND_REDIR;
    r->op = oper;
    r->func = f;
    r->file = fi;
    return (command*) r;
}

void
free_command(command* c) {
    if (*((char*)c) == KIND_SIMPLE) {
        simple* s = (simple*) c;
        free_array(s->args);
        free(s->cmd);
        free(s);
    } else if (*((char*)c) == KIND_COMBIN) {
        combin* cc = (combin*) c;
        free(cc->op);
        free_command(cc->left);
        if (cc->right) {
            free_command(cc->right);
        }
        free(cc);
    } else {
        redir* re = (redir*) c;
        free(re->op);
        free_command(re->func);
        free(re->file);
        free(re);
    }
}

void
free_array(char** arr) {
    int i = 0;
    while (!streq(arr[i]," ")) {
        free(arr[i]);
        ++i;
    }
    free(arr);
}

int
streq(const char* one, const char* two) {
    return strcmp(one, two) == 0;
}

int is_redir(char* c) {
    return streq(c,"<") || streq(c,">");
}

int is_combin(char* c) {
    return streq(c,";")
        || streq(c,",")
        || streq(c,"&")
        || streq(c,"||")
        || streq(c,"&&")
        || streq(c,"|");
}
