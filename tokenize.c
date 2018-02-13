// tokenize.c
// Author: Nat Tuck
// 3650F2017, Challenge01 Hints

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tokenize.h"

typedef int (*char_pred)(char);

int
is_op_char(char cc)
{
    return cc == '<' || cc == '>'
        || cc == '|' || cc == '&'
        || cc == ';';
}

int
is_nop_char(char cc)
{
    return cc != 0 && !is_op_char(cc) && !isspace(cc);
}

char*
get_tok(char* text, char_pred cpred)
{
    char* tt = malloc(256);
    int ii = 0;
    for (; cpred(text[ii]); ++ii) {
        tt[ii] = text[ii];
    }
    tt[ii] = 0;
    return tt;
}

svec*
tokenize(char* text)
{
    svec* xs = make_svec();

    while (text[0]) {
        if (isspace(text[0])) {
            text++;
            continue;
        }

        char* tt;

        if (is_op_char(text[0])) {
            tt = get_tok(text, is_op_char);
        }
        else {
            tt = get_tok(text, is_nop_char);
        }

        svec_push_back(xs, tt);
        text += strlen(tt);
        free(tt);
    }
    //printsvec(xs);
    return xs;
}

int
checkop(svec* target,char* op) {
    for (int i=0; i< target->size; i++) {
        if (strcmp(target->data[i], op)==0) {
            return i;
        }
    }
    return -1;
}























