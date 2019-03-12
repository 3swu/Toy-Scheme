//
// Created by wulei on 19-3-12.
//
// read & parse

#include "header/read.h"
#include "header/error.h"

#include <stdio.h>
#include <stdlib.h>

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