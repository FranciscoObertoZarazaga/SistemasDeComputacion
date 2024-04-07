#include <stdio.h>
#include <stdlib.h>

int convertirFloatAInt(float numero) {
    int resultado = (int)numero;
    return resultado + 1;
}

int main(int argc, char *argv[]) {

    float numeroFloat = atof(argv[1]);
    int numeroInt = convertirFloatAInt(numeroFloat);

    return numeroInt;
}