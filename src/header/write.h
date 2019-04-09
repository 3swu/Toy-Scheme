//
// Created by wulei on 19-4-9.
//

#include <bits/types/FILE.h>
#include "object.h"

#ifndef SCHEME_WRITE_H
#define SCHEME_WRITE_H

extern void write(FILE* out, object* obj);

void write_pair(FILE*, object*);
#endif //SCHEME_WRITE_H
