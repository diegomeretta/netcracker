compilar: servidor cliente

servidor:
	gcc -o servidor.out servidor.c -lm
cliente:
	gcc -o cliente.out cliente.c -lcrypt -lm
