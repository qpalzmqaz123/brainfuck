#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

#define CELL_SIZE 2048
#define LOOP_SIZE 128

typedef struct options {
    const char *cmd;
    const char *script;
    size_t dump_size;
} options_t;

typedef struct bf_ctx_t {
    char *cp; // cell pointer
    char cell[CELL_SIZE];

    const char **lp; // loop pointer
    const char *loops[LOOP_SIZE];
} bf_ctx_t;

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
                if (!nskip && *ctx->cp > 0) {
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

void bf_init(bf_ctx_t *ctx) {
    memset(ctx, 0, sizeof(bf_ctx_t));
    ctx->cp = ctx->cell;
    ctx->lp = ctx->loops;
}

void dump(const char *buff, size_t size) {
    for (int i = 0; i < size; i++) {
        if (0 == i % 10) {
            printf("\n%04d   ", (i / 10) * 10);
        }

        printf(" %03d", buff[i]);
    }

    printf("\n");
}

void print_help_and_exit(char *argv0) {
    printf("%s [OPTION] FILE\n", basename(argv0));
    printf("OPTIONS:\n");
    printf("\t-d SIZE: dump memory\n");
    printf("\t-c CMD: execute input string\n");
    printf("\t-h: print this message\n");

    exit(0);
}

int parse_options(options_t *options, int argc, char *argv[]) {
    memset(options, 0, sizeof(options_t));

    int c;
    while((c = getopt(argc, argv, "d:c:h")) != -1) {
        switch (c) {
            case 'c':
                options->cmd = optarg;
                break;
            case 'd':
                options->dump_size = atoi(optarg);
                break;
            case 'h':
            default:
                print_help_and_exit(argv[0]);
                break;
        }
    }

    if (optind < argc) {
        const char *filename = argv[optind];

        FILE *fp = fopen(filename, "r");
        if (NULL == fp) {
            fprintf(stderr, "Cannot open file: '%s'", filename);
            return 1;
        }

        fseek(fp, 0L, SEEK_END);
        size_t size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        options->script = calloc(1, size + 1);
        fread((void *)options->script, size, 1, fp);

        fclose(fp);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    options_t options;

    parse_options(&options, argc, argv);

    bf_ctx_t *ctx = malloc(sizeof(bf_ctx_t));

    bf_init(ctx);

    const char *script = options.cmd ? options.cmd : options.script;
    bf_eval(ctx, script);

    if (options.dump_size) dump(ctx->cell, options.dump_size);

    free(ctx);

    return 0;
}