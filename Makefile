PROG_CXX=perfdb
SRCS=lex.cpp parser.cpp perfdb.cpp pmccontext.cpp
CLEANFILES+= lex.cpp parser.cpp parser.cpp.h
LDADD=-lpmc -lncurses
MAN=

parser.cpp parser.cpp.h: parser.yy
	yacc -do parser.cpp parser.yy

lex.cpp: lex.ll
	lex -olex.cpp lex.ll

beforedepend: lex.cpp parser.cpp

CFLAGS+=-I. -I${.CURDIR}

.include <bsd.prog.mk>

