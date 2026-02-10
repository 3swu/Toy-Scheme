#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

#include "src/header/read.h"
#include "src/header/builtin.h"
#include "src/header/eval.h"
#include "src/header/environment.h"
#include "src/header/write.h"
#include "src/header/error.h"

void print_prompt() {
    printf("Welcome to Toy-Scheme\nPress Ctrl-C to exit\n");
    fflush(stdout);
}

static bool has_scm_suffix(const char* path) {
    size_t filename_len = strlen(path);
    return filename_len >= 4 && strcmp(path + filename_len - 4, ".scm") == 0;
}

void repl() {
    jmp_buf recovery_point;
    object* obj, * result;
    set_error_recovery(&recovery_point);

    for(; ;) {
        if(setjmp(recovery_point) != 0) {
            gc_collect();
        }
        printf("> ");
        fflush(stdout);
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
            fflush(stdout);
        }
        gc_collect();
    }
    clear_error_recovery();
}

static void eval_source_file(FILE* source_file) {
    char* pre_buf = read(source_file);
    char* buf;
    token* tokens;
    token_list* list;
    jmp_buf recovery_point;

    if(pre_buf == NULL)
        return;

    buf = buf_pre_handle(pre_buf);
    free(pre_buf);

    tokens = gen_token(buf);
    free(buf);

    list = gen_token_list(tokens);

    set_error_recovery(&recovery_point);
    while(list->token_pointer != NULL) {
        volatile token* token_before = list->token_pointer;
        object* obj;
        object* result;

        if(setjmp(recovery_point) != 0) {
            gc_collect();
            if(list->token_pointer == token_before && list->token_pointer != NULL)
                list_iter(list);
            continue;
        }
        obj = parse(list);
        result = eval(obj, the_global_environment);
        if(result == NULL) {
            error_handle(stderr, "eval return a null object\n", EXIT_FAILURE);
        }
        if(result != ok_symbol) {
            write(stdout, result);
            printf("\n");
            fflush(stdout);
        }
        gc_collect();
    }
    clear_error_recovery();
    destroy_token_list(list);
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
            if(!has_scm_suffix(argv[2])) {
                error_handle(stderr, "unexcepted file type, require .scm\n", EXIT_FAILURE);
            }

            FILE* source_file = fopen(argv[2], "r");
            if(source_file == NULL) {
                char buf[100];
                sprintf(buf, "cannot open file %s\n", argv[2]);
                error_handle(stderr, buf, EXIT_FAILURE);
            }

            /* start the interpreter process */
            init_built_in();
            print_prompt();
            printf("> evaluating %s\n", argv[2]);
            fflush(stdout);
            eval_source_file(source_file);
            fclose(source_file);
            repl();
        }
    }
}
