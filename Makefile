PROG_CXX=perfdb
SRCS := \
	CounterAgent.cpp \
	DetermineAgentVisitor.cpp \
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
	UncorePciUnit.cpp \
	UncoreMsrUnit.cpp \
	UncoreUnit.cpp \

CLEANFILES+= lex.cpp parser.cpp parser.cpp.h parser.i

LDADD=-lpmc -lncurses
MAN=

parser.cpp parser.cpp.h: parser.yy
	yacc -d -o parser.cpp parser.yy

parser.h: parser.cpp
	cp parser.cpp.h parser.h

lex.cpp: lex.ll parser.h
	lex -olex.cpp lex.ll

beforedepend: lex.cpp parser.cpp

CFLAGS+=-I. -I${.CURDIR} -g

#WARNS ?= 3

.include <bsd.prog.mk>

