//
// Created by wulei on 19-3-12.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header/object.h"
#include "header/error.h"

static object* gc_allocated_objects = NULL;

static char* copy_string(const char* str) {
    size_t len;
    char* dst;

    if(str == NULL)
        return NULL;

    len = strlen(str);
    dst = (char*) malloc((len + 1) * sizeof(char));
    if(dst == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    memcpy(dst, str, len + 1);
    return dst;
}

static void gc_mark(object* obj) {
    if(obj == NULL || obj->gc_marked)
        return;

    obj->gc_marked = true;
    switch(obj->type) {
        case PAIR:
            gc_mark(obj->data.pair.car);
            gc_mark(obj->data.pair.cdr);
            break;
        case COMPOUND_PROC:
            gc_mark(obj->data.compound_proc.parameters);
            gc_mark(obj->data.compound_proc.body);
            gc_mark(obj->data.compound_proc.env);
            break;
        default:
            break;
    }
}

static void gc_mark_roots(void) {
    gc_mark(true_obj);
    gc_mark(false_obj);
    gc_mark(the_empty_list);
    gc_mark(symbol_table);
    gc_mark(quote_symbol);
    gc_mark(define_symbol);
    gc_mark(set_symbol);
    gc_mark(ok_symbol);
    gc_mark(if_symbol);
    gc_mark(lambda_symbol);
    gc_mark(begin_symbol);
    gc_mark(cond_symbol);
    gc_mark(else_symbol);
    gc_mark(let_symbol);
    gc_mark(and_symbol);
    gc_mark(or_symbol);
    gc_mark(eof_object);
    gc_mark(the_empty_environment);
    gc_mark(the_global_environment);
}

static void gc_sweep(void) {
    object** current = &gc_allocated_objects;

    while(*current != NULL) {
        object* obj = *current;
        if(!obj->gc_marked) {
            *current = obj->gc_next;
            if(obj->type == SYMBOL && obj->data.symbol.value != NULL)
                free(obj->data.symbol.value);
            if(obj->type == STRING && obj->data.string.value != NULL)
                free(obj->data.string.value);
            free(obj);
        }
        else {
            obj->gc_marked = false;
            current = &obj->gc_next;
        }
    }
}

object *true_obj = NULL;
object *false_obj = NULL;
object *the_empty_list = NULL;
object *symbol_table = NULL;
object *quote_symbol = NULL;
object *define_symbol = NULL;
object *set_symbol = NULL;
object *ok_symbol = NULL;
object *if_symbol = NULL;
object *lambda_symbol = NULL;
object *begin_symbol = NULL;
object *cond_symbol = NULL;
object *else_symbol = NULL;
object *let_symbol = NULL;
object *and_symbol = NULL;
object *or_symbol = NULL;
object *eof_object = NULL;
object *the_empty_environment = NULL;
object *the_global_environment = NULL;

object* alloc_object() {
    object* obj = (object*) malloc (sizeof(object));

    if(obj == NULL){
        error_handle(stderr, "out of memory", EXIT_FAILURE);
    }
    obj->gc_marked = false;
    obj->gc_next = gc_allocated_objects;
    gc_allocated_objects = obj;
    return obj;
}

void gc_collect(void) {
    gc_mark_roots();
    gc_sweep();
}

bool is_empty_list(object* obj) {
    return obj->type == THE_EMPTY_LIST ? true : false;
}

bool is_boolean(object* obj) {
    return obj->type == BOOLEAN ? true : false;
}

bool is_symbol(object* obj) {
    return obj->type == SYMBOL ? true : false;
}

bool is_fixnum(object* obj) {
    return obj->type == FIXNUM ? true : false;
}

bool is_character(object* obj) {
    return obj->type == CHARACTER ? true : false;
}

bool is_string(object* obj) {
    return obj->type == STRING ? true : false;
}

bool is_pair(object* obj) {
    return obj->type == PAIR ? true : false;
}

bool is_primitive_proc(object* obj) {
    return obj->type == PRIMITIVE_PROC ? true : false;
}

bool is_compound_proc(object* obj) {
    return obj->type == COMPOUND_PROC ? true : false;
}

bool is_true(object* obj) {
    return obj != NULL && !is_false(obj);
}

bool is_false(object* obj) {
    return obj != NULL &&
           obj->type == BOOLEAN &&
           obj->data.boolean.value == false ? true : false;
}

object* car(object* pair) {
    return pair->data.pair.car;
}

object* cdr(object* pair) {
    return pair->data.pair.cdr;
}

object* cons(object* car, object* cdr) {
    object* pair = alloc_object();
    pair->type = PAIR;
    pair->data.pair.car = car;
    pair->data.pair.cdr = cdr;

    return pair;
}

void set_car(object* pair, object* car) {
    pair->data.pair.car = car;
}

void set_cdr(object* pair, object* cdr) {
    pair->data.pair.cdr = cdr;
}

//object* make_the_empty_list() {
//    object* obj = alloc_object();
//    obj->type = THE_EMPTY_LIST;
//    return obj;
//}

object* make_boolean(bool value) {
    object* obj = alloc_object();
    obj->type = BOOLEAN;
    if(value == true)
        obj->data.boolean.value = true;
    else
        obj->data.boolean.value = false;

    return obj;
}

object* make_fixnum(long value) {
     object* obj = alloc_object();
     obj->type = FIXNUM;
     obj->data.fixnum.value = value;

     return obj;
}

object* make_string(char* str) {

    object* obj = alloc_object();
    obj->type = STRING;
    obj->data.string.value = copy_string(str);
    return obj;
}

object* make_symbol(char* str) {
    /* init symbol table if it is not exist */
//    if(symbol_table == NULL)
//        symbol_table = make_symbol_table();

    /* if symbol table contain the symbol */
    for(object* obj = symbol_table;
        !is_empty_list(obj); obj = cdr(obj))
        if(strcmp(car(obj)->data.symbol.value, str) == 0)
            return car(obj);

    /* create symbol and add into symbol table */
    object* obj = alloc_object();
    obj->type = SYMBOL;
    obj->data.symbol.value = copy_string(str);

    symbol_table = cons(obj, symbol_table);
    return obj;
}

//object* make_symbol_table() {
//    object* obj = alloc_object();
//    obj->type = THE_EMPTY_LIST;
//
//    return obj;
//}

//object* get_the_empty_environment() {
//    if(the_empty_environment == NULL){
//        the_empty_environment = make_the_empty_list();
//        return make_the_empty_list();
//    }
//    return the_empty_environment;
//}
