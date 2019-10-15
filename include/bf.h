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

#ifndef JIT_DEFAULT_BC_BUF
#define JIT_DEFAULT_BC_BUF 1
#endif

typedef struct bf_ctx_t {
    char *cp; // cell pointer
    char cell[CELL_SIZE];

    const char **lp; // loop pointer
    const char *loops[LOOP_SIZE];

#if WITH_JIT
    uint8_t *bc; // bytecode
    uint8_t *current_bc;
    size_t bc_size;
    size_t bc_buf_size;
#endif
} bf_ctx_t;

void bf_init(bf_ctx_t *ctx);

void bf_eval(bf_ctx_t *ctx, const char *src);

#if WITH_JIT
void bf_compile(bf_ctx_t *ctx, const char *src);
#endif

#endif
