CC = gcc
CFLAGS = -Wall -Wextra -pthread -g

all: pzip punzip

run-pzip: all
	./pzip testfile1 testfile2 testfile3 testfile4 test0 test1

run-punzip: all
	./punzip testfile1.z testfile2.z testfile3.z testfile4.z test0.z test1.z

pzip: pzip.o
	$(CC) $(CFLAGS) -o pzip pzip.o

pzip.o: pzip.c
	$(CC) $(CFLAGS) -c pzip.c

punzip: punzip.o
	$(CC) $(CFLAGS) -o punzip punzip.o

punzip.o: punzip.c
	$(CC) $(CFLAGS) -c punzip.c

clean:
	rm -f *.o *.out *example *zip *.z *_unzip *.unz

.PHONY: all clean run