CC = gcc
CFLAGS = -Wall
LDFLAGS = -lcurl

all:
	$(CC) $(CFLAGS) -o main.o main.c cJSON.c $(LDFLAGS)

clean:
	rm -f main.o

