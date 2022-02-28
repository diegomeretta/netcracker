compilar: common server client

common:
	gcc -o common.o -c common.c
server:
	gcc -o server.out server.c common.o -lm
client:
	gcc -o client.out client.c common.o -lcrypt -lm
