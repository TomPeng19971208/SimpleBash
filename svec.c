// svec.c
// Author: Nat Tuck
// 3650F2017, Challenge01 Hints

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "svec.h"

svec*
make_svec()
{
    svec* sv = malloc(sizeof(svec));
    sv->size = 0;
    sv->cap  = 4;
    sv->data = malloc(4 * sizeof(char*));
    memset(sv->data, 0, 4 * sizeof(char*));
    return sv;
}

void
free_svec(svec* sv)
{
    for (int ii = 0; ii < sv->size; ++ii) {
        if (sv->data[ii] != 0) {
            free(sv->data[ii]);
        }
    }
    free(sv->data);
    free(sv);
}

char*
svec_get(svec* sv, int ii)
{
    assert(ii >= 0 && ii < sv->size);
    return sv->data[ii];
}

void
svec_put(svec* sv, int ii, char* item)
{
    assert(ii >= 0 && ii < sv->size);
    sv->data[ii] = strdup(item);

}

void svec_push_back(svec* sv, char* item)
{
    int ii = sv->size;

    if (ii >= sv->cap) {
        sv->cap *= 2;
        sv->data = (char**) realloc(sv->data, sv->cap * sizeof(char*));
    }

    sv->size = ii + 1;
    svec_put(sv, ii, item);
}

void printsvec(svec* target) {
    for (int i=0; i<target->size;i++) {
        printf("%s\n", target->data[i]);
    }
}

char* extract(svec* target, int from, int to) {
    char *result = malloc(256);
    for (int i=from; i<to;i++) {
        strcat(result, target->data[i]);
        strcat(result, " ");
    }
    result[strlen(result)-1] = '\0';
    return result;
}



























