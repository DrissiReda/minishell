default : Mishell

Mishell.o: Mishell.c
	gcc -c -g Mishell.c -o Mishell.o
Mishell : Mishell.o
	gcc -lreadline Mishell.o -o Mishell
clean :
	rm -f Mishell.o
	rm -f Mishell

