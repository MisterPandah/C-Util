all: proxyFilter 


CLIBS=-pthread
CC=gcc
CPPFLAGS=
CFLAGS=-g

PROXYOBJS=proxyFilter.o Thread.o Socket.o HTTPlib.o Proxy.o String.o

proxyFilter: $(PROXYOBJS)
	$(CC) -o proxyFilter $(PROXYOBJS)  $(CLIBS)



clean:
	rm -f *.o
	rm -f proxyFilter

