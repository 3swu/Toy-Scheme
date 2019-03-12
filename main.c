#include <stdio.h>
#include <memory.h>

#include "src/header/read.h"

int main() {
    char* buf = buf_pre_handle(read(stdin));
    printf("%s\n", buf);
    for(token* t = gen_token(buf); t != NULL; t = t->next)
        printf("%s\n", t->value);

    return 0;
}