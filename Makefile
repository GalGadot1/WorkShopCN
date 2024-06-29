CC = gcc
CFLAGS = -std=c11 -Wall
LDFLAGS =

SRCDIR = src
BINDIR = bin

SERVER_SRC = server.c
CLIENT_SRC = client.c

SERVER_OBJ = server.o
CLIENT_OBJ = client.o

SERVER_BIN = server
CLIENT_BIN = client

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

$(CLIENT_BIN): $(CLIENT_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

$(SERVER_OBJ): $(SERVER_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(CLIENT_OBJ): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BINDIR)/*.o $(SERVER_BIN) $(CLIENT_BIN)

.PHONY: all clean

