//
// Created by wulei on 19-3-12.
//
// read source code

#include <bits/types/FILE.h>
#include "object.h"

#ifndef SCHEME_READ_H
#define SCHEME_READ_H

#define TOKEN_MAX 50

typedef struct token {
    char* value;
    struct token* next;
} token;

typedef struct {
    token* haed_token;
    token* token_pointer;
} token_list;

/***** read *****/
char* read(FILE* in_stream);

char* buf_pre_handle(char* buf);

token* gen_token(char* buf);

token_list* gen_token_list(token* token_l);

/***** parse *****/
extern
object* parse(token_list* list);

object* parse_pair(token_list* list);

/***** more *****/
bool is_str_digit(char* str);

bool is_str_string(char* str);

bool is_str_symbol(char* str);

void  list_iter(token_list* list);
#endif //SCHEME_READ_H
