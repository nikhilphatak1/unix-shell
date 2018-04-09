/*
parse_sexp.h
utility functions for building
the command tree from input sexp
*/
#ifndef PARSE_SEXP_H
#define PARSE_SEXP_H

#include "command.h"

char* read_line(int fd);

char* get_first(char* line);

char* get_rest(char* line, int begin, int len);

char* get_first_shellcmd(char* input, int begin);

command* make_command_tree(char* line);

#endif
