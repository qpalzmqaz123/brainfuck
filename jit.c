#if WITH_JIT
#include "bf.h"
#include <sys/mman.h>

typedef struct loop_entry_t {
    uint8_t *start_addr;
    uint32_t *end_addr_value;
} loop_entry_t;

#define PUSH_1_BYTE(p, value) do { \
    *(uint8_t *)p = (uint8_t)(value); \
    p += 1; \
} while (0)

#define PUSH_2_BYTE(p, value) do { \
    *(uint16_t *)p = (uint16_t)(value); \
    p += 2; \
} while (0)

#define PUSH_4_BYTE(p, value) do { \
    *(uint32_t *)p = (uint32_t)(value); \
    p += 4; \
} while (0)

#define PUSH_8_BYTE(p, value) do { \
    *(uint64_t *)p = (uint64_t)(value); \
    p += 8; \
} while (0)

void bf_compile(bf_ctx_t *ctx, const char *src) {
    uint8_t *buf = mmap(NULL, 409600, PROT_WRITE | PROT_EXEC,  MAP_ANON | MAP_PRIVATE, -1, 0);
    uint8_t *p = buf;

    loop_entry_t loop_entries[128];
    loop_entry_t *lp = loop_entries;

    for (const char *pc = src; 0 != *pc;) {
        switch (*pc) {
            case '+':
                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_1_BYTE(p, 0xb8);
                    PUSH_8_BYTE(p, &ctx->cp);
                }

                { // rcx = ctx->cp
                    // mov rcx, [rax]
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_2_BYTE(p, 0x088b);
                }

                { // eax = *ctx->cp
                    // mov eax, [rcx]
                    PUSH_2_BYTE(p, 0x018b);
                }

                { // al += 1
                    // inc al
                    PUSH_2_BYTE(p, 0xc0fe);
                }

                { // *ctx->cp = eax
                    // mov [rcx], eax
                    PUSH_2_BYTE(p, 0x0189);
                }

                ++pc;
                break;
            case '-':
                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_1_BYTE(p, 0xb8);
                    PUSH_8_BYTE(p, &ctx->cp);
                }

                { // rcx = ctx->cp
                    // mov rcx, [rax]
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_2_BYTE(p, 0x088b);
                }

                { // eax = *ctx->cp
                    // mov eax, [rcx]
                    PUSH_2_BYTE(p, 0x018b);
                }

                { // al -= 1
                    // dec al
                    PUSH_2_BYTE(p, 0xc8fe);
                }

                { // *ctx->cp = eax
                    // mov [rcx], eax
                    PUSH_2_BYTE(p, 0x0189);
                }

                ++pc;
                break;
            case '>':
                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_1_BYTE(p, 0xb8);
                    PUSH_8_BYTE(p, &ctx->cp);
                }

                { // mov rcx, 1
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_2_BYTE(p, 0xc1c7);
                    PUSH_4_BYTE(p, 0x00000001);
                }

                { // add [rax], rcx
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_2_BYTE(p, 0x0801);
                }

                ++pc;
                break;
            case '<':
                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_1_BYTE(p, 0xb8);
                    PUSH_8_BYTE(p, &ctx->cp);
                }

                { // mov rcx, 1
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_2_BYTE(p, 0xc1c7);
                    PUSH_4_BYTE(p, 0x00000001);
                }

                { // sub [rax], rcx
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_2_BYTE(p, 0x0829);
                }

                ++pc;
                break;
            case '[':
                lp->start_addr = p;

                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_1_BYTE(p, 0xb8);
                    PUSH_8_BYTE(p, &ctx->cp);
                }

                { // rcx = ctx->cp
                    // mov rcx, [rax]
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_2_BYTE(p, 0x088b);
                }

                { // eax = *ctx->cp
                    // mov eax, [rcx]
                    PUSH_2_BYTE(p, 0x018b);
                }

                { // if (*rcx->cp == 0), jump matched ']'
                    // cmp al, 0
                    PUSH_2_BYTE(p, 0x003c);
                    // je matched ]
                    PUSH_2_BYTE(p, 0x840f);
                    lp->end_addr_value = (void *)p;
                    PUSH_4_BYTE(p, 0);
                }

                ++lp;
                ++pc;
                break;
            case ']':
                --lp;

                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_1_BYTE(p, 0xb8);
                    PUSH_8_BYTE(p, &ctx->cp);
                }

                { // rcx = ctx->cp
                    // mov rcx, [rax]
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_2_BYTE(p, 0x088b);
                }

                { // eax = *ctx->cp
                    // mov eax, [rcx]
                    PUSH_2_BYTE(p, 0x018b);
                }

                { // if (*ctx->cp != 0), jump mached '['
                    // cmp al, 0
                    PUSH_2_BYTE(p, 0x003c);
                    // jne matched [
                    PUSH_2_BYTE(p, 0x850f);
                    PUSH_4_BYTE(p, -(p - lp->start_addr + 4));
                }

                // set end_addr
                *lp->end_addr_value = (uint32_t)(p - lp->start_addr - (1 + 1 + 8 + 1 + 2 + 2 + 2 + 2 + 4));

                ++pc;
                break;
            case '.':
                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_1_BYTE(p, 0xb8);
                    PUSH_8_BYTE(p, &ctx->cp);
                }

                { // rcx = ctx->cp
                    // mov rcx, [rax]
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_2_BYTE(p, 0x088b);
                }

                { // edi = *ctx->cp
                    // mov edi, [rcx]
                    PUSH_2_BYTE(p, 0x398b);
                }

                { // putchar(edi)
                    // mov rax, putchar
                    PUSH_1_BYTE(p, 0x48);
                    PUSH_1_BYTE(p, 0xb8);
                    PUSH_8_BYTE(p, putchar);
                    // call rax
                    PUSH_2_BYTE(p, 0xd0ff);
                }

                ++pc;
                break;
            default:
                ++pc;
                break;
        }
    }
 
    // ret
    *p++ = 0xc3;

    void (*func)(void) = (void (*)(void))buf;

    func();
}
#endif
