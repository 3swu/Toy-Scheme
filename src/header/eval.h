//
// Created by wulei on 19-3-14.
//

#ifndef SCHEME_EVAL_H
#define SCHEME_EVAL_H

#include "object.h"

extern object* eval(object* exp, object* env);

/****** predicate ******/
extern bool is_self_evaluating(object* exp);

extern bool is_variable       (object* exp);

extern bool is_quoted         (object* exp);

extern bool is_assignment     (object* exp);

extern bool is_definition     (object* exp);

extern bool is_if             (object* exp);

extern bool is_lambda         (object* exp);

extern bool is_begin          (object* exp);

extern bool is_cond           (object* exp);

extern bool is_application    (object* exp);

/****** more ******/

extern object* text_of_quotation(object* exp);

extern object* eval_assignment(object* exp, object* env);

extern object* eval_definition(object* exp, object* env);

extern void    eval_if(object* exp, object* env);

extern object* make_procedure(object* parameters, object* body, object* env);

extern object* lambda_parameters(object* exp);

extern object* lambda_body(object* exp);

#endif //SCHEME_EVAL_H
