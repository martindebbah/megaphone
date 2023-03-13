CC = cc
CFLAGS = -Wall -g -pedantic
EXEC = main

all: $(EXEC)

main: src/main.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXEC) *.dSYM

.PHONY: all main clean
	