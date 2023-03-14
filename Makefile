CC = cc
CFLAGS = -Wall -g -pedantic
EXEC = client

all: $(EXEC)

client: src/client.c src/billet.c # Voir pour simplifier l'écriture
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(EXEC) *.dSYM

.PHONY: all client clean
	