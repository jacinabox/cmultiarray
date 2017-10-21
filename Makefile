CExceptions.o:
	gcc -c src/CExceptions.c
transpose.o: reflectable.o multiarray.o CExceptions.o
	gcc -c src/transpose.c
reflectable.o: CExceptions.o
	gcc -c src/reflectable.c
multiarray.o: CExceptions.o
	gcc -c src/multiarray.c src/CExceptions.h

all: transpose.o reflectable.o multiarray.o CExceptions.o
	gcc -o ctest reflectable.o CExceptions.o multiarray.o transpose.o -I src
