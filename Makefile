
CFLAGS=-I. -Wall -g -ggdb -O3

OBJS=perfdb.o pmccontext.o parser.o lex.o
EXEC=perfdb

LOCAL_LIBS=-lpmc -lncurses

all: ${EXEC}
release: all

parser.cpp: parser.yy
	yacc -do parser.cpp parser.yy

lex.cpp: lex.l
	lex -olex.cpp lex.l

${EXEC}: ${OBJS}
	${CXX} ${OBJS} -o $(EXEC) ${LOCAL_LIBS}

clean:
	rm -f ${EXEC} ${OBJS} parser.cpp lex.cpp
