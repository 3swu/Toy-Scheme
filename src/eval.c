//
// Created by wulei on 19-3-14.
// Implement of evaluate module

#include "header/object.h"
#include "header/eval.h"
#include "header/environment.h"
#include <stdbool.h>

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

    }
}

bool is_self_evaluating(object* exp) {

}

bool is_variable       (object* exp) {

}

bool is_quoted         (object* exp) {

}

bool is_assignment     (object* exp) {

}

bool is_definition     (object* exp) {

}

bool is_if             (object* exp) {

}

bool is_lambda         (object* exp) {

}

bool is_begin          (object* exp) {

}

bool is_cond           (object* exp) {

}

bool is_application    (object* exp) {

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