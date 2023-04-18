server: server.o channels.o
	g++ server.o channels.o -o server

client: client.o channels.o
	g++ client.o channels.o -o client

server.o: server.c
	g++ -c -g server.c

channels.o: channels.c
	g++ -c -g channels.c

client.o: client.c
	g++ -c -g client.c

clean:
	rm -f *.o client server
