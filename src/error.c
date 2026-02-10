//
// Created by wulei on 19-3-12.
//

#include <stdio.h>
#include <stdlib.h>
#include "header/error.h"
#include "header/read.h"
#include "header/object.h"

void error_handle(FILE* out, char* msg, int exit_code) {
    fprintf(out, "%s", msg);
    exit(exit_code);
}

void error_handle_with_object(FILE* out,
                              char* msg,
                              int exit_code,
                              object* obj) {
    char error_msg[TOKEN_MAX + 50];

    if(obj != NULL) {
        switch(obj->type) {
            case SYMBOL:
                sprintf(error_msg, "%s: %s", msg, obj->data.symbol.value);
                fprintf(out, "%s", error_msg);
                exit(exit_code);
            case STRING:
                sprintf(error_msg, "%s: %s", msg, obj->data.string.value);
                fprintf(out, "%s", error_msg);
                exit(exit_code);
            case FIXNUM:
                sprintf(error_msg, "%s: %ld", msg, obj->data.fixnum.value);
                fprintf(out, "%s", error_msg);
                exit(exit_code);
            default:
                fprintf(out, "%s", msg);
                exit(exit_code);
        }
    }
    else {
        fprintf(out, "%s", msg);
        exit(exit_code);
    }
}
