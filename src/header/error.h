//
// Created by wulei on 19-3-12.
// error handling

#ifndef SCHEME_ERROR_H
#define SCHEME_ERROR_H

#include <stdio.h>
#include <setjmp.h>
#include "object.h"

extern void error_handle(FILE* out, char* msg, int exit_code);

extern void error_handle_with_object(FILE* out, char* msg, int exit_code, object* obj);

extern void set_error_recovery(jmp_buf* recovery_point);

extern void clear_error_recovery(void);

#endif //SCHEME_ERROR_H
