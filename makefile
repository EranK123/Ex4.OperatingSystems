CFLAGS=-Wall
COMP = clang++

all: Client Server test

Client: Client.o
	$(COMP) $(CFLAGS) Client.o -o Client -lpthread

Server: Server.o
	$(COMP) $(CFLAGS) Server.o -o Server -lpthread

test: test.o
	$(COMP) $(CFLAGS) test.o -o test

Server.o: server.cpp
	$(COMP) -c $(CFLAGS) server.cpp -lpthread

Client.o: client.cpp
	$(COMP) -c $(CFLAGS) client.cpp -lpthread

test.o: test.cpp
	$(COMP) -c $(CFLAGS) test.cpp

.PHONY: clean all

clean:
	rm *.o Client Server test
			