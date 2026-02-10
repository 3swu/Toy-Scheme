//
// Created by wulei on 19-3-26.
//
// built in procedures and objects

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "header/builtin.h"
#include "header/object.h"
#include "header/environment.h"
#include "header/error.h"
#include "header/read.h"
#include "header/eval.h"

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
    let_star_symbol = make_symbol("let*");
    letrec_symbol   = make_symbol("letrec");
    and_symbol    = make_symbol("and"   );
    or_symbol     = make_symbol("or"    );
    unassigned_symbol = make_symbol("unassigned");

    the_empty_environment = the_empty_list;
    the_global_environment = make_environment();

}

object* make_environment() {
    object* env;
    env = setup_environment();
    add_primitive_to_environment(env);
    return env;
}

object* setup_environment() {
    object* env = extend_environment(the_empty_list, the_empty_list, the_empty_environment);
    return env;
}

object* make_compound_procedure(object* parameters, object* body, object* env) {
    object* obj = alloc_object();

    obj->type = COMPOUND_PROC;
    obj->data.compound_proc.parameters = parameters;
    obj->data.compound_proc.body       = body;
    obj->data.compound_proc.env        = env;
    return obj;
}

object* make_primitive_procedure(object* (* fun)(object* )) {
    object* obj = alloc_object();
    obj->type = PRIMITIVE_PROC;
    obj->data.primitive_proc.fun = fun;
    return obj;
}

static void primitive_error(const char* proc_name, const char* message) {
    char error_buf[256];
    snprintf(error_buf, sizeof(error_buf), "%s: %s", proc_name, message);
    error_handle(stderr, error_buf, EXIT_FAILURE);
}

static int argument_count(object* arguments) {
    int count = 0;
    while(!is_empty_list(arguments)) {
        count++;
        arguments = cdr(arguments);
    }
    return count;
}

static void require_exact_args(const char* proc_name, object* arguments, int expected) {
    int count = argument_count(arguments);
    if(count != expected) {
        char error_buf[128];
        snprintf(error_buf, sizeof(error_buf), "expected %d args, got %d", expected, count);
        primitive_error(proc_name, error_buf);
    }
}

static void require_min_args(const char* proc_name, object* arguments, int minimum) {
    int count = argument_count(arguments);
    if(count < minimum) {
        char error_buf[128];
        snprintf(error_buf, sizeof(error_buf), "expected at least %d args, got %d", minimum, count);
        primitive_error(proc_name, error_buf);
    }
}

static void require_fixnum_arg(const char* proc_name, object* arg, int index) {
    if(!is_fixnum(arg)) {
        char error_buf[128];
        snprintf(error_buf, sizeof(error_buf), "arg %d must be integer", index);
        primitive_error(proc_name, error_buf);
    }
}

static void require_string_arg(const char* proc_name, object* arg, int index) {
    if(!is_string(arg)) {
        char error_buf[128];
        snprintf(error_buf, sizeof(error_buf), "arg %d must be string", index);
        primitive_error(proc_name, error_buf);
    }
}

static void require_pair_arg(const char* proc_name, object* arg, int index) {
    if(!is_pair(arg)) {
        char error_buf[128];
        snprintf(error_buf, sizeof(error_buf), "arg %d must be pair", index);
        primitive_error(proc_name, error_buf);
    }
}

static object* eval_source_file(FILE* source_file, object* env) {
    char* pre_buf = read_source(source_file);
    char* buf;
    token* tokens;
    token_list* list;

    if(pre_buf == NULL)
        return ok_symbol;

    buf = buf_pre_handle(pre_buf);
    free(pre_buf);

    tokens = gen_token(buf);
    free(buf);

    list = gen_token_list(tokens);
    while(list->token_pointer != NULL) {
        eval(parse(list), env);
        gc_collect();
    }

    destroy_token_list(list);
    return ok_symbol;
}

/* implement of built-in procedures */
static object* add_procedure(object* arguments) {
    int index = 1;
    long result = 0;
    while(!is_empty_list(arguments)) {
        require_fixnum_arg("+", car(arguments), index++);
        result += (car(arguments)->data.fixnum.value);
        arguments = cdr(arguments);
    }
    return make_fixnum(result);
}

static object* sub_procedure(object* arguments) {
    int index = 2;
    require_min_args("-", arguments, 1);
    require_fixnum_arg("-", car(arguments), 1);
    long result = car(arguments)->data.fixnum.value;
    arguments = cdr(arguments);

    while(!is_empty_list(arguments)) {
        require_fixnum_arg("-", car(arguments), index++);
        result -= car(arguments)->data.fixnum.value;
        arguments = cdr(arguments);
    }
    return make_fixnum(result);
}

static object* mul_procedure(object* arguments) {
    int index = 1;
    long result = 1;

    while(!is_empty_list(arguments)) {
        require_fixnum_arg("*", car(arguments), index++);
        result *= car(arguments)->data.fixnum.value;
        arguments = cdr(arguments);
    }

    return make_fixnum(result);
}

static object* div_procedure(object* arguments) {
    long dividend;
    long divisor;

    require_exact_args("/", arguments, 2);
    require_fixnum_arg("/", car(arguments), 1);
    require_fixnum_arg("/", cadr(arguments), 2);

    dividend = car(arguments)->data.fixnum.value;
    divisor = cadr(arguments)->data.fixnum.value;
    if(divisor == 0)
        primitive_error("/", "division by zero");

    return make_fixnum(dividend / divisor);
}

static object* remainder_procedure(object* arguments) {
    long dividend;
    long divisor;

    require_exact_args("remainder", arguments, 2);
    require_fixnum_arg("remainder", car(arguments), 1);
    require_fixnum_arg("remainder", cadr(arguments), 2);

    dividend = car(arguments)->data.fixnum.value;
    divisor = cadr(arguments)->data.fixnum.value;
    if(divisor == 0)
        primitive_error("remainder", "division by zero");

    return make_fixnum(dividend % divisor);
}

static object* is_num_equal_procedure(object* arguments) {
    int index = 2;
    require_min_args("=", arguments, 2);
    require_fixnum_arg("=", car(arguments), 1);
    long value = car(arguments)->data.fixnum.value;

    while(!is_empty_list(arguments = cdr(arguments))) {
        require_fixnum_arg("=", car(arguments), index++);
        if(value != car(arguments)->data.fixnum.value)
            return false_obj;
    }
    return true_obj;
}

static object* is_less_procedure(object* arguments) {
    int index = 2;
    require_min_args("<", arguments, 2);
    require_fixnum_arg("<", car(arguments), 1);
    long previous = car(arguments)->data.fixnum.value;
    long next;

    while(!is_empty_list(arguments = cdr(arguments))) {
        require_fixnum_arg("<", car(arguments), index++);
        next = car(arguments)->data.fixnum.value;
        if(previous >= next)
            return false_obj;
        previous = next;
    }
    return true_obj;
}

static object* is_greater_procedure(object* arguments) {
    int index = 2;
    require_min_args(">", arguments, 2);
    require_fixnum_arg(">", car(arguments), 1);
    long previous = car(arguments)->data.fixnum.value;
    long next;

    while(!is_empty_list(arguments = cdr(arguments))) {
        require_fixnum_arg(">", car(arguments), index++);
        next = car(arguments)->data.fixnum.value;
        if(previous <= next)
            return false_obj;
        previous = next;
    }
    return true_obj;
}

static object* cons_procedure(object* arguments) {
    require_exact_args("cons", arguments, 2);
    return cons(car(arguments), cadr(arguments));
}

static object* car_procedure(object* arguments) {
    require_exact_args("car", arguments, 1);
    require_pair_arg("car", car(arguments), 1);
    return caar(arguments);
}

static object* cdr_procedure(object* arguments) {
    require_exact_args("cdr", arguments, 1);
    require_pair_arg("cdr", car(arguments), 1);
    return cdar(arguments);
}

static object* set_car_procedure(object* arguments) {
    require_exact_args("set-car!", arguments, 2);
    require_pair_arg("set-car!", car(arguments), 1);
    set_car(car(arguments), cadr(arguments));
    return ok_symbol;
}

static object* set_cdr_procedure(object* arguments) {
    require_exact_args("set-cdr!", arguments, 2);
    require_pair_arg("set-cdr!", car(arguments), 1);
    set_cdr(car(arguments), cadr(arguments));
    return ok_symbol;
}

static object* list_procedure(object* arguments) {
    return arguments;
}

static object* is_equal_procedure(object* arguments) {
    require_exact_args("eq?", arguments, 2);
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

static object* is_null_procedure(object* arguments) {
    require_exact_args("null?", arguments, 1);
    return is_empty_list(car(arguments)) ? true_obj : false_obj;
}

static object* is_bool_procedure(object* arguments) {
    require_exact_args("boolean?", arguments, 1);
    return is_boolean(car(arguments)) ? true_obj : false_obj;
}

static object* is_symbol_procedure(object* arguments) {
    require_exact_args("symbol?", arguments, 1);
    return is_symbol(car(arguments)) ? true_obj : false_obj;
}

static object* is_integer_procedure(object* arguments) {
    require_exact_args("integer?", arguments, 1);
    return is_fixnum(car(arguments)) ? true_obj : false_obj;
}

static object* is_string_procedure(object* arguments) {
    require_exact_args("string?", arguments, 1);
    return is_string(car(arguments)) ? true_obj : false_obj;
}

static object* is_pair_procedure(object* arguments) {
    require_exact_args("pair?", arguments, 1);
    return is_pair(car(arguments)) ? true_obj : false_obj;
}

static object* is_procedure_procedure(object* arguments) {
    require_exact_args("procedure?", arguments, 1);
    object* obj = car(arguments);
    return is_primitive_proc(obj) || is_compound_proc(obj) ? true_obj : false_obj;
}

static object* number_to_string_procedure(object* arguments) {
    char buffer[100];

    require_exact_args("number->string", arguments, 1);
    require_fixnum_arg("number->string", car(arguments), 1);
    sprintf(buffer, "%ld", car(arguments)->data.fixnum.value);
    return make_string(buffer);
}

static object* string_to_number_procedure(object* arguments) {
    require_exact_args("string->number", arguments, 1);
    require_string_arg("string->number", car(arguments), 1);
    return make_fixnum(atoi(car(arguments)->data.string.value));
}

static object* symbol_to_string_procedure(object* arguments) {
    require_exact_args("symbol->string", arguments, 1);
    if(!is_symbol(car(arguments)))
        primitive_error("symbol->string", "arg 1 must be symbol");
    return make_string(car(arguments)->data.symbol.value);
}

static object* string_to_symbol_procedure(object* arguments) {
    require_exact_args("string->symbol", arguments, 1);
    require_string_arg("string->symbol", car(arguments), 1);
    return make_symbol(car(arguments)->data.string.value);
}

static object* environment_procedure(object* arguments) {
    require_exact_args("environment", arguments, 0);
    return the_global_environment;
}

static object* load_procedure(object* arguments) {
    FILE* source_file;

    require_exact_args("load", arguments, 1);
    require_string_arg("load", car(arguments), 1);

    source_file = fopen(car(arguments)->data.string.value, "r");
    if(source_file == NULL)
        primitive_error("load", "cannot open file");

    eval_source_file(source_file, the_global_environment);
    fclose(source_file);
    return ok_symbol;
}


void add_primitive_to_environment(object* env) {
#define ADD_PRIMITIVE_PROCEDURE(scheme_name, c_name) \
    define_variable(make_symbol(scheme_name), make_primitive_procedure(c_name), env);

    ADD_PRIMITIVE_PROCEDURE("+",                           add_procedure)
    ADD_PRIMITIVE_PROCEDURE("-",                           sub_procedure)
    ADD_PRIMITIVE_PROCEDURE("*",                           mul_procedure)
    ADD_PRIMITIVE_PROCEDURE("/",                           div_procedure)
    ADD_PRIMITIVE_PROCEDURE("quotient",                    div_procedure)
    ADD_PRIMITIVE_PROCEDURE("remainder",             remainder_procedure)
    ADD_PRIMITIVE_PROCEDURE("=",                  is_num_equal_procedure)
    ADD_PRIMITIVE_PROCEDURE("<",                       is_less_procedure)
    ADD_PRIMITIVE_PROCEDURE(">",                    is_greater_procedure)
    ADD_PRIMITIVE_PROCEDURE("cons",                       cons_procedure)
    ADD_PRIMITIVE_PROCEDURE("car",                         car_procedure)
    ADD_PRIMITIVE_PROCEDURE("cdr",                         cdr_procedure)
    ADD_PRIMITIVE_PROCEDURE("set-car!",                set_car_procedure)
    ADD_PRIMITIVE_PROCEDURE("set-cdr!",                set_cdr_procedure)
    ADD_PRIMITIVE_PROCEDURE("list",                       list_procedure)
    ADD_PRIMITIVE_PROCEDURE("eq?",                    is_equal_procedure)
    ADD_PRIMITIVE_PROCEDURE("null?",                   is_null_procedure)
    ADD_PRIMITIVE_PROCEDURE("boolean?",                is_bool_procedure)
    ADD_PRIMITIVE_PROCEDURE("symbol?",               is_symbol_procedure)
    ADD_PRIMITIVE_PROCEDURE("integer?",             is_integer_procedure)
    ADD_PRIMITIVE_PROCEDURE("string?",               is_string_procedure)
    ADD_PRIMITIVE_PROCEDURE("pair?",                   is_pair_procedure)
    ADD_PRIMITIVE_PROCEDURE("procedure?",         is_procedure_procedure)
    ADD_PRIMITIVE_PROCEDURE("number->string", number_to_string_procedure)
    ADD_PRIMITIVE_PROCEDURE("string->number", string_to_number_procedure)
    ADD_PRIMITIVE_PROCEDURE("symbol->string", symbol_to_string_procedure)
    ADD_PRIMITIVE_PROCEDURE("string->symbol", string_to_symbol_procedure)
    ADD_PRIMITIVE_PROCEDURE("environment",         environment_procedure)
    ADD_PRIMITIVE_PROCEDURE("load",                     load_procedure)

}
