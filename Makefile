all: compilar link 

compilar: 
	gcc *.c -g -c

link:
	gcc *.o -o main.exe

