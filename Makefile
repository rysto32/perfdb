PROG_CXX=perfdb
SRCS := \
	lex.cpp \
	parser.cpp \
	perfdb.cpp \
	pmccontext.cpp \
	screenstate.cpp \
	keyaction.cpp \
	UncoreAgent.cpp \
	UncoreAllocatedCounter.cpp \
	UncoreContext.cpp \
	UncoreCounter.cpp \
	UncoreEvent.cpp \
	UncoreUnit.cpp \

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

