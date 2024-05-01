#include <stdio.h>
#include <stdlib.h>

extern int convertirFloatAInt(float numero);

int main(int argc, char *argv[]) {

    //float numeroFloat = atof(argv[1]);
    float numeroFloat = 4.2;
    int numeroInt = convertirFloatAInt(numeroFloat);

    return numeroInt;
}