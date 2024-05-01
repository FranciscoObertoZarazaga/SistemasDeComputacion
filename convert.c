#include <stdio.h>
#include <stdlib.h>

extern int convertirFloatAInt(float numero);

int main(int argc, char *argv[]) {

    float numeroFloat = atof(argv[1]);
    int numeroInt = convertirFloatAInt(numeroFloat);

    return numeroInt;
}