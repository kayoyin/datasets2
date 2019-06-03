LINK_TARGET = client server

OBJS = \
	client.o \
	server.o \
	common.o

REBUILDABLES = $(OBJS) $(LINK_TARGET)

all : $(LINK_TARGET)

clean:
	rm -f $(REBUILDABLES)

client : client.o common.o
	cc -g -o $@ $^

server: server.o common.o
	cc -g -o $@ $^

%.o : %.c
	cc -g -std=gnu99 -Wall -o $@ -c $<

client.o : client.c common.h
server.o: server.c common.h
common.o: common.h
