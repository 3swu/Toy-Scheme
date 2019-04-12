//
// Created by wulei on 19-4-9.
//

#include <stdio.h>
#include "header/write.h"
#include "header/object.h"

void write(FILE* out, object* obj) {
    switch(obj->type) {
        case THE_EMPTY_LIST:
            fprintf(out, "()");
            break;
        case BOOLEAN:
            fprintf(out, "#%s", is_true(obj) ? "t" : "f");
            break;
        case SYMBOL:
            fprintf(out, "%s", obj->data.symbol.value);
            break;
        case FIXNUM:
            fprintf(out, "%ld", obj->data.fixnum.value);
            break;
        case STRING:
            fprintf(out, "\"");
            fprintf(out, "%s", obj->data.string.value);
            fprintf(out, "\"");
            break;
        case PAIR:
            fprintf(out, "(");
            write_pair(out, obj);
            fprintf(out, ")");
            break;
        case PRIMITIVE_PROC:
            fprintf(out, "#<primitive-procedure>");
            break;
        case COMPOUND_PROC:
            fprintf(out, "#<compound-procedure>");
            break;
        default:
            fprintf(stderr, "unknown write type");
    }
}

 void write_pair(FILE* out, object* obj) {
    object* obj_car = car(obj);
    object* obj_cdr = cdr(obj);

    write(out, obj_car);
    if(obj_cdr->type == PAIR) {
        fprintf(out, " ");
        write_pair(out, obj_cdr);
    }
    else if(obj_cdr->type == THE_EMPTY_LIST)
        return;
    else {
        fprintf(out, " ");
        write(out, obj_cdr);
    }

}