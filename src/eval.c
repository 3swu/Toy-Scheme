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
#include "header/builtin.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

object* eval(object* exp, object* env) {

    for(;;) {
        object* result;

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
            exp = is_true(eval(if_predicate(exp), env)) ?
                  if_consequent(exp) :
                  if_alternative(exp);
            continue;
        }
        else if(is_lambda(exp)) {
            return make_procedure(lambda_parameters(exp),
                                  lambda_body(exp),
                                  env);
        }
        else if(is_begin(exp)) {
            exp = begin_actions(exp);
            while(!is_last_exp(exp)) {
                eval(first_exp(exp), env);
                exp = rest_exp(exp);
            }
            exp = first_exp(exp);
            continue;
        }
        else if(is_cond(exp)) {
            exp = cond_to_if(exp);
            continue;
        }
        else if(is_let(exp)) {
            exp = let_to_application(exp);
            continue;
        }
        else if(is_let_star(exp)) {
            exp = let_star_to_nested_lets(exp);
            continue;
        }
        else if(is_letrec(exp)) {
            exp = letrec_to_let(exp);
            continue;
        }
        else if(is_and(exp)) {
            exp = and_tests(exp);
            if(is_empty_list(exp))
                return true_obj;
            while(!is_last_exp(exp)) {
                result = eval(first_exp(exp), env);
                if(is_false(result))
                    return false_obj;

                exp = rest_exp(exp);
            }
            exp = first_exp(exp);
            continue;
        }
        else if(is_or(exp)) {
            exp = or_tests(exp);
            if(is_empty_list(exp))
                return false_obj;
            while(!is_last_exp(exp)) {
                result = eval(first_exp(exp), env);
                if(is_true(result))
                    return result;

                exp = rest_exp(exp);
            }
            exp = first_exp(exp);
            continue;
        }
        else if(is_application(exp)) {
            object* procedure = eval(operator(exp), env);
            object* arguments = list_of_values(operands(exp), env);

            if(is_primitive_proc(procedure)) {
                return (procedure->data.primitive_proc.fun)(arguments);
            }
            else if(is_compound_proc(procedure)) {
                env = extend_environment(procedure->data.compound_proc.parameters,
                                         arguments,
                                         procedure->data.compound_proc.env);
                exp = make_begin(procedure->data.compound_proc.body);
                continue;
            }
            else {
                error_handle_with_object(stderr,
                                         "Unknown procedure type --EVAL",
                                         EXIT_FAILURE,
                                         procedure);
            }
        }
        else {
            error_handle_with_object(stderr,
                                     "Unknown expression type --EVAL",
                                     EXIT_FAILURE,
                                     exp);
        }
    }
}

bool is_self_evaluating(object* exp) {
    return is_fixnum(exp) || is_string(exp) || is_boolean(exp) ? true : false;
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

bool is_let            (object* exp) {
    return is_tagged_list(exp, let_symbol);
}

bool is_let_star       (object* exp) {
    return is_tagged_list(exp, let_star_symbol);
}

bool is_letrec         (object* exp) {
    return is_tagged_list(exp, letrec_symbol);
}

bool is_and            (object* exp) {
    return is_tagged_list(exp, and_symbol);
}

bool is_or             (object* exp) {
    return is_tagged_list(exp, or_symbol);
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
    return cadr(exp);
}

object* eval_assignment(object* exp, object* env) {
    set_variable_value(assignment_varialbe(exp),
                       eval(assignment_value(exp), env),
                       env);
    return ok_symbol;
}

object* eval_definition(object* exp, object* env) {
    define_variable(definition_variable(exp),
                    eval(definition_value(exp), env),
                    env);
    return ok_symbol;
}

object* eval_if(object* exp, object* env) {
    if(is_true(eval(if_predicate(exp), env)))
        return eval(if_consequent(exp), env);
    else
        return eval(if_alternative(exp), env);
}

object* make_procedure(object* parameters, object* body, object* env) {
    return make_compound_procedure(parameters, body, env);
}

object* lambda_parameters(object* exp) {
    return cadr(exp);
}

object* lambda_body(object* exp) {
    return cddr(exp);
}

object* eval_sequence(object* exp, object* env) {
    while(!is_last_exp(exp)) {
        eval(first_exp(exp), env);
        exp = rest_exp(exp);
    }
    return eval(first_exp(exp), env);
}

object* begin_actions(object* exp) {
    return cdr(exp);
}


object* operator(object* exp) {
    return car(exp);
}

object* operands(object* exp) {
    return cdr(exp);
}

object* list_of_values(object* exps, object* env) {
    if(is_no_operands(exps))
        return the_empty_list;
    return cons(eval(first_operand(exps), env),
            list_of_values(rest_operands(exps), env));
}

bool is_last_exp(object* seq) {
    return is_empty_list(cdr(seq));
}

object* first_exp(object* seq) {
    return car(seq);
}

object* rest_exp(object* seq) {
    return cdr(seq);
}

object* assignment_varialbe(object* exp) {
    return cadr(exp);
}

object* assignment_value(object* exp) {
    return caddr(exp);
}

object* definition_variable(object* exp) {
    if(is_symbol(cadr(exp)))
        return cadr(exp);
    else
        return caadr(exp);
}

object* definition_value(object* exp) {
    if(is_symbol(cadr(exp)))
        return caddr(exp);
    else
        return make_lambda(cdadr(exp), cddr(exp));
}

object* make_lambda(object* parameters, object* body) {
    return cons(lambda_symbol, cons(parameters, body));
}

object* if_predicate(object* exp) {
    return cadr(exp);
}

object* if_consequent(object* exp) {
    return caddr(exp);
}

object* if_alternative(object* exp) {
    if(is_empty_list(cdddr(exp)))
        return false_obj;
    else
        return cadddr(exp);
}

object* cond_clauses(object* exp) {
    return cdr(exp);
}

bool    is_cond_else_clause(object* clause) {
    return cond_predicate(clause) == else_symbol ? true : false;
}

object* cond_predicate(object* clause) {
    return car(clause);
}

object* cond_actions(object* clause) {
    return cdr(clause);
}

object* cond_to_if(object* exp) {
    return expand_clause(cond_clauses(exp));
}

object* expand_clause(object* clauses) {
    object* first;
    object* rest;
    if(is_empty_list(clauses))
        return false_obj;
    else {
        first = car(clauses);
        rest = cdr(clauses);

        if(is_cond_else_clause(first)) {
            if(is_empty_list(rest))
                return sequence_to_exp(cond_actions(first));
            else
                error_handle_with_object(stderr,
                        "ELSE clause isn't last --COND->IF",
                        EXIT_FAILURE,
                        clauses);
        }
        else {
            return make_if(cond_predicate(first),
                           sequence_to_exp(cond_actions(first)),
                           expand_clause(rest));
        }
    }
    return NULL;
}

object* sequence_to_exp(object* seq) {
    if(is_empty_list(seq))
        return seq;
    else if(is_last_exp(seq))
        return first_exp(seq);
    else
        return make_begin(seq);
}

object* make_if(object* predicate, object* consequent, object* alternative) {
    return cons(if_symbol,
            cons(predicate,
                    cons(consequent,
                            cons(alternative, the_empty_list))));
}

object* make_begin(object* seq) {
    return cons(begin_symbol, seq);
}

bool    is_no_operands(object* ops) {
    return is_empty_list(ops);
}

object* first_operand(object* ops) {
    return car(ops);
}

object* rest_operands(object* ops) {
    return cdr(ops);
}

object* let_to_application(object* exp) {
    return make_application(make_lambda(let_parameters(exp), let_body(exp)), let_arguments(exp));
}

object* let_parameters(object* exp) {
    return binding_parameters(let_bindings(exp));
}

object* let_body(object* exp) {
    return cddr(exp);
}

object* let_arguments(object* exp) {
    return binding_arguments(let_bindings(exp));
}

object* make_application(object* operator, object* operands) {
    return cons(operator, operands);
}

object* binding_parameters(object* bindings) {
    return is_empty_list(bindings) ?
        the_empty_list :
        cons(binding_parameter(car(bindings)), binding_parameters(cdr(bindings)));
}

object* binding_arguments(object* bindings) {
    return is_empty_list(bindings) ?
        the_empty_list :
        cons(binding_argument(car(bindings)), binding_arguments(cdr(bindings)));

}

object* binding_parameter(object* binding) {
    return car(binding);
}

object* binding_argument(object* binding) {
    return cadr(binding);
}

object* let_bindings(object* exp) {
    return cadr(exp);
}

static object* append_lists(object* first, object* second) {
    object* head = first;

    if(is_empty_list(first))
        return second;

    while(!is_empty_list(cdr(first)))
        first = cdr(first);
    set_cdr(first, second);
    return head;
}

static object* letrec_bindings_to_let_bindings(object* bindings) {
    if(is_empty_list(bindings))
        return the_empty_list;

    return cons(
            cons(binding_parameter(car(bindings)),
                 cons(cons(quote_symbol,
                           cons(unassigned_symbol, the_empty_list)),
                      the_empty_list)),
            letrec_bindings_to_let_bindings(cdr(bindings)));
}

static object* letrec_bindings_to_sets(object* bindings) {
    if(is_empty_list(bindings))
        return the_empty_list;

    return cons(
            cons(set_symbol,
                 cons(binding_parameter(car(bindings)),
                      cons(binding_argument(car(bindings)),
                           the_empty_list))),
            letrec_bindings_to_sets(cdr(bindings)));
}

object* let_star_to_nested_lets(object* exp) {
    object* bindings = let_bindings(exp);
    object* body = let_body(exp);

    if(is_empty_list(bindings))
        return cons(let_symbol, cons(the_empty_list, body));

    if(is_empty_list(cdr(bindings)))
        return cons(let_symbol, cons(bindings, body));

    return cons(let_symbol,
                cons(cons(car(bindings), the_empty_list),
                     cons(let_star_to_nested_lets(
                             cons(let_star_symbol,
                                  cons(cdr(bindings), body))),
                          the_empty_list)));
}

object* letrec_to_let(object* exp) {
    object* bindings = let_bindings(exp);
    object* body = let_body(exp);
    object* let_bindings_seq = letrec_bindings_to_let_bindings(bindings);
    object* set_seq = letrec_bindings_to_sets(bindings);

    return cons(let_symbol,
                cons(let_bindings_seq,
                     append_lists(set_seq, body)));
}

object* and_tests(object* exp) {
    return cdr(exp);
}

object* or_tests(object* exp) {
    return cdr(exp);
}
