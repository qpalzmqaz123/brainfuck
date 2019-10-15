#include "bf.h"

typedef struct options {
    const char *cmd;
    const char *script;
    size_t dump_size;
} options_t;

void dump(const char *buff, size_t size) {
    for (int i = 0; i < size; i++) {
        if (0 == i % 10) {
            printf("\n%04d   ", (i / 10) * 10);
        }

        printf(" %03d", (uint8_t)buff[i]);
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
    int hit = 0;

    memset(options, 0, sizeof(options_t));

    int c;
    while((c = getopt(argc, argv, "d:c:h")) != -1) {
        switch (c) {
            case 'c':
                hit = 1;
                options->cmd = optarg;
                break;
            case 'd':
                hit = 1;
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
            fprintf(stderr, "Cannot open file: '%s'\n", filename);
            return 1;
        }

        fseek(fp, 0L, SEEK_END);
        size_t size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        options->script = calloc(1, size + 1);
        fread((void *)options->script, size, 1, fp);

        fclose(fp);

        hit = 1;
    }

    if (!hit) print_help_and_exit(argv[0]);

    return 0;
}

int main(int argc, char *argv[]) {
    options_t options;

    if (0 != parse_options(&options, argc, argv)) {
        return 1;
    }

    bf_ctx_t *ctx = malloc(sizeof(bf_ctx_t));

    bf_init(ctx);

    const char *script = options.cmd ? options.cmd : options.script;

#if WITH_JIT
    bf_compile(ctx, script);
    bf_run(ctx);
#else
    bf_eval(ctx, script);
#endif

    if (options.dump_size) dump(ctx->cell, options.dump_size);

    free(ctx);

    return 0;
}

