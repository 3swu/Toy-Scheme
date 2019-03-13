//
// Created by wulei on 19-3-12.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "header/object.h"
#include "header/error.h"

object* alloc_object() {
    object* obj = (object*) malloc (sizeof(object));

    if(obj == NULL){
        error_handle(stderr, "out of memory", EXIT_FAILURE);
    }
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

object* make_the_empty_list() {
    object* obj = alloc_object();
    obj->type = THE_EMPTY_LIST;
    return obj;
}

object* make_boolean(bool value) {
    object* obj = alloc_object();
    obj->type = BOOLEAN;
    if(value == true)
        obj->data.boolean.value = true;
    else
        obj->data.boolean.value = false;

    return obj;
}

object* make_fixnum(char* str) {
     object* obj = alloc_object();
     obj->type = FIXNUM;
     obj->data.fixnum.value = atol(str);

     return obj;
}

object* make_string(char* str) {
    char* s = (char*) malloc(strlen(str) * sizeof(char));
    if(s == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    int i = 0;
    for(; str[i + 1] != '"'; i++)
        s[i] = str[i + 1];
    s[i + 1] = '\0';

    object* obj = alloc_object();
    obj->type = STRING;
    obj->data.string.value = s;
    return obj;
}