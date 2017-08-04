CC=gcc
CFLAGS=-g -Wall
INCLUDES=-I .
CLILIB=libcli.a
TARGET:exe ${CLILIB}
OBJ=cmd_hier.o parser.o serialize.o string_util.o
exe:${OBJ} testapp.o
	${CC} ${CFLAGS} ${OBJ} testapp.o -o exe 
cmd_hier.o:cmd_hier.c
	gcc -g -c ${INCLUDES} cmd_hier.c -o cmd_hier.o
parser.o:parser.c
	gcc -g -c ${INCLUDES} parser.c -o parser.o
serialize.o:serialize.c
	gcc -g -c ${INCLUDES} serialize.c -o serialize.o
string_util.o:string_util.c
	gcc -g -c ${INCLUDES} string_util.c -o string_util.o
testapp.o:testapp.c
	gcc -g -c ${INCLUDES} testapp.c -o testapp.o
${CLILIB}: ${OBJ}
	ar rs ${CLILIB} ${OBJ}
clean:
	rm exe
	rm *.o
	rm ${CLILIB}
