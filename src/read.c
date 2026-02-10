//
// Created by wulei on 19-3-12.
//
// read & parse

#include "header/read.h"
#include "header/error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXSIZE 10240

char* read(FILE* in_stream) { /* from in_stream to buffer */
    char *buf;
    int ch;
    int i = 0;
    size_t capacity = MAXSIZE;

    buf = (char*) malloc(capacity * sizeof(char));
    if(buf == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    if(in_stream == stdin) {
        ch = getc(in_stream);
        if(ch == EOF) {
            free(buf);
            return NULL;
        }

        while(ch != EOF && ch != '\n') {
            buf[i++] = (char)ch;

            if((size_t)i >= capacity - 1) {
                capacity *= 2;
                buf = (char*) realloc(buf, capacity * sizeof(char));
                if(buf == NULL)
                    error_handle(stderr, "out of memory", EXIT_FAILURE);
            }
            ch = getc(in_stream);
        }
    }
    else {
        while((ch = getc(in_stream)) != EOF) {
            buf[i++] = (char)ch;

            if((size_t)i >= capacity - 1) {
                capacity *= 2;
                buf = (char*) realloc(buf, capacity * sizeof(char));
                if(buf == NULL)
                    error_handle(stderr, "out of memory", EXIT_FAILURE);
            }
        }
    }
    buf[i] = '\0';

    return buf;
}

char* buf_pre_handle(char* pre_buf) { /* add space and remove comments */
    char* buf;
    bool in_string = false;
    size_t capacity = MAXSIZE;
    buf = (char*) malloc(capacity * sizeof(char));
    if(buf == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    int buf_i = 0, pbuf_i = 0;
    while(pre_buf[pbuf_i] != '\0') {
        if(in_string) {
            buf[buf_i++] = pre_buf[pbuf_i];
            if(pre_buf[pbuf_i] == '"')
                in_string = false;
            pbuf_i++;
            continue;
        }

        switch(pre_buf[pbuf_i]) {
            case '"':
                in_string = true;
                buf[buf_i++] = pre_buf[pbuf_i++];
                break;
            case '(':
                buf[buf_i++] = ' ';
                buf[buf_i++] = '(';
                buf[buf_i++] = ' ';
                pbuf_i++;
                break;
            case ')':
                buf[buf_i++] = ' ';
                buf[buf_i++] = ')';
                buf[buf_i++] = ' ';
                pbuf_i++;
                break;
            case ';':
                while(pre_buf[pbuf_i] != '\0' && pre_buf[pbuf_i] != '\n')
                    pbuf_i++;
                break;
            case '\n':
                buf[buf_i++] = ' ';
                buf[buf_i++] = '\n';
                pbuf_i++;
                break;
            default:
                buf[buf_i++] = pre_buf[pbuf_i++];
        }

        if((size_t)buf_i >= capacity - 3) {
            capacity *= 2;
            buf = (char*) realloc(buf, capacity * sizeof(char));
            if(buf == NULL)
                error_handle(stderr, "out of memory", EXIT_FAILURE);
        }
    }
    buf[buf_i] = '\0';
    return buf;
}

token* gen_token(char* buf) {
    token* token_list, * token_p; /*init token list with head token*/
    size_t i = 0;
    size_t j = 0;
    size_t len = strlen(buf);

    token_list = (token*) malloc(sizeof(token));
    if(token_list == NULL)
        error_handle(stderr, "out of memory while parse token", EXIT_FAILURE);
    token_list->value = NULL;
    token_list->next = NULL;
    token_p = token_list;

    while(i < len) {
        size_t token_len;
        token* t = (token*) malloc(sizeof(token));
        if(t == NULL)
            error_handle(stderr, "out of memory while parse token", EXIT_FAILURE);

        while(i < len && (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t' || buf[i] == '\r'))
            i++;
        if(i >= len) {
            free(t);
            break;
        }

        if(buf[i] == '"') {
            j = i + 1;
            while(j < len && buf[j] != '"')
                j++;
            if(j >= len)
                error_handle(stderr, "unterminated string literal\n", EXIT_FAILURE);
            j++;
        }
        else {
            j = i;
            while(j < len &&
                  buf[j] != ' ' &&
                  buf[j] != '\n' &&
                  buf[j] != '\t' &&
                  buf[j] != '\r')
                j++;
        }

        token_len = j - i;
        t->value = (char*) malloc((token_len + 1) * sizeof(char));
        if(t->value == NULL)
            error_handle(stderr, "out of memory while parse token", EXIT_FAILURE);
        t->next = NULL;

        memcpy(t->value, buf + i, token_len);
        t->value[token_len] = '\0';
        i = j;

        token_p->next = t;
        token_p = t;
    }
    return token_list;
}

token_list* gen_token_list(token* token_l) {
    token_list* list = (token_list*) malloc(sizeof(token_list));
    if(list == NULL)
        error_handle(stderr, "out of memory while generate token list", EXIT_FAILURE);

    list->haed_token = token_l;
    list->token_pointer = token_l->next;

    return list;
}

void destroy_token_list(token_list* list) {
    token* current;
    token* next;

    if(list == NULL)
        return;

    current = list->haed_token;
    while(current != NULL) {
        next = current->next;
        if(current->value != NULL)
            free(current->value);
        free(current);
        current = next;
    }
    free(list);
}

object* parse(token_list* list) {
    char* token_value = list->token_pointer->value;

    if(strcmp(token_value, "(") == 0) {
        list_iter(list);
        return parse_pair(list);
    }

    if(is_str_digit(token_value)) {
        list_iter(list);
        return make_fixnum(atol(token_value));
    }

    if(is_str_symbol(token_value)) {
        list_iter(list);
        return make_symbol(token_value);
    }

    if(is_str_string(token_value)) {
        list_iter(list);
        return parse_string(token_value);
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
    return NULL;
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
    int i = 0;

    if(str == NULL || str[0] == '\0')
        return false;

    if(str[0] == '+' || str[0] == '-') {
        if(str[1] == '\0')
            return false;
        i = 1;
    }

    for(; str[i] != '\0'; i++)
        if(!isdigit((unsigned char)str[i]))
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

object* parse_string(char* str) {
    size_t str_len = strlen(str);
    char* s = (char*) malloc((str_len - 1) * sizeof(char));
    object* string_obj;
    if(s == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    for(size_t i = 0; i < str_len - 2; i++)
        s[i] = str[i + 1];
    s[str_len - 2] = '\0';

    string_obj = make_string(s);
    free(s);
    return string_obj;
}

object* reader(FILE* in) {
    while(true) {
        char* pre_buf = read(in);
        char* buf;
        token* t;
        token_list* list;
        object* obj;

        if(pre_buf == NULL)
            return NULL;

        buf = buf_pre_handle(pre_buf);
        free(pre_buf);

        t = gen_token(buf);
        free(buf);

//        for(token* p = t; p != NULL; p = p->next)
//            printf("%s\n", p->value);

        list = gen_token_list(t);
        if(list->token_pointer == NULL) {
            destroy_token_list(list);
            if(in == stdin)
                continue;
            return NULL;
        }
        obj = parse(list);
        destroy_token_list(list);
        return obj;
    }
}
