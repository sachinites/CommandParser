CC=gcc
CFLAGS=-g -Wall
INCLUDES=-I .
TARGET:exe
OBJ=cmd_hier.o parser.o serialize.o string_util.o
exe:${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o exe 
cmd_hier.o:cmd_hier.c
	gcc -g -c ${INCLUDES} cmd_hier.c -o cmd_hier.o
parser.o:parser.c
	gcc -g -c ${INCLUDES} parser.c -o parser.o
serialize.o:serialize.c
	gcc -g -c ${INCLUDES} serialize.c -o serialize.o
string_util.o:string_util.c
	gcc -g -c ${INCLUDES} string_util.c -o string_util.o
clean:
	rm exe
	rm *.o
