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

extern bool is_tagged_list    (object* exp, object* tag);

/****** more ******/

extern object* text_of_quotation(object* exp);

extern object* eval_assignment(object* exp, object* env);

extern object* eval_definition(object* exp, object* env);

extern void    eval_if(object* exp, object* env);

extern object* make_procedure(object* parameters, object* body, object* env);

extern object* lambda_parameters(object* exp);

extern object* lambda_body(object* exp);

extern void    eval_sequence(object* exp, object* env);

extern object* begin_actions(object* exp);

extern object* operator(object* exp);

extern object* operands(object* exp);

extern object* list_of_values(object* exps, object* env);

extern bool    is_last_exp(object* seq);

extern object* first_exp(object* seq);

extern object* rest_exp(object* seq);

extern object* assignment_varialbe(object* exp);

extern object* assignment_value(object* exp);

extern object* definition_variable(object* exp);

extern object* definition_value(object* exp);

extern object* make_lambda(object* parameters, object* body);

extern object* if_predicate(object* exp);

extern object* if_consequent(object* exp);

extern object* if_alternative(object* exp);

extern object* cond_clauses(object* exp);

extern bool    is_cond_else_clause(object* clause);

extern object* cond_predicate(object* clause);

extern object* cond_actions(object* clause);

extern object* cond_to_if(object* exp);

extern object* expand_clause(object* clauses);

#endif //SCHEME_EVAL_H
