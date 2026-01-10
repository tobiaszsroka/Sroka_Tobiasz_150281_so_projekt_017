CC = gcc
CFLAGS = -Wall -pthread -g

all: dziekan komisja kandydat

dziekan: dziekan.c common.h
	$(CC) $(CFLAGS) -o dziekan dziekan.c

komisja: komisja.c common.h
	$(CC) $(CFLAGS) -o komisja komisja.c

kandydat: kandydat.c common.h
	$(CC) $(CFLAGS) -o kandydat kandydat.c

clean:
	rm -f dziekan komisja kandydat *.o
	rm -f raport.txt
