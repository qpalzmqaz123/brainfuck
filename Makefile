.PHONEY: all

CFLAGS = -Iinclude -Wall -Werror

SRCS = main.c bf.c jit.c

all: bf

bf: ${SRCS}
	cc ${CFLAGS} -o $@ $^

jit: ${SRCS}
	cc ${CFLAGS} -DWITH_JIT=1 -o bf $^

clean:
	rm -rvf bf
