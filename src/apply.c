//
// Created by wulei on 19-3-26.
//
// Implement of apply module

#include <stdio.h>
#include <stdlib.h>
#include "header/object.h"
#include "header/apply.h"
#include "header/environment.h"
#include "header/eval.h"
#include "header/builtin.h"
#include "header/error.h"

object* apply(object* procedure, object* arguments) {
    if(is_primitive_proc(procedure)) {
        return (procedure->data.primitive_proc.fun)(arguments);
    }
    else if(is_compound_proc(procedure)) {
        object* environ = extend_environment(procedure->data.compound_proc.parameters,
                                             arguments,
                                             procedure->data.compound_proc.env);
        eval_sequence(make_begin(procedure->data.compound_proc.body), environ);
    }
    else {
        error_handle_with_object(stderr,
                "Unknown procedure type --APPLY",
                EXIT_FAILURE,
                procedure);
    }
}

object* procedure_parameters(object* procedure) {
    return cadr(procedure);
}

object* procedure_body(object* proceduere) {
    return caddr(proceduere);
}

object* procedure_environment(object* procedure) {
    return cadddr(procedure);
}