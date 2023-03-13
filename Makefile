CC = cc
CFLAGS = -Wall -g -pedantic
EXEC = client

all: $(EXEC)

client: src/client.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXEC) *.dSYM

.PHONY: all main clean
	