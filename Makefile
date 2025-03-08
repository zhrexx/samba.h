CC = gcc
CFLAGS = -O2 -fPIC

all: samba.o libsamba.a libsamba.so

samba.o: samba.c samba.h
	$(CC) $(CFLAGS) -c samba.c -o samba.o 

libsamba.a: samba.o
	ar rcs $@ $^

libsamba.so: samba.o
	$(CC) -shared -o $@ $^

samba: samba_compiler.c 
	$(CC) -lsamba -I. -L. -Wl,-rpath=$(PWD) samba_compiler.c -o samba

clean:
	rm samba.o libsamba*
