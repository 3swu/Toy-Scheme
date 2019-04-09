#include <stdio.h>
#include <memory.h>

#include "src/header/read.h"
#include "src/header/builtin.h"
#include "src/header/eval.h"
#include "src/header/environment.h"
#include "src/header/write.h"

object* testfun(object* arguments){
    return alloc_object();
}

int main() {
//    init_built_in();
//    object* obj = lookup_variable_value(make_symbol("+"), the_global_environment);
//
//    write(stdout, obj);
    char* buf = buf_pre_handle(read(stdin));
    printf("%s\n", buf);
    token* t = gen_token(buf);
    for(token* p = t; p != NULL; p = p->next)
        printf("%s\n", p->value);

    init_built_in();

    token_list* list = gen_token_list(t);
    object* obj = parse(list);
    init_built_in();
    object* result = lookup_variable_value(make_symbol("+"), the_global_environment);
    write(stdout, result);
    fflush(stdout);
    object* o = eval(obj, the_global_environment);
    write(stdout, o);
    return 0;
}

