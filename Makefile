build: release
release: main.c
	gcc main.c -o 2048 -Wall -Wextra -lcurses -lm -Ofast

debug: main.c
	gcc main.c -o 2048_debug -Wall -Wextra -lcurses -lm -Og -g

run: build
	./2048
