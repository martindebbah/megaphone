SRCS_CLIENT = src/client.c src/message.c
SRCS_SERVER = src/server.c

CC = cc
CFLAGS = -Wall -g -pedantic
EXEC = client server

all: ${EXEC}

client: ${SRCS_CLIENT}
	$(CC) $(CFLAGS) $^ -o $@

server: ${SRCS_SERVER}
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf ${EXEC} *.dSYM

.PHONY: all client server clean
