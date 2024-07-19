# Makefile for RDMA Performance Measurements

CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -libverbs

# Source files
SRC = bw_template.c

# Targets
TARGETS = server client

.PHONY: all clean

# Default target
all: $(TARGETS)

# Build server
server: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Create client as a symlink to server
client: server
	ln -sf server client

# Clean up build files
clean:
	rm -f $(TARGETS) client
