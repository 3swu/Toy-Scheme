//
// Created by wulei on 19-3-26.
//
// built in procedures and objects

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