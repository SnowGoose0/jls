CC = gcc
CFLAGS = -Wall -Wextra

myls: myls.o utils.o
	$(CC) $(CFLAGS) -o myls myls.o utils.o

myls.o: myls.c myls.h
	$(CC) $(CFLAGS) -c myls.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f myls myls.o utils.o
