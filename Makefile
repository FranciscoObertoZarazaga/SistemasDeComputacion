all: compilar link borrar

compilar: 
	gcc *.c -g -c

link:
	gcc *.o -pg -o main.exe

borrar:
	rm *.o