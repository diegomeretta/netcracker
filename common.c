#include <string.h>
#include <math.h>

#include "common.h"

void devuelvePalabra(int posicion, char alfabeto[128], char * palabra){
	int largo = 1;
	int corte = 0;
	int primero = 1;
	int ultimo = strlen(alfabeto);
	while (largo <= strlen(alfabeto) && corte == 0){
		if (ultimo >= posicion){
			corte = 1;
		} else {
		largo++;
		primero = ultimo + 1;
		ultimo = ultimo + pow(strlen(alfabeto), largo);
		}
	}
	int posicionRango = posicion - primero;
	palabra[largo] = '\0';
	int i = largo;
	int posicionLetra = 0;
	do {
		posicionLetra = posicionRango % strlen(alfabeto);
		palabra[i - 1] = alfabeto[posicionLetra];
		posicionRango = (posicionRango / strlen(alfabeto));
		i--;
	}
	while (i > 0);
}