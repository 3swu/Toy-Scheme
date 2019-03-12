//
// Created by wulei on 19-3-12.
//

#include <stdio.h>
#include <stdlib.h>
#include "header/error.h"

void error_handle(FILE* out, char* msg, int exit_code) {
    fprintf(out, msg);
    exit(exit_code);
}