#ifndef __BF_H__
#define __BF_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

#ifndef WITH_JIT
#define WITH_JIT 0
#endif

#ifndef CELL_SIZE
#define CELL_SIZE 2048
#endif

#ifndef LOOP_SIZE
#define LOOP_SIZE 128
#endif

typedef struct bf_ctx_t {
    char *cp; // cell pointer
    char cell[CELL_SIZE];

    const char **lp; // loop pointer
    const char *loops[LOOP_SIZE];
} bf_ctx_t;

void bf_init(bf_ctx_t *ctx);

void bf_eval(bf_ctx_t *ctx, const char *src);

#if WITH_JIT
void bf_compile(bf_ctx_t *ctx, const char *src);
#endif

#endif
