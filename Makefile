compilar: common servidor cliente

common:
	gcc -o common.o -c common.c
servidor:
	gcc -o servidor.out servidor.c common.o -lm
cliente:
	gcc -o cliente.out cliente.c common.o -lcrypt -lm
