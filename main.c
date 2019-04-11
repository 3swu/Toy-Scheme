#include <stdio.h>
#include <memory.h>

#include "src/header/read.h"
#include "src/header/builtin.h"
#include "src/header/eval.h"
#include "src/header/environment.h"
#include "src/header/write.h"

int main() {
//    init_built_in();
//    object* obj = lookup_variable_value(make_symbol("+"), the_global_environment);
//
//    write(stdout, obj);
//    char* buf = buf_pre_handle(read(stdin));
//    printf("%s\n", buf);
//    token* t = gen_token(buf);
//    for(token* p = t; p != NULL; p = p->next)
//        printf("%s\n", p->value);

    init_built_in();

//    token_list* list = gen_token_list(t);
//    object* obj = parse(list);
    object* obj;
    init_built_in();
    printf("Welcome to Toy-Scheme\n");
    printf("Press Ctrl-C to exit\n");
    for(; ;) {
        printf("> ");
        obj = reader(stdin);
        if(obj == NULL)
            break;
        write(stdout, eval(obj, the_global_environment));
        printf("\n");
    }

    return 0;
}

