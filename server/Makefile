all : server

server : server.o encip.a
	gcc -Wall server.o encip.a -pthread -o server

server.o : server.c
	gcc -c server.c
	
encip.a : encip.o
	ar -r encip.a encip.o

encip.o : encip.c
	gcc -c encip.c

clean :
	rm -rf *.o main
