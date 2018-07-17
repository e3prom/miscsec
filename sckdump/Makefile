TARGET=sckdump
CC=gcc
CFLAGS=-O2 -pipe -Wall -Werror=format-security -D_FORTIFY_SOURCE=2 -fpie -Wl,-pie -Wl,-z,relro -fstack-protector-all

default: $(TARGET)
all: default

sdump.o: sckdump.c
	$(CC) $(CFLAGS) -c sckdump.c -o sckdump.o

$(TARGET): sckdump.o
	$(CC) $(CFLAGS) sckdump.o -o $(TARGET)

clean:
	-rm -f sckdump.o
	-rm -f $(TARGET)
