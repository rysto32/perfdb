%{
#include <string>
#include <map>
#include "expression.h"
#include "statistic.h"
#include "page.h"
#include "cpu.h"
#include "pointervector.h"

int yylex(void);

extern int yylineno;
extern int yychar;

void yyerror(const char* msg)
{
	fprintf(stderr, "%s @ line %d(next=%c(%d))\n", msg, yylineno, yychar, yychar);
}
%}

%union {
	double threshold;
	std::string *name;
	Expression *expr;
	PointerVector<Page> *pageList;
	Page *page;
	PointerVector<Statistic> *statList;
	Statistic *stat;
	CpuDef *cpuDef;
	PointerVector<CpuDef> *cpuList;
}

%token PAGE
%token STAT
%token SHORTCUT
%token GOOD_TOK OK_TOK BAD_TOK
%token <threshold> NUMBER
%token <name> ID
%token CPU_TOK

%type <pageList> page_list
%type <page> page
%type <statList> stat_list
%type <stat> stat
%type <cpuDef> cpu_def
%type <cpuList> cpu_list

%type <expr> expr pmc_expr primary_expr mult_expr add_expr

%start cpu_list

%%

cpu_list:
	cpu_list cpu_def
	{
		$1->push_back($2);
		$$ = $1;
	}
	| cpu_def
	{
		$$ = new PointerVector<CpuDef>;
		$$->push_back($1);
	}
	;

cpu_def:
	CPU_TOK ID page_list
	{
		$$ = new CpuDef($2, $3);
	}
	;

page_list:
	page_list page
	{
		$1->push_back($2);
		$$ = $1;
	}
	| page
	{
		$$ = new PointerVector<Page>;
		$$->push_back($1);
	}
	;

page:
	PAGE ID SHORTCUT ID stat_list
	{
		$$ = new Page($2, $4, $5);
	}
	;

stat_list:
	stat_list stat
	{
		$1->push_back($2);
		$$ = $1;
	}
	| stat
	{
		$$ = new PointerVector<Statistic>;
		$$->push_back($1);
	}
	;

stat:
	STAT ID expr GOOD_TOK NUMBER OK_TOK NUMBER BAD_TOK NUMBER
	{
		$$ = new Statistic($2, $3, $5, $7, $9);
	}
	;

pmc_expr:
	ID
	{
		$$ = new PmcExpr($1);
	}
	;

primary_expr:
	pmc_expr
	{
		$$ = $1;
	}
	| '(' expr ')'
	{
		$$ = $2;
	}
	| NUMBER
	{
		$$ = new ConstExpr($1);
	}
	;

mult_expr:
	primary_expr
	{
		$$ = $1;
	}
	| mult_expr '*' primary_expr
	{
		$$ = new BinaryExpr($1, BinaryExpr::MULT, $3);
	}
	| mult_expr '/' primary_expr
	{
		$$ = new BinaryExpr($1, BinaryExpr::DIV, $3);
	}
	;

add_expr:
	mult_expr
	{
		$$ = $1;
	}
	| add_expr '+' mult_expr
	{
		$$ = new BinaryExpr($1, BinaryExpr::ADD, $3);
	}
	| add_expr '-' mult_expr
	{
		$$ = new BinaryExpr($1, BinaryExpr::SUB, $3);
	}
	;

expr:
	add_expr
	{
		$$ = $1;
	}
	;


