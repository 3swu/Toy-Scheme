#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "src/header/read.h"
#include "src/header/builtin.h"
#include "src/header/eval.h"
#include "src/header/environment.h"
#include "src/header/write.h"
#include "src/header/error.h"

void print_prompt() {
    printf("Welcome to Toy-Scheme\nPress Ctrl-C to exit\n");
}

void repl() {
    object* obj, * result;
    for(; ;) {
        printf("> ");
        obj = reader(stdin);
        if(obj == NULL){
            break;
        }
        result = eval(obj, the_global_environment);
        if(result == NULL) {
            error_handle(stderr, "eval return a null object\n", EXIT_FAILURE);
        }
        else {
            write(stdout, result);
            printf("\n");
        }
    }
}

int main(int argc, char** argv) {

    if(argc == 1) {
        init_built_in();
        print_prompt();
        repl();
    }
    else if(argc > 1) {
        if(strcmp(argv[1], "-f") != 0) {
            char buf[50];
            sprintf(buf, "unknown argument %s\n", argv[1]);
            error_handle(stderr, buf, EXIT_FAILURE);
        }
        if(strcmp(argv[1], "-f") == 0 && argc == 2) {
            error_handle(stderr, "no file name\n", EXIT_FAILURE);
        }
        else {
            int filename_len = (int)strlen(argv[2]);
            if(!(argv[2][filename_len - 1] == 'm' &&
                 argv[2][filename_len - 2] == 'c' &&
                 argv[2][filename_len - 3] == 's' &&
                 argv[2][filename_len - 4] == '.')) {
                error_handle(stderr, "unexcepted file type, require .scm\n", EXIT_FAILURE);
            }

            FILE* source_file = fopen(argv[2], "r");
            if(source_file == NULL) {
                char buf[100];
                sprintf(buf, "cannot open file %s\n", argv[2]);
                error_handle(stderr, buf, EXIT_FAILURE);
            }

            /* start the interpreter process */
            object* obj, * result;
            init_built_in();
            print_prompt();
            printf("> evaluating %s\n", argv[2]);

            token_list* list = gen_token_list(
                    gen_token(
                            buf_pre_handle(
                                    read(
                                            source_file))));

            while(list->token_pointer != NULL) {
                obj = parse(list);
                result = eval(obj, the_global_environment);
                if(result == NULL) {
                    error_handle(stderr, "eval return a null object\n", EXIT_FAILURE);
                }
                if(result != ok_symbol) {
                    write(stdout, result);
                    printf("\n");
                }
            }
            repl();
        }
    }
}
