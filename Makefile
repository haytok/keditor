###
CFLAGS = -std=c99
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -pedantic

main: main.c
	@echo Compiling ...
	@$(CC) $(CFLAGS) -o main.out main.c
	@./main.out
	@echo Success!

.PHONY: clean
clean:
	rm -rf *.out
