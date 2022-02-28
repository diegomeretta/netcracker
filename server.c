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

#define WORDS_PER_PACKET 10000

struct packet_struct {
	uint64_t packet_number;
	char alphabet[128];
	char hash[27];
	uint64_t number_start;
	uint64_t number_end;
	uint64_t result;
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
	fd_set file_descriptor_copy;
	fd_set file_descriptor_set;
	char buffer[P_SIZE];
	struct sockaddr_in server;
	struct sockaddr_in client;
	struct packet_struct *packet;

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = INADDR_ANY;

	int socket_descriptor = 0;
	socket_descriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (bind(socket_descriptor, (struct sockaddr *) &server, sizeof(server)) < 0) {
		perror("BIND");
		exit(-1);
	}

	listen(socket_descriptor, 5);

	FD_ZERO (&file_descriptor_set);
	FD_SET (socket_descriptor, &file_descriptor_set);

	int socket_descriptor_copy = 0;
	packet = (struct packet_struct *) buffer;
	int readed = 0;

	int generated_packets = 0;
	char *string_alphabet = malloc(128 * sizeof(char));
	load_file("alphabet.txt", string_alphabet);
	char *string_hash = malloc(27 * sizeof(char));
	load_file("hash.txt", string_hash);
	int termino = 0;
	uint64_t contador = 0;

	while (termino == 0) {
		file_descriptor_copy = file_descriptor_set;
		select (FD_SETSIZE, &file_descriptor_copy , NULL , NULL , NULL );
		if ( FD_ISSET(socket_descriptor, &file_descriptor_copy)) {
			lon = sizeof(client);
			socket_descriptor_copy = accept(socket_descriptor, (struct sockaddr *) &client, &lon);
			FD_SET ( socket_descriptor_copy , &file_descriptor_set);
		}

		for ( socket_descriptor_copy = 1 ; socket_descriptor_copy < FD_SETSIZE ; socket_descriptor_copy++ ) {
			if ( FD_ISSET (socket_descriptor_copy, &file_descriptor_copy) && (socket_descriptor_copy != socket_descriptor)) {
				readed = 0;
				while (readed < P_SIZE) {
					if ((n = recv(socket_descriptor_copy, buffer + readed, P_SIZE - readed, 0)) <= 0) {
						close(socket_descriptor_copy);
						FD_CLR ( socket_descriptor_copy , &file_descriptor_set);
						break;
					}
				readed = readed + n;
				}
				if ( readed == P_SIZE ) {
					if (ntohl(packet->result) != 0){
						char resultado[128];
						devuelvePalabra(ntohl(packet->result), string_alphabet, resultado);
						printf ("Resultado encontrado : %s\n", resultado);
						termino = 1;
					} else {
						generated_packets++;
						packet->packet_number = htonl(generated_packets);
						strcpy(packet->alphabet, string_alphabet);
						strcpy(packet->hash, string_hash);
						packet->number_start = htonl(contador + 1);
						contador = contador + WORDS_PER_PACKET;
						packet->number_end = htonl(contador);
						packet->result = htonl(0);
						char start_word[128];
						devuelvePalabra(ntohl(packet->number_start), string_alphabet, start_word);
						char finish_word[128];
						devuelvePalabra(ntohl(packet->number_end), string_alphabet, finish_word);
						printf ("Paquete: %d, inicio: %s, fin: %s\n", ntohl(packet->packet_number), start_word, finish_word);
						if ((n = send(socket_descriptor_copy, buffer, P_SIZE, 0) < 0)) {
							perror("Send");
						}
					}
				}
			}
		}
	}
}
