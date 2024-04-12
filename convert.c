#include <stdio.h>
#include <stdlib.h>


int convertirFloatAInt(float numero) {
    int resultado;
    asm(
        "cvttss2si %1, %%eax\n"  // Convierte el float a int en eax, truncando los decimales
        "addl $1, %%eax\n"        // Suma 1 al valor entero
        "movl %%eax, %0\n"        // Mueve el resultado a la variable 'resultado'
        : "=r" (resultado)        // salida en un registro
        : "m" (numero)            // entrada
        : "eax"                   // clobbers
    );
    return resultado;
}

int main(int argc, char *argv[]) {

    float numeroFloat = atof(argv[1]);
    int numeroInt = convertirFloatAInt(numeroFloat);

    return numeroInt;
}