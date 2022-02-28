#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>

#include "common.h"

#define P_SIZE sizeof (struct packet_struct)

struct packet_struct {
	uint64_t packet_number;
	char alfabeto[128];
	char hash[27];
	uint64_t inicio;
	uint64_t fin;
	uint64_t resultado;
};

void load_file(char *file_name, char *string_array) {
	int i = 0;
	FILE *file;
	char character;
	file = fopen(file_name, "r");
	if (file == NULL) {
		printf("\nError de apertura del archivo. \n");
	}
	while ((feof(file) == 0) && (character = fgetc(file)) != '\n')  {
		string_array[i] = character;
		i++;
	}
	string_array[i] = '\0';
	fclose(file);
}

int main(){

	int n = 0;
	int lon = 0;
	fd_set copia;
	fd_set conjunto;
	char buffer[P_SIZE];
	struct sockaddr_in servidor;
	struct sockaddr_in cliente;
	struct packet_struct *paquete;

	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(4444);
	servidor.sin_addr.s_addr = INADDR_ANY;

	int sd = 0;
	sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (bind(sd, (struct sockaddr *) &servidor, sizeof(servidor)) < 0) {
		perror("BIND");
		exit(-1);
	}

	listen(sd, 5);

	FD_ZERO (&conjunto);
	FD_SET (sd, &conjunto);

	int sdc = 0;
	paquete = (struct packet_struct *) buffer;
	int leidos = 0;

	int cantidadPaquetes = 0;
	char *cadenaalfabeto = malloc(128 * sizeof(char));
	load_file("alfabeto.txt", cadenaalfabeto);
	char *cadenaHash = malloc(27 * sizeof(char));
	load_file("hash.txt", cadenaHash);
	int termino = 0;
	uint64_t contador = 0;

	while (termino == 0) {
		copia = conjunto;
		select (FD_SETSIZE, &copia , NULL , NULL , NULL );
		if ( FD_ISSET(sd, &copia)) {
			lon = sizeof(cliente);
			sdc = accept(sd, (struct sockaddr *) &cliente, &lon);
			FD_SET ( sdc , &conjunto);
		}

		for ( sdc = 1 ; sdc < FD_SETSIZE ; sdc++ ) {
			if ( FD_ISSET (sdc, &copia) && (sdc != sd)) {
				leidos = 0;
				while (leidos < P_SIZE) {
					if ((n = recv(sdc, buffer + leidos, P_SIZE - leidos, 0)) <= 0) {
						close(sdc);
						FD_CLR ( sdc , &conjunto);
						break;
					}
				leidos = leidos + n;
				}
				if ( leidos == P_SIZE ) {
					if (ntohl(paquete->resultado) != 0){
						char resultado[128];
						devuelvePalabra(ntohl(paquete->resultado), cadenaalfabeto, resultado);
						printf ("Resultado encontrado : %s\n", resultado);
						termino = 1;
					} else {
						cantidadPaquetes++;
						paquete->packet_number = htonl(cantidadPaquetes);
						strcpy(paquete->alfabeto, cadenaalfabeto);
						strcpy(paquete->hash, cadenaHash);
						paquete->inicio = htonl(contador + 1);
						contador = contador + 10000;
						paquete->fin = htonl(contador);
						paquete->resultado = htonl(0);
						char palabraInicio[128];
						devuelvePalabra(ntohl(paquete->inicio), cadenaalfabeto, palabraInicio);
						char palabraFin[128];
						devuelvePalabra(ntohl(paquete->fin), cadenaalfabeto, palabraFin);
						printf ("Paquete: %d, inicio: %s, fin: %s\n", ntohl(paquete->packet_number), palabraInicio, palabraFin);
						if ((n = send(sdc, buffer, P_SIZE, 0) < 0)) {
							perror("Send");
						}
					}
				}
			}
		}
	}
}
