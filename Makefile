all: compile link borrar

compile:
	gcc *.c -c

link:
	gcc *.o -g -o convert.exe

borrar:
	rm -f *.o