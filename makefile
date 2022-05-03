CFLAGS=-Wall
COMP = clang++

all: client server test

client: client.o
	$(COMP) $(CFLAGS) client.o -o client -lpthread

server: server.o
	$(COMP) $(CFLAGS) server.o -o server -lpthread

test: test.o
	$(COMP) $(CFLAGS) test.o -o test

server.o: server.cpp
	$(COMP) -c $(CFLAGS) server.cpp -lpthread

client.o: client.cpp
	$(COMP) -c $(CFLAGS) client.cpp -lpthread

test.o: test.cpp
	$(COMP) -c $(CFLAGS) test.cpp

.PHONY: clean all

clean:
	rm *.o client server test
			