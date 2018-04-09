/*
command.h
*/
#ifndef COMMAND_H
#define COMMAND_H

#define KIND_SIMPLE 0
#define KIND_COMBIN 1
#define KIND_REDIR 2

typedef union command {
    struct combin* comb;
    struct simple* simp;
    struct redir* redr;
} command;

typedef struct simple {
    int kind;
    char* cmd;
    char** args;
} simple;

typedef struct combin {
    int kind;
    char* op;
    command* left;
    command* right;
} combin;

typedef struct redir {
    int kind;
    char* op;
    command* func;
    char* file;
} redir;

command* make_simple(char* comm, char** cmd_args);
command* make_combin(char* oper, command* l, command* r);
command* make_redir(char* oper, command* f, char* fi);

void free_command(command* c);

int streq(const char* one, const char* two);

int is_redir(char* c);
int is_combin(char* c);

void free_array(char** arr);

#endif
