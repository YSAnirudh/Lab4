all: 548.exe

548.exe: 548.o
	gcc -o 548.exe 548.o -lreadline
	
548.o: 548.c
	gcc -c 548.c -lreadline
	
clean:
	rm 548.exe 548.o