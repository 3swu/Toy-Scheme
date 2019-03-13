#include <stdio.h>
#include <memory.h>

#include "src/header/read.h"

int main() {
    char* buf = buf_pre_handle(read(stdin));
    printf("%s\n", buf);
    token* t = gen_token(buf);
    for(token* p = t; p != NULL; p = p->next)
        printf("%s\n", p->value);

    token_list* list = gen_token_list(t);
    object* obj = parse(list);
    return 0;
}