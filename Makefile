CC = gcc
CFLAGS = -Wall -g -pedantic
EXEC = main

.PHONY: clean

all: $(EXEC)

main:
	echo "à définir"

clean:
	rm -rf $(EXEC) *.dSYM
	