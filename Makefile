###
CFLAGS = -std=c99
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -pedantic

main: main.c
	@$(CC) $(CFLAGS) -o main.out main.c
	@./main.out

debug: debug.c
	@$(CC) $(CFLAGS) -o debug.out debug.c
	@./debug.out

.PHONY: clean
clean:
	rm -rf *.out
