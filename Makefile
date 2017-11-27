CC=gcc
CFLAGS=-g -Wall
INCLUDES=-I .
CLILIB=libcli.a
TARGET:exe ${CLILIB}
OBJ=cmd_hier.o parser.o serialize.o string_util.o clistd.o clicbext.o
exe:testapp.o ${CLILIB}
	${CC} ${CFLAGS} ${INCLUDES} testapp.o -o exe -L . -lcli
cmd_hier.o:cmd_hier.c
	${CC} ${CFLAGS} -c ${INCLUDES} cmd_hier.c -o cmd_hier.o
parser.o:parser.c
	${CC} ${CFLAGS} -c ${INCLUDES} parser.c -o parser.o
serialize.o:serialize.c
	${CC} ${CFLAGS} -c ${INCLUDES} serialize.c -o serialize.o
string_util.o:string_util.c
	${CC} ${CFLAGS} -c ${INCLUDES} string_util.c -o string_util.o
clistd.o:clistd.c
	${CC} ${CFLAGS} -c ${INCLUDES} clistd.c -o clistd.o
clicbext.o:clicbext.c
	${CC} ${CFLAGS} -c ${INCLUDES} clicbext.c -o clicbext.o
testapp.o:testapp.c
	${CC} ${CFLAGS} -c ${INCLUDES} testapp.c -o testapp.o
${CLILIB}: ${OBJ}
	ar rs ${CLILIB} ${OBJ}
clean:
	rm exe
	rm *.o
	rm ${CLILIB}
	rm CMD_HIST_RECORD_FILE.txt
install:
	cp ${CLILIB} /usr/local/lib/
	cp libcli.h /usr/include/
	cp cmdtlv.h /usr/include/
uninstall:
	rm /usr/local/lib/${CLILIB}
	rm /usr/include/libcli.h
	rm /usr/include/cmdtlv.h
