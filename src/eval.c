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
#include <string.h>

static bool is_quasiquote(object* exp);
static bool is_define_syntax(object* exp);
static object* eval_define_syntax(object* exp, object* env);
static object* eval_quasiquote(object* exp, object* env, int depth);
static object* eval_quasiquote_list(object* exp, object* env, int depth);
static void append_cell(object** head, object** tail, object* value);
static void append_list_cells(object** head, object** tail, object* values);
static int list_length(object* list);
static bool datum_equal(object* first, object* second);
static bool symbol_in_list(object* symbol, object* list);
static object* lookup_binding(object* bindings, object* variable);
static bool bind_variable(object** bindings, object* variable, object* value);
static bool match_pattern(object* pattern, object* input, object* literals, object** bindings);
static object* expand_template(object* template_exp, object* bindings);
static object* expand_template_list(object* template_exp, object* bindings);
static object* expand_macro_application(object* macro, object* form);
static bool is_macro_application(object* exp, object* env, object** expanded);

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
        else if(is_quasiquote(exp)) {
            return eval_quasiquote(cadr(exp), env, 1);
        }
        else if(is_assignment(exp)) {
            return eval_assignment(exp, env);
        }
        else if(is_definition(exp)) {
            return eval_definition(exp, env);
        }
        else if(is_define_syntax(exp)) {
            return eval_define_syntax(exp, env);
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
            object* expanded_exp = NULL;
            object* procedure = eval(operator(exp), env);
            object* arguments;

            if(is_macro_application(exp, env, &expanded_exp)) {
                exp = expanded_exp;
                continue;
            }

            arguments = list_of_values(operands(exp), env);

            if(is_primitive_proc(procedure)) {
                return (procedure->data.primitive_proc.fun)(arguments);
            }
            else if(is_continuation(procedure)) {
                if(list_length(arguments) != 1)
                    error_handle(stderr, "continuation expected exactly 1 value", EXIT_FAILURE);
                if(!procedure->data.continuation.active)
                    error_handle(stderr, "inactive continuation", EXIT_FAILURE);
                procedure->data.continuation.value = car(arguments);
                longjmp(procedure->data.continuation.return_point, 1);
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
    return is_fixnum(exp) ||
           is_string(exp) ||
           is_boolean(exp) ||
           is_character(exp) ||
           is_vector(exp) ? true : false;
}

bool is_variable       (object* exp) {
    return is_symbol(exp);
}

bool is_quoted         (object* exp) {
    return is_tagged_list(exp, quote_symbol);
}

static bool is_quasiquote(object* exp) {
    return is_tagged_list(exp, quasiquote_symbol);
}

bool is_assignment     (object* exp) {
    return is_tagged_list(exp, set_symbol);
}

bool is_definition     (object* exp) {
    return is_tagged_list(exp, define_symbol);
}

static bool is_define_syntax(object* exp) {
    return is_tagged_list(exp, define_syntax_symbol);
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

static object* eval_define_syntax(object* exp, object* env) {
    object* name = cadr(exp);
    object* transformer = caddr(exp);
    object* literals;
    object* rules;

    if(!is_symbol(name))
        error_handle(stderr, "define-syntax requires a symbol name", EXIT_FAILURE);
    if(!is_tagged_list(transformer, syntax_rules_symbol))
        error_handle(stderr, "define-syntax requires syntax-rules", EXIT_FAILURE);

    literals = cadr(transformer);
    rules = cddr(transformer);
    define_variable(name, make_macro(literals, rules, env), env);
    return ok_symbol;
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

static void append_cell(object** head, object** tail, object* value) {
    object* cell = cons(value, the_empty_list);
    if(is_empty_list(*head))
        *head = cell;
    else
        set_cdr(*tail, cell);
    *tail = cell;
}

static void append_list_cells(object** head, object** tail, object* values) {
    while(is_pair(values)) {
        append_cell(head, tail, car(values));
        values = cdr(values);
    }
    if(!is_empty_list(values))
        error_handle(stderr, "unquote-splicing requires a proper list", EXIT_FAILURE);
}

static object* eval_quasiquote_list(object* exp, object* env, int depth) {
    object* head = the_empty_list;
    object* tail = the_empty_list;
    object* current = exp;

    while(is_pair(current)) {
        object* item = car(current);
        if(depth == 1 &&
           is_pair(item) &&
           is_tagged_list(item, unquote_splicing_symbol))
            append_list_cells(&head, &tail, eval(cadr(item), env));
        else
            append_cell(&head, &tail, eval_quasiquote(item, env, depth));
        current = cdr(current);
    }

    if(is_empty_list(head))
        return is_empty_list(current) ? the_empty_list : eval_quasiquote(current, env, depth);

    if(is_empty_list(current))
        set_cdr(tail, the_empty_list);
    else
        set_cdr(tail, eval_quasiquote(current, env, depth));
    return head;
}

static object* eval_quasiquote(object* exp, object* env, int depth) {
    if(is_pair(exp)) {
        if(is_tagged_list(exp, unquote_symbol)) {
            if(depth == 1)
                return eval(cadr(exp), env);
            return cons(unquote_symbol,
                        cons(eval_quasiquote(cadr(exp), env, depth - 1),
                             the_empty_list));
        }
        if(is_tagged_list(exp, unquote_splicing_symbol)) {
            if(depth == 1)
                error_handle(stderr, "unquote-splicing cannot appear here", EXIT_FAILURE);
            return cons(unquote_splicing_symbol,
                        cons(eval_quasiquote(cadr(exp), env, depth - 1),
                             the_empty_list));
        }
        if(is_tagged_list(exp, quasiquote_symbol))
            return cons(quasiquote_symbol,
                        cons(eval_quasiquote(cadr(exp), env, depth + 1),
                             the_empty_list));
        return eval_quasiquote_list(exp, env, depth);
    }
    if(is_vector(exp)) {
        object* elements = eval_quasiquote_list(exp->data.vector.elements, env, depth);
        size_t length = 0;
        object* cursor = elements;
        while(is_pair(cursor)) {
            length++;
            cursor = cdr(cursor);
        }
        if(!is_empty_list(cursor))
            error_handle(stderr, "quasiquote vector must remain proper", EXIT_FAILURE);
        return make_vector(elements, length);
    }
    return exp;
}

static int list_length(object* list) {
    int count = 0;
    while(is_pair(list)) {
        count++;
        list = cdr(list);
    }
    if(!is_empty_list(list))
        error_handle(stderr, "expected proper list", EXIT_FAILURE);
    return count;
}

static bool datum_equal(object* first, object* second) {
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
        case THE_EMPTY_LIST:
            return true;
        case SYMBOL:
            return first == second;
        case PAIR:
            return datum_equal(car(first), car(second)) &&
                   datum_equal(cdr(first), cdr(second));
        case VECTOR: {
            object* a = first->data.vector.elements;
            object* b = second->data.vector.elements;
            if(first->data.vector.length != second->data.vector.length)
                return false;
            while(is_pair(a) && is_pair(b)) {
                if(!datum_equal(car(a), car(b)))
                    return false;
                a = cdr(a);
                b = cdr(b);
            }
            return is_empty_list(a) && is_empty_list(b);
        }
        default:
            return first == second;
    }
}

static bool symbol_in_list(object* symbol, object* list) {
    while(is_pair(list)) {
        if(car(list) == symbol)
            return true;
        list = cdr(list);
    }
    return false;
}

static object* lookup_binding(object* bindings, object* variable) {
    while(is_pair(bindings)) {
        object* binding = car(bindings);
        if(car(binding) == variable)
            return binding;
        bindings = cdr(bindings);
    }
    return NULL;
}

static bool bind_variable(object** bindings, object* variable, object* value) {
    object* exists = lookup_binding(*bindings, variable);
    if(exists == NULL) {
        *bindings = cons(cons(variable, value), *bindings);
        return true;
    }
    return datum_equal(cdr(exists), value);
}

static bool match_pattern(object* pattern, object* input, object* literals, object** bindings) {
    if(is_symbol(pattern)) {
        object* wildcard = make_symbol("_");
        if(pattern == wildcard)
            return true;
        if(symbol_in_list(pattern, literals))
            return is_symbol(input) && input == pattern;
        return bind_variable(bindings, pattern, input);
    }

    if(is_pair(pattern)) {
        while(is_pair(pattern)) {
            object* pat_item = car(pattern);
            object* pat_rest = cdr(pattern);

            if(is_pair(pat_rest) && car(pat_rest) == ellipsis_symbol) {
                object* after = cdr(pat_rest);
                if(!is_empty_list(after))
                    error_handle(stderr, "complex ellipsis pattern is not supported", EXIT_FAILURE);
                if(is_symbol(pat_item)) {
                    object* wildcard = make_symbol("_");
                    if(pat_item == wildcard)
                        return is_empty_list(input) || is_pair(input);
                    if(symbol_in_list(pat_item, literals))
                        return false;
                    return bind_variable(bindings, pat_item, input);
                }
                while(is_pair(input)) {
                    if(!match_pattern(pat_item, car(input), literals, bindings))
                        return false;
                    input = cdr(input);
                }
                return is_empty_list(input);
            }

            if(!is_pair(input))
                return false;
            if(!match_pattern(pat_item, car(input), literals, bindings))
                return false;
            pattern = pat_rest;
            input = cdr(input);
        }
        return match_pattern(pattern, input, literals, bindings);
    }

    return datum_equal(pattern, input);
}

static object* expand_template_list(object* template_exp, object* bindings) {
    object* head = the_empty_list;
    object* tail = the_empty_list;
    object* current = template_exp;

    while(is_pair(current)) {
        object* item = car(current);
        object* rest = cdr(current);

        if(is_pair(rest) && car(rest) == ellipsis_symbol) {
            object* after = cdr(rest);
            if(!is_empty_list(after))
                error_handle(stderr, "complex ellipsis template is not supported", EXIT_FAILURE);
            if(!is_symbol(item))
                error_handle(stderr, "ellipsis template item must be a symbol", EXIT_FAILURE);

            object* binding = lookup_binding(bindings, item);
            object* values = binding == NULL ? the_empty_list : cdr(binding);
            append_list_cells(&head, &tail, values);
            current = after;
            continue;
        }

        append_cell(&head, &tail, expand_template(item, bindings));
        current = rest;
    }

    if(is_empty_list(head))
        return is_empty_list(current) ? the_empty_list : expand_template(current, bindings);

    if(is_empty_list(current))
        set_cdr(tail, the_empty_list);
    else
        set_cdr(tail, expand_template(current, bindings));
    return head;
}

static object* expand_template(object* template_exp, object* bindings) {
    object* binding;

    if(is_symbol(template_exp)) {
        binding = lookup_binding(bindings, template_exp);
        return binding == NULL ? template_exp : cdr(binding);
    }
    if(is_pair(template_exp))
        return expand_template_list(template_exp, bindings);
    if(is_vector(template_exp)) {
        object* expanded = expand_template_list(template_exp->data.vector.elements, bindings);
        size_t length = 0;
        object* cursor = expanded;
        while(is_pair(cursor)) {
            length++;
            cursor = cdr(cursor);
        }
        if(!is_empty_list(cursor))
            error_handle(stderr, "vector template expansion must be proper list", EXIT_FAILURE);
        return make_vector(expanded, length);
    }
    return template_exp;
}

static object* expand_macro_application(object* macro, object* form) {
    object* rules = macro->data.macro.rules;
    object* keyword = car(form);
    object* macro_literals = cons(keyword, macro->data.macro.literals);

    while(is_pair(rules)) {
        object* rule = car(rules);
        object* pattern;
        object* template_exp;
        object* bindings = the_empty_list;

        if(!is_pair(rule) || is_empty_list(cdr(rule)))
            error_handle(stderr, "invalid syntax-rules rule", EXIT_FAILURE);
        pattern = car(rule);
        template_exp = cadr(rule);
        if(match_pattern(pattern, form, macro_literals, &bindings))
            return expand_template(template_exp, bindings);
        rules = cdr(rules);
    }

    error_handle(stderr, "macro pattern did not match", EXIT_FAILURE);
    return NULL;
}

static bool is_macro_application(object* exp, object* env, object** expanded) {
    object* op;
    object* value;

    if(!is_pair(exp))
        return false;
    op = car(exp);
    if(!is_symbol(op))
        return false;

    value = lookup_variable_value(op, env);
    if(!is_macro(value))
        return false;

    *expanded = expand_macro_application(value, exp);
    return true;
}
