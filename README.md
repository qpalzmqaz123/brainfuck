# Brainfuck

A brainfuck c implementation

## Build

```bash
$ gcc -o bf main.c
```

## Usage

```bash
bf [OPTION] FILE
OPTIONS:
        -d SIZE: dump memory
        -c CMD: execute input string
        -h: print this message
```

## Hello world

```bash
$ ./bf -c '++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.'
Hello World!
```

## Dump memory

```bash
$ ./bf -c '+.' -d 20

0000    001 000 000 000 000 000 000 000 000 000
0010    000 000 000 000 000 000 000 000 000 000
```

