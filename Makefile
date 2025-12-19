CC=gcc
CFLAGS=-std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -Wall -Wextra -Werror -Wno-unused-parameter -fno-asm
SRCS=src/main.c src/prompt.c src/parser.c src/hop.c src/reveal.c src/log.c src/execute.c src/jobs.c
OBJS=$(SRCS:.c=.o)

all: shell.out

shell.out: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o shell.out

clean:
	rm -f shell.out src/*.o

#llm generated