CFLAGS := -Wall -Wextra -Werror -O2
LDFLAGS := -lcurses -lm

all: 2048

2048: main.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

.PHONY: all
