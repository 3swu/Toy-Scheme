//
// Created by wulei on 19-3-12.
//
// read & parse

#include "header/read.h"
#include "header/error.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#define MAXSIZE 1024

char* read(FILE* in_stream) { /* from in_stream to buffer */
    char *buf, ch;
    int i = 0;

    buf = (char*) malloc(MAXSIZE * sizeof(char));
    if(buf == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    while((ch = getc(in_stream)) != EOF && ch != '\n') {
        buf[i++] = ch;

        if(i >= MAXSIZE) {
            buf = (char*) realloc(buf, MAXSIZE * 10 * sizeof(char));
            if(buf == NULL)
                error_handle(stderr, "out of memory", EXIT_FAILURE);
        }
    }
    buf[i] = '\0';

    return buf;
}

char* buf_pre_handle(char* pre_buf) { /* add space and remove comments */
    char* buf;
    buf = (char*) malloc(MAXSIZE * sizeof(char));
    if(buf == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    int buf_i = 0, pbuf_i = 0;
    while(pre_buf[pbuf_i] != '\0') {
        if(pre_buf[pbuf_i] == '(') {
            buf[buf_i++] = ' ';
            buf[buf_i++] = '(';
            buf[buf_i++] = ' ';
            pbuf_i++;
        }
        if(pre_buf[pbuf_i] == ')') {
            buf[buf_i++] = ' ';
            buf[buf_i++] = ')';
            buf[buf_i++] = ' ';
            pbuf_i++;
        }
        if(pre_buf[pbuf_i] == ';') {
            while(pre_buf[pbuf_i] != '\n')
                pbuf_i++;
        }
        buf[buf_i++] = pre_buf[pbuf_i++];

        if(buf_i > MAXSIZE - 3) {
            buf = (char*) realloc(buf, MAXSIZE * 10 * sizeof(char));
            if(buf == NULL)
                error_handle(stderr, "out of memory", EXIT_FAILURE);
        }
    }
    buf[buf_i] = '\0';
    return buf;
}

token* gen_token(char* buf) {
    token* token_list, * token_p; /*init token list with head token*/
    token_list = (token*) malloc(sizeof(token));
    if(token_list == NULL)
        error_handle(stderr, "out of memory while parse token", EXIT_FAILURE);
    token_list->value = "Head Token";
    token_p = token_list;

    int i = 0, j = 0;
    while(1) {
        token* t = (token*) malloc(sizeof(token));
        t->value = (char*) malloc(TOKEN_MAX * sizeof(char));
        if(t == NULL || t->value == NULL)
            error_handle(stderr, "out of memory while parse token", EXIT_FAILURE);
        t->next = NULL;

//        int len = strlen(buf);
        for(; buf[i] == ' ' || buf[i] == '\n'; i++)
            if(i >= strlen(buf) - 1 )
                return token_list;

        for(j = i; buf[j] != ' '; j++)
            if(buf[j] == '\0') {
                t->value = ")";
                token_p->next = t;
                return token_list;
            }

        if(j - i >= TOKEN_MAX) {
            t->value = (char*) realloc(t->value, TOKEN_MAX * 10 * sizeof(char));
            if(t->value == NULL) {
                error_handle(stderr, "out of memory while parse token", EXIT_FAILURE);
            }
        }

        int t_i = 0;
        for(int k = i; k < j; k++, t_i++)
            t->value[t_i] = buf[k];
        t->value[t_i] = '\0';
        i = j;

        token_p->next = t;
        token_p = t;
    }

}

token_list* gen_token_list(token* token_l) {
    token_list* list = (token_list*) malloc(sizeof(token_list));
    if(list == NULL)
        error_handle(stderr, "out of memory while generate token list", EXIT_FAILURE);

    list->haed_token = token_l;
    list->token_pointer = token_l->next;

    return list;
}

object* parse(token_list* list) {
    char* token_value = list->token_pointer->value;

    if(strcmp(token_value, "(") == 0) {
        list_iter(list);
        return parse_pair(list);
    }

    if(is_str_symbol(token_value)) {
        list_iter(list);
        return make_symbol(token_value);
    }

    if(is_str_digit(token_value)) {
        list_iter(list);
        return make_fixnum(token_value);
    }

    if(is_str_string(token_value)) {
        list_iter(list);
        return make_string(token_value);
    }

    if(strcmp(token_value, "#t") == 0) {
        list_iter(list);
        return make_boolean(true);
    }

    if(strcmp(token_value, "#f") == 0) {
        list_iter(list);
        return make_boolean(false);
    }

    char error_msg[TOKEN_MAX + 50];
    sprintf(error_msg, "unexcepted symbol : %s", token_value);
    error_handle(stderr, error_msg, EXIT_FAILURE);

}

object* parse_pair(token_list* list) {
    if(strcmp(list->token_pointer->value, ")") == 0) {
        list_iter(list);
        return the_empty_list;
    }

    object * car, * cdr;
    car = parse(list);
    cdr = parse_pair(list);
    return cons(car, cdr);
}

bool is_str_digit(char* str) {
    for(int i = 0; i < strlen(str); i++)
        if(!isdigit(str[i]) && i != 0)
            return false;
    return true;
}

bool is_str_string(char* str) {
    return str[0] == '"' && str[strlen(str) - 1] == '"' ? true : false;
}

bool is_str_symbol(char* str) {
    char c = str[0];
    if(isalpha(c) ||
       c == '*' ||
       c == '/' ||
       c == '+' ||
       c == '-' ||
       c == '>' ||
       c == '<' ||
       c == '=' ||
       c == '?' ||
       c == '!')
        return true;
    return false;
}

void list_iter(token_list* list) {
    list->token_pointer = list->token_pointer->next;
}