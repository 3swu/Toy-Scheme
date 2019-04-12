#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#include "src/header/read.h"
#include "src/header/builtin.h"
#include "src/header/eval.h"
#include "src/header/environment.h"
#include "src/header/write.h"
#include "src/header/error.h"

int main() {

    object* obj, * result;
    init_built_in();
    printf("Welcome to Toy-Scheme\n");
    printf("Press Ctrl-C to exit\n");
    for(; ;) {
        printf("> ");
        obj = reader(stdin);
        if(obj == NULL){
            break;
        }
        result = eval(obj, the_global_environment);
        if(result == NULL) {
            error_handle(stderr, "eval return a null object", EXIT_FAILURE);
            break;
        }
        else {
            write(stdout, result);
            printf("\n");
        }
    }

    return 0;
}

