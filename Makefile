output: gates child
	
gates: ask2.o
	gcc ask2.o -o gates

child: child.o
	gcc child.o -o child

child.o: child.c
	gcc -c child.c
	
ask2.o: ask2.c
	gcc -c ask2.c
	
clean:
	rm -rf *o gates child
