//
// Created by wulei on 19-3-26.
//

#include "object.h"

#ifndef SCHEME_APPLY_H
#define SCHEME_APPLY_H

extern object* apply(object* procedure, object* arguments);

extern object* procedure_parameters(object* procedure);

extern object* procedure_body(object* proceduere);

extern object* procedure_environment(object* procedure);


#endif //SCHEME_APPLY_H
