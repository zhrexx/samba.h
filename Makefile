CC = gcc
CFLAGS = -O2 -fPIC

all: samba.o vector.o libsamba.a libsamba.so example

samba.o: samba.c samba.h
	$(CC) $(CFLAGS) -c samba.c -o samba.o

vector.o: vector.c vector.h
	$(CC) $(CFLAGS) -c vector.c -o vector.o

libsamba.a: samba.o vector.o
	ar rcs $@ $^

libsamba.so: samba.o vector.o
	$(CC) -shared -o $@ $^

example: example.c libsamba.so
	$(CC) -I. -L. -Wl,-rpath=$(PWD) -static example.c -o example -lsamba

clean:
	rm -f *.o lib*.a lib*.so example
