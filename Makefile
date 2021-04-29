build: release
release: main.c
	gcc main.c -o 2048 -Wall -Wextra -lcurses -lm -Ofast

debug: main.c
	gcc main.c -o /tmp/debug_output_2048 -Wall -Wextra -lcurses -lm -Og -g
	gdb /tmp/debug_output_2048
	rm -f /tmp/debug_output_2048

run: build
	./2048
