prog: server.o client.o serverImage.o
	gcc server.o -o server.exe
	gcc client.o -o client.exe
	gcc serverImage.o -o serverImage.exe

main.o: server.c client.c serverImage.c
	gcc -Wall -c  server.c 
	gcc -Wall -c  client.c
	gcc -Wall -c  serverImage.c

rm:  
	rm *.exe

all: prog

PHONY: clean

clean:
	rm client.o client.exe
	rm server.o server.exe
	rm serverImage.o serverImage.exe
