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
#include "header/apply.h"
#include "header/write.h"

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
    quasiquote_symbol = make_symbol("quasiquote");
    unquote_symbol = make_symbol("unquote");
    unquote_splicing_symbol = make_symbol("unquote-splicing");
    define_symbol = make_symbol("define");
    define_syntax_symbol = make_symbol("define-syntax");
    syntax_rules_symbol = make_symbol("syntax-rules");
    ellipsis_symbol = make_symbol("...");
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
    eof_object = make_symbol("#<eof>");

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

static void require_port_arg(const char* proc_name, object* arg, int index) {
    if(!is_port(arg)) {
        char error_buf[128];
        snprintf(error_buf, sizeof(error_buf), "arg %d must be port", index);
        primitive_error(proc_name, error_buf);
    }
}

static void require_input_port_arg(const char* proc_name, object* arg, int index) {
    require_port_arg(proc_name, arg, index);
    if(!arg->data.port.is_input || arg->data.port.file == NULL)
        primitive_error(proc_name, "input port is closed or invalid");
}

static void require_output_port_arg(const char* proc_name, object* arg, int index) {
    require_port_arg(proc_name, arg, index);
    if(!arg->data.port.is_output || arg->data.port.file == NULL)
        primitive_error(proc_name, "output port is closed or invalid");
}

static bool is_datum_equal(object* first, object* second) {
    if(first == second)
        return true;
    if(first->type != second->type)
        return false;

    switch(first->type) {
        case FIXNUM:
            return first->data.fixnum.value == second->data.fixnum.value;
        case BOOLEAN:
            return first->data.boolean.value == second->data.boolean.value;
        case CHARACTER:
            return first->data.character.value == second->data.character.value;
        case STRING:
            return strcmp(first->data.string.value, second->data.string.value) == 0;
        case SYMBOL:
            return first == second;
        case THE_EMPTY_LIST:
            return true;
        case PAIR:
            return is_datum_equal(car(first), car(second)) &&
                   is_datum_equal(cdr(first), cdr(second));
        case VECTOR: {
            object* left = first->data.vector.elements;
            object* right = second->data.vector.elements;
            if(first->data.vector.length != second->data.vector.length)
                return false;
            while(is_pair(left) && is_pair(right)) {
                if(!is_datum_equal(car(left), car(right)))
                    return false;
                left = cdr(left);
                right = cdr(right);
            }
            return is_empty_list(left) && is_empty_list(right);
        }
        default:
            return first == second;
    }
}

static object* copy_list(object* list) {
    object* head = the_empty_list;
    object* tail = the_empty_list;

    while(is_pair(list)) {
        object* cell = cons(car(list), the_empty_list);
        if(is_empty_list(head))
            head = cell;
        else
            set_cdr(tail, cell);
        tail = cell;
        list = cdr(list);
    }

    if(!is_empty_list(list))
        primitive_error("list-copy", "expected proper list");
    return head;
}

static size_t proper_list_length(object* list, const char* proc_name) {
    size_t length = 0;
    while(is_pair(list)) {
        length++;
        list = cdr(list);
    }
    if(!is_empty_list(list))
        primitive_error(proc_name, "expected proper list");
    return length;
}

static object* vector_ref_cell(object* vector, long index, const char* proc_name) {
    object* cursor;

    if(index < 0 || (size_t)index >= vector->data.vector.length)
        primitive_error(proc_name, "index out of bounds");

    cursor = vector->data.vector.elements;
    while(index-- > 0)
        cursor = cdr(cursor);
    return cursor;
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

static object* is_eqv_procedure(object* arguments) {
    require_exact_args("eqv?", arguments, 2);
    return is_datum_equal(car(arguments), cadr(arguments)) ? true_obj : false_obj;
}

static object* is_equal_deep_procedure(object* arguments) {
    require_exact_args("equal?", arguments, 2);
    return is_datum_equal(car(arguments), cadr(arguments)) ? true_obj : false_obj;
}

static object* is_char_procedure(object* arguments) {
    require_exact_args("char?", arguments, 1);
    return is_character(car(arguments)) ? true_obj : false_obj;
}

static object* char_to_integer_procedure(object* arguments) {
    require_exact_args("char->integer", arguments, 1);
    if(!is_character(car(arguments)))
        primitive_error("char->integer", "arg 1 must be character");
    return make_fixnum((unsigned char)car(arguments)->data.character.value);
}

static object* integer_to_char_procedure(object* arguments) {
    long value;
    require_exact_args("integer->char", arguments, 1);
    require_fixnum_arg("integer->char", car(arguments), 1);
    value = car(arguments)->data.fixnum.value;
    if(value < 0 || value > 255)
        primitive_error("integer->char", "codepoint out of range [0,255]");
    return make_character((char)value);
}

static object* is_vector_procedure(object* arguments) {
    require_exact_args("vector?", arguments, 1);
    return is_vector(car(arguments)) ? true_obj : false_obj;
}

static object* is_port_procedure(object* arguments) {
    require_exact_args("port?", arguments, 1);
    return is_port(car(arguments)) ? true_obj : false_obj;
}

static object* vector_procedure(object* arguments) {
    return make_vector(copy_list(arguments), proper_list_length(arguments, "vector"));
}

static object* vector_length_procedure(object* arguments) {
    require_exact_args("vector-length", arguments, 1);
    if(!is_vector(car(arguments)))
        primitive_error("vector-length", "arg 1 must be vector");
    return make_fixnum((long)car(arguments)->data.vector.length);
}

static object* vector_ref_procedure(object* arguments) {
    object* vector_obj;
    long index;

    require_exact_args("vector-ref", arguments, 2);
    vector_obj = car(arguments);
    if(!is_vector(vector_obj))
        primitive_error("vector-ref", "arg 1 must be vector");
    require_fixnum_arg("vector-ref", cadr(arguments), 2);
    index = cadr(arguments)->data.fixnum.value;
    return car(vector_ref_cell(vector_obj, index, "vector-ref"));
}

static object* vector_set_procedure(object* arguments) {
    object* vector_obj;
    long index;
    object* cell;

    require_exact_args("vector-set!", arguments, 3);
    vector_obj = car(arguments);
    if(!is_vector(vector_obj))
        primitive_error("vector-set!", "arg 1 must be vector");
    require_fixnum_arg("vector-set!", cadr(arguments), 2);
    index = cadr(arguments)->data.fixnum.value;
    cell = vector_ref_cell(vector_obj, index, "vector-set!");
    set_car(cell, caddr(arguments));
    return ok_symbol;
}

static object* vector_to_list_procedure(object* arguments) {
    require_exact_args("vector->list", arguments, 1);
    if(!is_vector(car(arguments)))
        primitive_error("vector->list", "arg 1 must be vector");
    return copy_list(car(arguments)->data.vector.elements);
}

static object* list_to_vector_procedure(object* arguments) {
    object* list_obj;
    require_exact_args("list->vector", arguments, 1);
    list_obj = car(arguments);
    if(!is_empty_list(list_obj) && !is_pair(list_obj))
        primitive_error("list->vector", "arg 1 must be list");
    return make_vector(copy_list(list_obj), proper_list_length(list_obj, "list->vector"));
}

static object* apply_procedure(object* arguments) {
    object* procedure;
    object* last;
    object* prefix = the_empty_list;
    object* prefix_tail = the_empty_list;
    object* full_args;
    object* result;

    require_min_args("apply", arguments, 2);
    procedure = car(arguments);
    arguments = cdr(arguments);

    while(is_pair(cdr(arguments))) {
        object* cell = cons(car(arguments), the_empty_list);
        if(is_empty_list(prefix))
            prefix = cell;
        else
            set_cdr(prefix_tail, cell);
        prefix_tail = cell;
        arguments = cdr(arguments);
    }

    last = car(arguments);
    if(!is_empty_list(last) && !is_pair(last))
        primitive_error("apply", "last arg must be list");

    full_args = is_empty_list(prefix) ? last : prefix;
    if(!is_empty_list(prefix))
        set_cdr(prefix_tail, last);

    result = apply(procedure, full_args);
    if(result == NULL)
        primitive_error("apply", "application failed");
    return result;
}

static object* map_procedure(object* arguments) {
    object* procedure;
    object* lists;
    object* head = the_empty_list;
    object* tail = the_empty_list;

    require_min_args("map", arguments, 2);
    procedure = car(arguments);
    lists = cdr(arguments);

    while(true) {
        object* iter = lists;
        object* call_args = the_empty_list;
        object* call_tail = the_empty_list;
        bool saw_empty = false;
        bool saw_non_empty = false;

        while(is_pair(iter)) {
            object* current_list = car(iter);
            if(is_empty_list(current_list)) {
                saw_empty = true;
            }
            else if(is_pair(current_list)) {
                object* arg_cell = cons(car(current_list), the_empty_list);
                saw_non_empty = true;
                if(is_empty_list(call_args))
                    call_args = arg_cell;
                else
                    set_cdr(call_tail, arg_cell);
                call_tail = arg_cell;
                set_car(iter, cdr(current_list));
            }
            else {
                primitive_error("map", "list args must be proper lists");
            }
            iter = cdr(iter);
        }

        if(saw_empty) {
            if(saw_non_empty)
                primitive_error("map", "list args have different lengths");
            break;
        }

        object* value = apply(procedure, call_args);
        object* cell = cons(value, the_empty_list);
        if(is_empty_list(head))
            head = cell;
        else
            set_cdr(tail, cell);
        tail = cell;
    }

    return head;
}

static object* for_each_procedure(object* arguments) {
    object* procedure;
    object* lists;

    require_min_args("for-each", arguments, 2);
    procedure = car(arguments);
    lists = cdr(arguments);

    while(true) {
        object* iter = lists;
        object* call_args = the_empty_list;
        object* call_tail = the_empty_list;
        bool saw_empty = false;
        bool saw_non_empty = false;

        while(is_pair(iter)) {
            object* current_list = car(iter);
            if(is_empty_list(current_list)) {
                saw_empty = true;
            }
            else if(is_pair(current_list)) {
                object* arg_cell = cons(car(current_list), the_empty_list);
                saw_non_empty = true;
                if(is_empty_list(call_args))
                    call_args = arg_cell;
                else
                    set_cdr(call_tail, arg_cell);
                call_tail = arg_cell;
                set_car(iter, cdr(current_list));
            }
            else {
                primitive_error("for-each", "list args must be proper lists");
            }
            iter = cdr(iter);
        }

        if(saw_empty) {
            if(saw_non_empty)
                primitive_error("for-each", "list args have different lengths");
            break;
        }

        apply(procedure, call_args);
    }

    return ok_symbol;
}

static object* call_cc_procedure(object* arguments) {
    object* procedure;
    object* continuation;

    require_exact_args("call/cc", arguments, 1);
    procedure = car(arguments);
    if(!is_primitive_proc(procedure) && !is_compound_proc(procedure))
        primitive_error("call/cc", "arg 1 must be procedure");

    continuation = make_continuation();
    continuation->data.continuation.active = true;

    if(setjmp(continuation->data.continuation.return_point) != 0) {
        continuation->data.continuation.active = false;
        return continuation->data.continuation.value;
    }

    continuation->data.continuation.value = NULL;
    continuation->data.continuation.active = true;
    arguments = apply(procedure, cons(continuation, the_empty_list));
    continuation->data.continuation.active = false;
    return arguments;
}

static void display_object(FILE* out, object* obj) {
    switch(obj->type) {
        case STRING:
            fprintf(out, "%s", obj->data.string.value);
            break;
        case CHARACTER:
            fputc(obj->data.character.value, out);
            break;
        default:
            write(out, obj);
            break;
    }
}

static object* current_input_port_procedure(object* arguments) {
    require_exact_args("current-input-port", arguments, 0);
    return make_port(stdin, true, false, false);
}

static object* current_output_port_procedure(object* arguments) {
    require_exact_args("current-output-port", arguments, 0);
    return make_port(stdout, false, true, false);
}

static object* open_input_file_procedure(object* arguments) {
    FILE* file;
    require_exact_args("open-input-file", arguments, 1);
    require_string_arg("open-input-file", car(arguments), 1);
    file = fopen(car(arguments)->data.string.value, "r");
    if(file == NULL)
        primitive_error("open-input-file", "cannot open file");
    return make_port(file, true, false, true);
}

static object* open_output_file_procedure(object* arguments) {
    FILE* file;
    require_exact_args("open-output-file", arguments, 1);
    require_string_arg("open-output-file", car(arguments), 1);
    file = fopen(car(arguments)->data.string.value, "w");
    if(file == NULL)
        primitive_error("open-output-file", "cannot open file");
    return make_port(file, false, true, true);
}

static object* close_input_port_procedure(object* arguments) {
    object* port;
    require_exact_args("close-input-port", arguments, 1);
    port = car(arguments);
    require_input_port_arg("close-input-port", port, 1);
    if(port->data.port.close_on_gc && port->data.port.file != NULL)
        fclose(port->data.port.file);
    port->data.port.file = NULL;
    return ok_symbol;
}

static object* close_output_port_procedure(object* arguments) {
    object* port;
    require_exact_args("close-output-port", arguments, 1);
    port = car(arguments);
    require_output_port_arg("close-output-port", port, 1);
    if(port->data.port.close_on_gc && port->data.port.file != NULL)
        fclose(port->data.port.file);
    port->data.port.file = NULL;
    return ok_symbol;
}

static object* read_procedure(object* arguments) {
    object* port = NULL;
    char* pre_buf;
    char* buf;
    token* tokens;
    token_list* list;
    object* result;

    if(argument_count(arguments) == 0)
        port = make_port(stdin, true, false, false);
    else {
        require_exact_args("read", arguments, 1);
        port = car(arguments);
    }

    require_input_port_arg("read", port, 1);
    pre_buf = read_source(port->data.port.file);
    if(pre_buf == NULL)
        return eof_object;

    buf = buf_pre_handle(pre_buf);
    free(pre_buf);

    tokens = gen_token(buf);
    free(buf);

    list = gen_token_list(tokens);
    if(list->token_pointer == NULL) {
        destroy_token_list(list);
        return eof_object;
    }

    result = parse(list);
    destroy_token_list(list);
    return result;
}

static object* write_procedure(object* arguments) {
    object* target_port;
    require_min_args("write", arguments, 1);
    if(argument_count(arguments) == 1)
        target_port = make_port(stdout, false, true, false);
    else {
        require_exact_args("write", arguments, 2);
        target_port = cadr(arguments);
    }
    require_output_port_arg("write", target_port, 1);
    write(target_port->data.port.file, car(arguments));
    return ok_symbol;
}

static object* display_procedure(object* arguments) {
    object* target_port;
    require_min_args("display", arguments, 1);
    if(argument_count(arguments) == 1)
        target_port = make_port(stdout, false, true, false);
    else {
        require_exact_args("display", arguments, 2);
        target_port = cadr(arguments);
    }
    require_output_port_arg("display", target_port, 1);
    display_object(target_port->data.port.file, car(arguments));
    return ok_symbol;
}

static object* newline_procedure(object* arguments) {
    object* target_port;
    if(argument_count(arguments) == 0)
        target_port = make_port(stdout, false, true, false);
    else {
        require_exact_args("newline", arguments, 1);
        target_port = car(arguments);
    }
    require_output_port_arg("newline", target_port, 1);
    fputc('\n', target_port->data.port.file);
    fflush(target_port->data.port.file);
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
    ADD_PRIMITIVE_PROCEDURE("eqv?",                     is_eqv_procedure)
    ADD_PRIMITIVE_PROCEDURE("equal?",             is_equal_deep_procedure)
    ADD_PRIMITIVE_PROCEDURE("null?",                   is_null_procedure)
    ADD_PRIMITIVE_PROCEDURE("boolean?",                is_bool_procedure)
    ADD_PRIMITIVE_PROCEDURE("symbol?",               is_symbol_procedure)
    ADD_PRIMITIVE_PROCEDURE("integer?",             is_integer_procedure)
    ADD_PRIMITIVE_PROCEDURE("number?",              is_integer_procedure)
    ADD_PRIMITIVE_PROCEDURE("string?",               is_string_procedure)
    ADD_PRIMITIVE_PROCEDURE("char?",                   is_char_procedure)
    ADD_PRIMITIVE_PROCEDURE("pair?",                   is_pair_procedure)
    ADD_PRIMITIVE_PROCEDURE("vector?",               is_vector_procedure)
    ADD_PRIMITIVE_PROCEDURE("port?",                   is_port_procedure)
    ADD_PRIMITIVE_PROCEDURE("procedure?",         is_procedure_procedure)
    ADD_PRIMITIVE_PROCEDURE("char->integer",   char_to_integer_procedure)
    ADD_PRIMITIVE_PROCEDURE("integer->char",   integer_to_char_procedure)
    ADD_PRIMITIVE_PROCEDURE("vector",                 vector_procedure)
    ADD_PRIMITIVE_PROCEDURE("vector-length",   vector_length_procedure)
    ADD_PRIMITIVE_PROCEDURE("vector-ref",         vector_ref_procedure)
    ADD_PRIMITIVE_PROCEDURE("vector-set!",       vector_set_procedure)
    ADD_PRIMITIVE_PROCEDURE("vector->list",   vector_to_list_procedure)
    ADD_PRIMITIVE_PROCEDURE("list->vector",   list_to_vector_procedure)
    ADD_PRIMITIVE_PROCEDURE("apply",                   apply_procedure)
    ADD_PRIMITIVE_PROCEDURE("map",                       map_procedure)
    ADD_PRIMITIVE_PROCEDURE("for-each",             for_each_procedure)
    ADD_PRIMITIVE_PROCEDURE("call/cc",               call_cc_procedure)
    ADD_PRIMITIVE_PROCEDURE("number->string", number_to_string_procedure)
    ADD_PRIMITIVE_PROCEDURE("string->number", string_to_number_procedure)
    ADD_PRIMITIVE_PROCEDURE("symbol->string", symbol_to_string_procedure)
    ADD_PRIMITIVE_PROCEDURE("string->symbol", string_to_symbol_procedure)
    ADD_PRIMITIVE_PROCEDURE("environment",         environment_procedure)
    ADD_PRIMITIVE_PROCEDURE("read",                     read_procedure)
    ADD_PRIMITIVE_PROCEDURE("write",                   write_procedure)
    ADD_PRIMITIVE_PROCEDURE("display",               display_procedure)
    ADD_PRIMITIVE_PROCEDURE("newline",               newline_procedure)
    ADD_PRIMITIVE_PROCEDURE("open-input-file", open_input_file_procedure)
    ADD_PRIMITIVE_PROCEDURE("open-output-file",open_output_file_procedure)
    ADD_PRIMITIVE_PROCEDURE("close-input-port",close_input_port_procedure)
    ADD_PRIMITIVE_PROCEDURE("close-output-port",close_output_port_procedure)
    ADD_PRIMITIVE_PROCEDURE("current-input-port", current_input_port_procedure)
    ADD_PRIMITIVE_PROCEDURE("current-output-port", current_output_port_procedure)
    ADD_PRIMITIVE_PROCEDURE("load",                     load_procedure)

}
