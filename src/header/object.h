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
    bool gc_marked;
    struct object* gc_next;
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
            struct object* (*fun) (struct object* argument);
        } primitive_proc;
        struct {
            struct object* parameters;
            struct object* body;
            struct object* env;
        } compound_proc;
    } data;
} object;

extern object* alloc_object();

extern bool is_empty_list    (object* obj);

extern bool is_boolean       (object* obj);

extern bool is_symbol        (object* obj);

extern bool is_fixnum        (object* obj);

extern bool is_character     (object* obj);

extern bool is_string        (object* obj);

extern bool is_pair          (object* obj);

extern bool is_primitive_proc(object* obj);

extern bool is_compound_proc (object* obj);

extern bool is_true          (object* obj);

extern bool is_false         (object* obj);

extern object* car(object* pair);

extern object* cdr(object* pair);

extern object* cons(object* car, object* cdr);

extern void set_car(object* pair, object* car);

extern void set_cdr(object* pair, object* cdr);

/**** object constructor ****/
extern object* make_the_empty_list();

extern object* make_boolean(bool value);

extern object* make_fixnum(long value);

extern object* make_string(char* str);

extern object* make_symbol(char* str);

extern void gc_collect(void);

/**** global object constructor ****/
extern object* make_symbol_table();

extern object* get_the_empty_environment();

extern object *true_obj;
extern object *false_obj;
extern object *the_empty_list;
extern object *symbol_table;
extern object *quote_symbol;
extern object *define_symbol;
extern object *set_symbol;
extern object *ok_symbol;
extern object *if_symbol;
extern object *lambda_symbol;
extern object *begin_symbol;
extern object *cond_symbol;
extern object *else_symbol;
extern object *let_symbol;
extern object *and_symbol;
extern object *or_symbol;
extern object *eof_object;
extern object *the_empty_environment;
extern object *the_global_environment;

#endif //SCHEME_OBJECT_H
