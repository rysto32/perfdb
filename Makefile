PROG_CXX=perfdb
SRCS=lex.cpp parser.cpp perfdb.cpp pmccontext.cpp screenstate.cpp keyaction.cpp
CLEANFILES+= lex.cpp parser.cpp parser.h parser.i
LDADD=-lpmc -lncurses
MAN=

parser.cpp parser.cpp.h: parser.yy
	yacc -i -d -o parser.cpp parser.yy

lex.cpp: lex.ll
	lex -olex.cpp lex.ll

beforedepend: lex.cpp parser.cpp

CFLAGS+=-I. -I${.CURDIR}

.include <bsd.prog.mk>

