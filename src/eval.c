//
// Created by wulei on 19-3-14.
//
// Implement of evaluate module

#include "header/object.h"
#include "header/eval.h"
#include "header/environment.h"
#include "header/apply.h"
#include "header/error.h"
#include "header/read.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

object* eval(object* exp, object* env) {

    /* deal with this cases */

    if(is_self_evaluating(exp)) {
        return exp;
    }
    else if(is_variable(exp)) {
        return lookup_variable_value(exp, env);
    }
    else if(is_quoted(exp)) {
        return text_of_quotation(exp);
    }
    else if(is_assignment(exp)) {
        return eval_assignment(exp, env);
    }
    else if(is_definition(exp)) {
        return eval_definition(exp, env);
    }
    else if(is_if(exp)) {
        eval_if(exp, env);
    }
    else if(is_lambda(exp)) {
        return make_procedure(lambda_parameters(exp),
                              lambda_body(exp),
                              env);
    }
    else if(is_begin(exp)) {
        eval_sequence(begin_actions(exp), env);
    }
    else if(is_cond(exp)) {
        eval(cond_to_if(exp), env);
    }
    else if(is_application(exp)) {
        apply(eval(operator(exp), env),
                list_of_values(operands(exp), env));
    }
    else {
        error_handle_with_object(stderr,
                                 "Unknown expression type --EVAL",
                                 EXIT_FAILURE,
                                 exp);
    }
}

bool is_self_evaluating(object* exp) {
    return is_fixnum(exp) || is_string(exp) ? true : false;
}

bool is_variable       (object* exp) {
    return is_symbol(exp);
}

bool is_quoted         (object* exp) {
    return is_tagged_list(exp, quote_symbol);
}

bool is_assignment     (object* exp) {
    return is_tagged_list(exp, set_symbol);
}

bool is_definition     (object* exp) {
    return is_tagged_list(exp, define_symbol);
}

bool is_if             (object* exp) {
    return is_tagged_list(exp, if_symbol);
}

bool is_lambda         (object* exp) {
    return is_tagged_list(exp, lambda_symbol);
}

bool is_begin          (object* exp) {
    return is_tagged_list(exp, begin_symbol);
}

bool is_cond           (object* exp) {
    return is_tagged_list(exp, cond_symbol);
}

bool is_application    (object* exp) {
    return is_pair(exp);
}

bool is_tagged_list(object* exp, object* tag) {
    if(is_pair(exp)) {
        object* car_exp = car(exp);
        return is_symbol(car_exp) && car_exp == tag;
    }
    return false;
}

object* text_of_quotation(object* exp) {

}

object* eval_assignment(object* exp, object* env) {

}

object* eval_definition(object* exp, object* env) {

}

void eval_if(object* exp, object* env) {

}

object* make_procedure(object* parameters, object* body, object* env) {

}

object* lambda_parameters(object* exp) {

}

object* lambda_body(object* exp) {

}

void eval_sequence(object* exp, object* env) {

}

object* begin_actions(object* exp) {

}

object* cond_to_if(object* exp) {

}

object* operator(object* exp) {

}

object* operands(object* exp) {

}

object* list_of_values(object* exps, object* env) {

}