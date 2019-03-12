//
// Created by wulei on 19-3-12.
//
// read source code

#include <bits/types/FILE.h>

#ifndef SCHEME_READ_H
#define SCHEME_READ_H

#define TOKEN_MAX 50

typedef struct token {
    char* value;
    struct token* next;
} token;


/***** read *****/
char* read(FILE* in_stream);

char* buf_pre_handle(char* buf);

token* gen_token(char* buf);

/***** parse *****/

#endif //SCHEME_READ_H
