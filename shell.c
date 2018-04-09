/*
shell.c
*/
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "command.h"
#include "parse_sexp.h"

// attribution:
// - fork() syntax from lecture notes sort-pipe.c
// - pipe() syntax from lecture notes sort-pipe.c
// - syscall_check() based on check_rv() from lecture notes,
//   changed perror to fprintf
// - in exec_redir(), 0666 tag in open() call from man page
//   using O_CREAT

int exec_simple(simple* si);
int exec_combin(combin* co);
int exec_redir(redir* re);
int execute(command* cc);

int syscall_check(int sys_return) {
    if (sys_return == -1) {
        fprintf(stderr,"syscall failed\n");
        exit(1);
    }
}


int
exec_simple(simple* si) {
    int cpid;
    int kind = si->kind;
    assert(kind == KIND_SIMPLE);
    char* cmd = si->cmd;
    char** args = si->args;
    char* arguments[20];
    arguments[0] = cmd;
    int i = 1;
    while (!streq(args[i-1]," ")) {
        arguments[i] = args[i-1];
        ++i;
    }
    arguments[i] = 0; // terminate arguments array with 0

    if (streq(cmd,"cd")) {
        if (arguments[1]) {
            chdir(arguments[1]);
        }
        return 0;

    } else if (streq(cmd,"exit")) {

        exit(0);

    } else {

        if ((cpid = fork())) {
            // parent
            int status;
            waitpid(cpid, &status, 0);
            return status;

        } else {
            // child
            execvp(cmd, arguments);
            assert(0);
        }
    }
}

int
exec_combin(combin* co) {

    assert(co->kind == KIND_COMBIN);
    char* op = co->op;
    command* left = co->left;
    command* right = co->right;

    if (streq(op,"|")) {
        int cpid1,cpid2,cpid3, sys_return;
        int rw_pipe[2];

        if ((cpid1 = fork())) {         // parent 1
            int status;
            waitpid(cpid1, &status, 0);
            return status;
        } else {                        // child 1
            sys_return = pipe(rw_pipe);
            syscall_check(sys_return);

            if ((cpid2 = fork())) {     // parent 2
                int status2;
                close(rw_pipe[1]);

                if ((cpid3 = fork())) { // parent 3
                    int status3;
                    waitpid(cpid3, &status3, 0);
                } else {                // child 3
                    close(0);
                    dup(rw_pipe[0]);
                    execute(right);
                    exit(0);
                }
                waitpid(cpid2,&status2,0);
            } else {                    // child 2
                close(rw_pipe[0]);
                close(1);
                dup(rw_pipe[1]);
                execute(left);
                exit(0);
            }
            exit(0);
        }

    } else if (streq(op,"&")) {
        int cpid;
        if ((cpid = fork())) {
            int status;
            return 0;
        } else {
            execute(left);
        }

    } else if (streq(op,"&&")) {
        int sys_return = execute(left);
        if (sys_return == 0) {
            sys_return = execute(right);
        }
        return sys_return;

    } else if (streq(op,"||")) {
        int sys_return = execute(left);
        if (sys_return != 0) {
            execute(right);
        }

    } else if (streq(op,";")) {

        if (!right) {

            execute(left);

        } else {

            execute(left);
            execute(right);

        }

    }

}


int
exec_redir(redir* re) {

    int cpid,sys_return,return_ex;

    int kind = re->kind;
    assert(kind == KIND_REDIR);
    char* op = re->op;
    command* f = re->func;
    char* file = re->file;

    char newfile[50];

    int i = 1;
    while (i < strlen(file) - 1) {
        newfile[i-1] = file[i];
        ++i;
    }
    newfile[i-1] = '\0';

    if (streq(op,"<")) {

        if ((cpid = fork())) {

            int status;
            waitpid(cpid, &status, 0);

        } else {

            int fd_file = open(newfile,O_RDONLY);
            close(0);
            sys_return = dup(fd_file);
            syscall_check(sys_return);

            return_ex = execute(f);

            sys_return = close(fd_file);
            syscall_check(sys_return);

            return return_ex;
        }

    } else {

        if ((cpid = fork())) {

            int status;
            waitpid(cpid, &status, 0);

        } else {

            int fd_file = open(newfile,O_WRONLY|O_TRUNC|O_CREAT,0666);
            syscall_check(fd_file);

            int save_fd = dup(1);
            syscall_check(save_fd);

            sys_return = close(1);
            syscall_check(sys_return);


            sys_return = dup(fd_file);
            syscall_check(sys_return);

            sys_return = close(fd_file);
            syscall_check(sys_return);

            return_ex = execute(f);

            sys_return = close(1);
            syscall_check(sys_return);

            sys_return = dup(save_fd);
            syscall_check(sys_return);

            return return_ex;

        }
    }
}

int
execute(command* cc) {

    if (*((char*)cc) == KIND_SIMPLE) {
        simple* si = (simple*) cc;
        return exec_simple(si);
    } else if (*((char*)cc) == KIND_COMBIN) {
        combin* co = (combin*) cc;
        return exec_combin(co);
    } else {
        redir* re = (redir*) cc;
        return exec_redir(re);
    }

}

int
main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: ./shell input.sexp\n");
        return 1;
    }

    int input = open(argv[1], O_RDONLY);
    if (input == -1) {
        perror("open failed");
        return 1;
    }

    char* line;
    int sys_return = 0;

    while ((line = read_line(input))) {
        command* cc = make_command_tree(line);
        execute(cc);
        free_command(cc);
    }

    close(input);

    return sys_return;
}
