//
// Created by wulei on 19-3-26.
//
// built in procedures and objects

#include <memory.h>
#include "header/builtin.h"
#include "header/object.h"

void init_built_in() {
    true_obj = alloc_object(); /* init true_obj */
    true_obj->type = BOOLEAN;
    true_obj->data.boolean.value = true;

    false_obj = alloc_object(); /* init false_obj */
    false_obj->type = BOOLEAN;
    false_obj->data.boolean.value = false;

    the_empty_list = alloc_object(); /* init the_empty_list */
    the_empty_list->type = THE_EMPTY_LIST;
    symbol_table = the_empty_list;

    quote_symbol  = make_symbol("quote" );
    define_symbol = make_symbol("define");
    set_symbol    = make_symbol("set!"  );
    ok_symbol     = make_symbol("ok"    );
    if_symbol     = make_symbol("if"    );
    lambda_symbol = make_symbol("lambda");
    begin_symbol  = make_symbol("begin" );
    cond_symbol   = make_symbol("cond"  );
    else_symbol   = make_symbol("else"  );
    let_symbol    = make_symbol("let"   );
    and_symbol    = make_symbol("and"   );
    or_symbol     = make_symbol("or"    );

    the_empty_environment = the_empty_list;
//TODO
// make the global environment, include init the env
// and binding the builtin procedure to env

}

object* make_compound_procedure(object* parameters, object* body, object* env) {
    object* obj = alloc_object();

    obj->type = COMPOUND_PROC;
    obj->data.compound_proc.parameters = parameters;
    obj->data.compound_proc.body       = body;
    obj->data.compound_proc.env        = env;
    return env;
}

/* implement of built-in procedures */
static object* add_procedure(object* arguments) {
    long result = 0;
    while(!is_empty_list(arguments)) {
        result += (car(arguments)->data.fixnum.value);
        arguments = cdr(arguments);
    }
    return make_fixnum(result);
}

static object* sub_procedure(object* arguments) {
    long result = car(arguments)->data.fixnum.value;
    arguments = cdr(arguments);

    while(!is_empty_list(arguments)) {
        result -= car(arguments)->data.fixnum.value;
        arguments = cdr(arguments);
    }
    return make_fixnum(result);
}

static object* mul_procedure(object* arguments) {
    long result = 1;

    while(!is_empty_list(arguments)) {
        result *= car(arguments)->data.fixnum.value;
        arguments = cdr(arguments);
    }

    return make_fixnum(result);
}

static object* div_procedure(object* arguments) {
    return make_fixnum(car(arguments)->data.fixnum.value /
                        cadr(arguments)->data.fixnum.value);
}

static object* remainder_procedure(object* arguments) {
    return make_fixnum(car(arguments)->data.fixnum.value %
                        cadr(arguments)->data.fixnum.value);
}

static object* is_num_equal_procedure(object* arguments) {
    long value = car(arguments)->data.fixnum.value;

    while(!is_empty_list(arguments = cdr(arguments))) {
        if(value != car(arguments)->data.fixnum.value)
            return false_obj;
    }
    return true_obj;
}

static object* is_less_procedure(object* arguments) {
    long previous = car(arguments)->data.fixnum.value;
    long next;

    while(!is_empty_list(arguments = cdr(arguments))) {
        next = car(arguments)->data.fixnum.value;
        if(previous > next)
            return false_obj;
    }
    return true_obj;
}

static object* is_greater_procedure(object* arguments) {
    long previous = car(arguments)->data.fixnum.value;
    long next;

    while(!is_empty_list(arguments = cdr(arguments))) {
        next = car(arguments)->data.fixnum.value;
        if(previous < next)
            return false_obj;
    }
    return true_obj;
}

static object* cons_procedure(object* arguments) {
    return cons(car(arguments), cadr(arguments));
}

static object* car_procedure(object* arguments) {
    return caar(arguments);
}

static object* cdr_procedure(object* arguments) {
    return cdar(arguments);
}

static object* set_car_procedure(object* arguments) {
    set_car(car(arguments), cadr(arguments));
    return ok_symbol;
}

static object* set_cdr_procedure(object* arguments) {
    set_cdr(car(arguments), cadr(arguments));
    return ok_symbol;
}

static object* list_procedure(object* arguments) {
    return arguments;
}

static object* is_equal_procedure(object* arguments) {
    object* first = car(arguments);
    object* second = cadr(arguments);

    if(first->type != second->type)
        return false_obj;

    switch(first->type) {
        case FIXNUM:
            return first->data.fixnum.value == second->data.fixnum.value ?
                    true_obj : false_obj;

        case STRING:
            return strcmp(first->data.string.value, second->data.string.value) == 0 ?
                    true_obj : false_obj;
        default:
            return first == second ? true_obj : false_obj;
    }
}

