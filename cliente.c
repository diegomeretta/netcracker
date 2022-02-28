#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "common.h"

#define P_SIZE sizeof ( struct protocolo )

struct protocolo {
	uint64_t numeroPaquete;
	char alfabeto[128];
	char hash[27];
	uint64_t inicio;
	uint64_t fin;
	uint64_t resultado;
};

int main(int argc, char *argv[]){

	char buffer[P_SIZE];
	struct sockaddr_in servidor;
	struct protocolo *paquete;
	struct hostent *h;

	if (argc < 2) {
		printf("Ejecute %s (nombre del servidor)\n", argv[0]);
		exit(-1);
	}

	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(4444);

	if (h = gethostbyname(argv[1])) {
		memcpy(&servidor.sin_addr, h->h_addr, h->h_length);
	} else {
		printf("No se encontro %s \n", argv[1]);
		exit(-1);
	}

	int sd = 0;
	sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connect(sd, (struct sockaddr *) &servidor, sizeof(servidor)) < 0) {
		perror("Connect:");
		exit(-1);
	}

	uint64_t nroPaquete = 0;
	char alfabeto[128] = "";
	char hash[27] = "";
	uint64_t inicio = 0;
	uint64_t fin = 0;
	uint64_t resultado = 0;

	paquete = (struct protocolo *) buffer;

	paquete->numeroPaquete = htons(nroPaquete);
	strcpy(paquete->alfabeto, alfabeto);
	strcpy(paquete->hash, hash);
	paquete->inicio = htons(inicio);
	paquete->fin = htons(fin);
	paquete->resultado = htons(resultado);

	send(sd, buffer, P_SIZE, 0);
	int leidos = 0;
	int termino = 0;

	while (termino == 0) {
		leidos = 0;
		int n = 0;
		while (leidos < P_SIZE) {
			if ((n = recv(sd, buffer + leidos, P_SIZE - leidos, 0)) <= 0) {
				close(sd);
				break;
			}
			leidos = leidos + n;
		}
		printf("Se recibio el paquete: %d, el alfabeto: %s, y el hash: %s\n", ntohl(paquete->numeroPaquete), paquete->alfabeto, paquete->hash);
		inicio = ntohl(paquete->inicio);
		fin = ntohl(paquete->fin);
		{
		do{
			char *codificado;
			char *salt = "$1$";
			char pa[128];
			devuelvePalabra(inicio, paquete->alfabeto, pa);
			codificado = crypt(pa,salt);
			if (strcmp(codificado, paquete->hash) == 0){
				resultado = inicio;
				termino = 1;
			}
			inicio++;
		} while ((inicio < fin) && (termino == 0));
		}
		//sleep(1);
		if (resultado != 0) {
			printf("Resultado: %zu\n", resultado);
			paquete = (struct protocolo *) buffer;
			paquete->numeroPaquete = htons(nroPaquete);
			strcpy(paquete->alfabeto, alfabeto);
			strcpy(paquete->hash, hash);
			paquete->inicio = htons(inicio);
			paquete->fin = htons(fin);
			paquete->resultado = htonl(resultado);
			send(sd, buffer, P_SIZE, 0);
			close(sd);
			exit(0);
		}
		inicio = 0;
		fin = fin++;

		paquete = (struct protocolo *) buffer;

		paquete->numeroPaquete = htons(nroPaquete);
		strcpy(paquete->alfabeto, alfabeto);
		strcpy(paquete->hash, hash);
		paquete->inicio = htons(inicio);
		paquete->fin = htons(fin);
		paquete->resultado = htons(resultado);
		send(sd, buffer, P_SIZE, 0);
	}
}
