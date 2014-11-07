# Matthew Fritze
# 1360555
# CMPUT 379
# Assignment 2

CFLAGS = -g -Wall 
OBJS   = server_p.o server_f.o shared_server.o
POBJS  = server_p.o shared_server.o
FOBJS  = server_f.o shared_server.o
PDEPS  = shared_server.h shared_server.h
FDEPS  = server_f.c server_f.h
SDEPS  = shared_server.c shared_server.h

all: $(OBJS)
	gcc $(CFLAGS) $(POBJS) -o server_p -lpthread
	gcc $(CFLAGS) $(FOBJS) -o server_f

server_f.o: $(FDEPS) $(SDEPS)  
	gcc $(CFLAGS) -c server_f.c -o server_f.o

server_p.o: $(PDEPS) $(SDEPS)
	gcc $(CFLAGS) -c server_p.c -o server_p.o 

shared_server.o: $(SDEPS)
	gcc $(CFLAGS) -c shared_server.c -o shared_server.o

clean:
	rm -rf *.o server_f server_p

