//
// Created by wulei on 19-3-14.
// the evaluate environment of Scheme

#ifndef SCHEME_ENVIRONMENT_H
#define SCHEME_ENVIRONMENT_H

#include "object.h"

extern object* lookup_variable_value(object* var, object* env);

extern object* extend_environment(object* variables, object* values, object* base_env);

extern void    set_variable_value(object* var, object* value, object* env);

extern void    define_variable(object* var, object* val, object* env);

extern object* enclosing_environment(object* env);

extern object* first_frame(object* env);

extern object* make_frame(object* variables, object* values);

extern object* frame_variables(object* frame);

extern object* frame_values(object* frame);

extern void add_binding_to_frame(object* var, object* val, object* frame);
#endif //SCHEME_ENVIRONMENT_H
