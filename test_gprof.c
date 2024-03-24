//test_gprof.c
#include<stdio.h>
#include <time.h>

void new_func1(void);

void func1(void)
{
    printf("\n Inside func1 \n");
    int i = 0;

    for(;i<0xffffffff;i++);
    new_func1();

    return;
}

static void func2(void)
{
    printf("\n Inside func2 \n");
    int i = 0;

    for(;i<0xafffffff;i++);
    return;
}

int main(void)
{
    // Obtener el tiempo de inicio
    clock_t inicio = clock();
    printf("\n Inside main()\n");
    int i = 0;

    for(;i<0xfffff11;i++);
    func1();
    func2();

   // Obtener el tiempo de finalización
    clock_t final = clock();

    // Calcular el tiempo de ejecución en segundos
    double tiempoEjecucion = (double)(final - inicio) / CLOCKS_PER_SEC;

    printf("Tiempo de ejecución: %f segundos\n", tiempoEjecucion);

    return 0;
}

