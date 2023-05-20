SRCS_CLIENT = src/client.c src/message.c src/file.c src/utils.c
SRCS_SERVER = src/server.c src/message.c src/file.c src/utils.c

CC = cc
CFLAGS = -Wall -g -pedantic
CCLINK = -pthread
EXEC = client server

all: ${EXEC}

client: ${SRCS_CLIENT}
	$(CC) $(CFLAGS) $(CCLINK) $^ -o $@

server: ${SRCS_SERVER}
	$(CC) $(CFLAGS) $(CCLINK) $^ -o $@

clean:
	rm -rf ${EXEC} *.dSYM

.PHONY: all client server clean
