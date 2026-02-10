//
// Created by wulei on 19-3-14.
//

#include <stdio.h>
#include <stdlib.h>
#include "header/environment.h"
#include "header/error.h"
#include "header/read.h"
#include "header/object.h"

object* lookup_variable_value(object* var, object* env) {
    while(!is_empty_list(env)) {
        object* frame = first_frame(env);
        object* vars = frame_variables(frame);
        object* vals = frame_values(frame);
        while(!is_empty_list(vars)) {
            if(!is_pair(vars) || !is_pair(vals))
                error_handle(stderr, "invalid environment frame", EXIT_FAILURE);

            if(var == car(vars))
                return car(vals);

            vars = cdr(vars);
            vals = cdr(vals);
        }
        if(!is_empty_list(vals))
            error_handle(stderr, "invalid environment frame", EXIT_FAILURE);
        env = enclosing_environment(env);
    }
    char error_msg[TOKEN_MAX + 50];
    sprintf(error_msg, "undefined variable: %s\n", var->data.symbol.value);
    error_handle(stderr, error_msg, EXIT_FAILURE);
    return NULL;
}

object* extend_environment(object* variables,
                           object* values,
                           object* base_env) {
    object* vars_iter = variables;
    object* vals_iter = values;

    /* variadic form: (lambda args body...) */
    if(is_symbol(variables))
        return cons(make_frame(cons(variables, the_empty_list),
                               cons(values, the_empty_list)),
                    base_env);

    while(!is_empty_list(vars_iter) && !is_empty_list(vals_iter)) {
        if(!is_pair(vars_iter))
            error_handle(stderr, "invalid parameter list", EXIT_FAILURE);
        if(!is_pair(vals_iter))
            error_handle(stderr, "invalid argument list", EXIT_FAILURE);

        vars_iter = cdr(vars_iter);
        vals_iter = cdr(vals_iter);
    }

    if(!is_empty_list(vars_iter)) {
        if(!is_pair(vars_iter))
            error_handle(stderr, "invalid parameter list", EXIT_FAILURE);
        error_handle(stderr, "too few arguments supplied", EXIT_FAILURE);
    }

    if(!is_empty_list(vals_iter)) {
        if(!is_pair(vals_iter))
            error_handle(stderr, "invalid argument list", EXIT_FAILURE);
        error_handle(stderr, "too many arguments supplied", EXIT_FAILURE);
    }

    return cons(make_frame(variables, values), base_env);
}

void set_variable_value(object* var, object* value, object* env) {
    while(!is_empty_list(env)) {
        object* frame = first_frame(env);
        object* vars = frame_variables(frame);
        object* vals = frame_values(frame);

        while(!is_empty_list(vars)) {
            if(!is_pair(vars) || !is_pair(vals))
                error_handle(stderr, "invalid environment frame", EXIT_FAILURE);

            if(var == car(vars)){
                set_car(vals, value);
                return;
            }
            vars = cdr(vars);
            vals = cdr(vals);
        }
        if(!is_empty_list(vals))
            error_handle(stderr, "invalid environment frame", EXIT_FAILURE);
        env = enclosing_environment(env);
    }

    char error_msg[TOKEN_MAX + 50];
    sprintf(error_msg, "undefined variable: %s\n", var->data.symbol.value);
    error_handle(stderr, error_msg, EXIT_FAILURE);
}

void define_variable(object* var, object* val, object* env) {
    if(!is_empty_list(env)) {
        object* frame = first_frame(env);
        object* vars = frame_variables(frame);
        object* vals = frame_values(frame);

        while(!is_empty_list(vars)) {
            if(!is_pair(vars) || !is_pair(vals))
                error_handle(stderr, "invalid environment frame", EXIT_FAILURE);

            if(var == car(vars)) {
                set_car(vals, val);
                return;
            }
            vars = cdr(vars);
            vals = cdr(vals);
        }
        if(!is_empty_list(vals))
            error_handle(stderr, "invalid environment frame", EXIT_FAILURE);

        /* undefined in first frame, add binding */
        add_binding_to_frame(var, val, frame);
    }
}

object* enclosing_environment(object* env) {
    return cdr(env);
}

object* first_frame(object* env) {
    return car(env);
}

object* make_frame(object* variables, object* values) {
    return cons(variables, values);
}

object* frame_variables(object* frame) {
    return car(frame);
}

object* frame_values(object* frame) {
    return cdr(frame);
}

void add_binding_to_frame(object* var, object* val, object* frame) {
    set_car(frame, cons(var, car(frame)));
    set_cdr(frame, cons(val, cdr(frame)));
}
