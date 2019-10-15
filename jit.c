#if WITH_JIT
#include "bf.h"
#include <sys/mman.h>

typedef struct loop_entry_t {
    size_t start_offset;
    size_t end_addr_value_offset;
} loop_entry_t;

#define PUSH_1_BYTE(p, value) do { \
    bf_extend_bytecode(ctx, 1); \
    *(uint8_t *)(p) = (uint8_t)(value); \
    (p) += 1; \
} while (0)

#define PUSH_2_BYTE(p, value) do { \
    bf_extend_bytecode(ctx, 2); \
    *(uint16_t *)(p) = (uint16_t)(value); \
    (p) += 2; \
} while (0)

#define PUSH_4_BYTE(p, value) do { \
    bf_extend_bytecode(ctx, 4); \
    *(uint32_t *)(p) = (uint32_t)(value); \
    (p) += 4; \
} while (0)

#define PUSH_8_BYTE(p, value) do { \
    bf_extend_bytecode(ctx, 8); \
    *(uint64_t *)(p) = (uint64_t)(value); \
    (p) += 8; \
} while (0)

void bf_extend_bytecode(bf_ctx_t *ctx, size_t size) {
    ctx->bc_size += size;
    while (ctx->bc_size > ctx->bc_buf_size) {
        uint8_t *old_bc = ctx->bc;
        size_t old_size = ctx->bc_buf_size;

        if (NULL == ctx->bc) {
            ctx->bc = mmap(NULL, JIT_DEFAULT_BC_BUF, PROT_WRITE | PROT_EXEC,  MAP_ANON | MAP_PRIVATE, -1, 0);
            ctx->bc_buf_size += JIT_DEFAULT_BC_BUF;
            ctx->current_bc = ctx->bc;
        } else {
            // create new map
            ctx->bc_buf_size *= 2;
            ctx->bc = mmap(NULL, ctx->bc_buf_size, PROT_WRITE | PROT_EXEC,  MAP_ANON | MAP_PRIVATE, -1, 0);

            // copy bytecodes
            memcpy(ctx->bc, old_bc, old_size);

            // unmap
            munmap(old_bc, old_size);

            // update current_bc
            ctx->current_bc += ctx->bc - old_bc;
        }
    }
}

void bf_compile(bf_ctx_t *ctx, const char *src) {
    loop_entry_t loop_entries[128];
    loop_entry_t *lp = loop_entries;

    for (const char *pc = src; 0 != *pc;) {
        switch (*pc) {
            case '+':
                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_1_BYTE(ctx->current_bc, 0xb8);
                    PUSH_8_BYTE(ctx->current_bc, &ctx->cp);
                }

                { // rcx = ctx->cp
                    // mov rcx, [rax]
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_2_BYTE(ctx->current_bc, 0x088b);
                }

                { // eax = *ctx->cp
                    // mov eax, [rcx]
                    PUSH_2_BYTE(ctx->current_bc, 0x018b);
                }

                { // al += 1
                    // inc al
                    PUSH_2_BYTE(ctx->current_bc, 0xc0fe);
                }

                { // *ctx->cp = eax
                    // mov [rcx], eax
                    PUSH_2_BYTE(ctx->current_bc, 0x0189);
                }

                ++pc;
                break;
            case '-':
                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_1_BYTE(ctx->current_bc, 0xb8);
                    PUSH_8_BYTE(ctx->current_bc, &ctx->cp);
                }

                { // rcx = ctx->cp
                    // mov rcx, [rax]
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_2_BYTE(ctx->current_bc, 0x088b);
                }

                { // eax = *ctx->cp
                    // mov eax, [rcx]
                    PUSH_2_BYTE(ctx->current_bc, 0x018b);
                }

                { // al -= 1
                    // dec al
                    PUSH_2_BYTE(ctx->current_bc, 0xc8fe);
                }

                { // *ctx->cp = eax
                    // mov [rcx], eax
                    PUSH_2_BYTE(ctx->current_bc, 0x0189);
                }

                ++pc;
                break;
            case '>':
                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_1_BYTE(ctx->current_bc, 0xb8);
                    PUSH_8_BYTE(ctx->current_bc, &ctx->cp);
                }

                { // mov rcx, 1
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_2_BYTE(ctx->current_bc, 0xc1c7);
                    PUSH_4_BYTE(ctx->current_bc, 0x00000001);
                }

                { // add [rax], rcx
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_2_BYTE(ctx->current_bc, 0x0801);
                }

                ++pc;
                break;
            case '<':
                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_1_BYTE(ctx->current_bc, 0xb8);
                    PUSH_8_BYTE(ctx->current_bc, &ctx->cp);
                }

                { // mov rcx, 1
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_2_BYTE(ctx->current_bc, 0xc1c7);
                    PUSH_4_BYTE(ctx->current_bc, 0x00000001);
                }

                { // sub [rax], rcx
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_2_BYTE(ctx->current_bc, 0x0829);
                }

                ++pc;
                break;
            case '[':
                lp->start_offset = ctx->current_bc - ctx->bc;

                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_1_BYTE(ctx->current_bc, 0xb8);
                    PUSH_8_BYTE(ctx->current_bc, &ctx->cp);
                }

                { // rcx = ctx->cp
                    // mov rcx, [rax]
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_2_BYTE(ctx->current_bc, 0x088b);
                }

                { // eax = *ctx->cp
                    // mov eax, [rcx]
                    PUSH_2_BYTE(ctx->current_bc, 0x018b);
                }

                { // if (*rcx->cp == 0), jump matched ']'
                    // cmp al, 0
                    PUSH_2_BYTE(ctx->current_bc, 0x003c);
                    // je matched ]
                    PUSH_2_BYTE(ctx->current_bc, 0x840f);
                    lp->end_addr_value_offset = ctx->current_bc - ctx->bc;
                    PUSH_4_BYTE(ctx->current_bc, 0);
                }

                ++lp;
                ++pc;
                break;
            case ']':
                --lp;

                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_1_BYTE(ctx->current_bc, 0xb8);
                    PUSH_8_BYTE(ctx->current_bc, &ctx->cp);
                }

                { // rcx = ctx->cp
                    // mov rcx, [rax]
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_2_BYTE(ctx->current_bc, 0x088b);
                }

                { // eax = *ctx->cp
                    // mov eax, [rcx]
                    PUSH_2_BYTE(ctx->current_bc, 0x018b);
                }

                { // if (*ctx->cp != 0), jump mached '['
                    // cmp al, 0
                    PUSH_2_BYTE(ctx->current_bc, 0x003c);
                    // jne matched [
                    PUSH_2_BYTE(ctx->current_bc, 0x850f);
                    PUSH_4_BYTE(ctx->current_bc, -(ctx->current_bc - ctx->bc - lp->start_offset + 4));
                }

                // set end_addr
                *(uint32_t *)(ctx->bc + lp->end_addr_value_offset) = (uint32_t)(ctx->current_bc - ctx->bc - lp->start_offset - (1 + 1 + 8 + 1 + 2 + 2 + 2 + 2 + 4));

                ++pc;
                break;
            case '.':
                { // rax = &ctx->cp
                    // mov rax, &ctx->cp
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_1_BYTE(ctx->current_bc, 0xb8);
                    PUSH_8_BYTE(ctx->current_bc, &ctx->cp);
                }

                { // rcx = ctx->cp
                    // mov rcx, [rax]
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_2_BYTE(ctx->current_bc, 0x088b);
                }

                { // edi = *ctx->cp
                    // mov edi, [rcx]
                    PUSH_2_BYTE(ctx->current_bc, 0x398b);
                }

                { // putchar(edi)
                    // mov rax, putchar
                    PUSH_1_BYTE(ctx->current_bc, 0x48);
                    PUSH_1_BYTE(ctx->current_bc, 0xb8);
                    PUSH_8_BYTE(ctx->current_bc, putchar);
                    // call rax
                    PUSH_2_BYTE(ctx->current_bc, 0xd0ff);
                }

                ++pc;
                break;
            default:
                ++pc;
                break;
        }
    }
 
    // ret
    PUSH_1_BYTE(ctx->current_bc, 0xc3);

    void (*func)(void) = (void (*)(void))ctx->bc;

    (void)func;
    func();
}
#endif
