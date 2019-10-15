#include "bf.h"

void bf_init(bf_ctx_t *ctx) {
    memset(ctx, 0, sizeof(bf_ctx_t));
    ctx->cp = ctx->cell;
    ctx->lp = ctx->loops;
}

void bf_eval(bf_ctx_t *ctx, const char *src) {
    int nskip = 0; // skip loop counter

    for (const char *pc = src; 0 != *pc;) {
        switch (*pc) {
            case '+':
                if (!nskip) {
                    ++*ctx->cp;
                }

                ++pc;
                break;
            case '-':
                if (!nskip) {
                    --*ctx->cp;
                }

                ++pc;
                break;
            case '.':
                if (!nskip) {
                    putchar(*ctx->cp);
                }

                ++pc;
                break;
            case ',':
                if (!nskip) {
                    *ctx->cp = getchar();
                }

                ++pc;
                break;
            case '>':
                if (!nskip) {
                    ++ctx->cp;
                }

                ++pc;
                break;
            case '<':
                if (!nskip) {
                    --ctx->cp;
                }

                ++pc;
                break;
            case '[':
                if (0 == *ctx->cp) ++nskip;

                *ctx->lp = pc + 1;
                ++ctx->lp;

                ++pc;
                break;
            case ']':
                if (0 == *ctx->cp) {
                    if (nskip) {
                        --nskip;
                    }

                    --ctx->lp;
                    ++pc;
                } else {
                    pc = *(ctx->lp - 1);
                }

                break;
            case 0:
                return;
            default:
                ++pc;
                break;
        }
    }
}
