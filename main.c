#include <stdio.h>

#include "src/header/read.h"

int main() {
    printf("%s\n", buf_pre_handle(read(stdin)));
    return 0;
}