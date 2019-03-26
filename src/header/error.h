//
// Created by wulei on 19-3-12.
// error handling

#include <bits/types/FILE.h>
#include "object.h"

#ifndef SCHEME_ERROR_H
#define SCHEME_ERROR_H

#endif //SCHEME_ERROR_H
extern void error_handle(FILE* out, char* msg, int exit_code);

extern void error_handle_with_object(FILE* out, char* msg, int exit_code, object* obj);
