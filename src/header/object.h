//
// Created by wulei on 19-3-12.
//
// The internal model of Scheme

#ifndef SCHEME_OBJECT_H
#define SCHEME_OBJECT_H

#include <stdbool.h>

typedef enum {THE_EMPTY_LIST, BOOLEAN, SYMBOL,
              FIXNUM, CHARACTER, STRING, PAIR,
              PRIMITIVE_PROC, COMPOUND_PROC}
              object_type;

typedef struct object {
    object_type type;
    union {
        struct {
            bool value;
        } boolean;
        struct {
            char* value;
        } symbol;
        struct {
            long value;
        } fixnum;
        struct {
            char value;
        } character;
        struct {
            char* value;
        } string;
        struct {
            struct object* car;
            struct object* cdr;
        } pair;
        struct {
            struct objetc* (*fun) (struct object* arguement);
        } primitive_proc;
        struct {
            struct object* parameters;
            struct object* body;
            struct object* env;
        } compound_proc;
    } data;
} object;

extern object* alloc_object();

extern bool is_boolean       (object* obj);

extern bool is_symbol        (object* obj);

extern bool is_fixnum        (object* obj);

extern bool is_character     (object* obj);

extern bool is_string        (object* obj);

extern bool is_pair          (object* obj);

extern bool is_primitive_proc(object* obj);

extern bool is_compound_proc (object* obj);

extern object* car(object* pair);

extern object* cdr(object* pair);

extern void set_car(object* pair, object* car);

extern void set_cdr(object* pair, object* cdr);

#endif //SCHEME_OBJECT_H
