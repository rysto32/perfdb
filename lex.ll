%{
#include <string>
#include "expression.h"
#include "statistic.h"
#include "page.h"
#include "pointervector.h"
#include "cpu.h"
#include "parser.h"
%}

%option noyywrap
%option yylineno

%x LINE_COMMENT

%%

"#"         BEGIN(LINE_COMMENT);
"+"         { return '+'; }
"-"         { return '-'; }
"*"         { return '*'; }
"/"         { return '/'; }
"("         { return '('; }
")"         { return ')'; }

page        { return PAGE; }
stat        { return STAT; }
shortcut    { return SHORTCUT; }
good        { return GOOD_TOK; }
bad         { return BAD_TOK; }
ok          { return OK_TOK; }
cpu         { return CPU_TOK; }

[0-9]+(\.[0-9]*([eE][-+]?[0-9]+)?)? {
		yylval.threshold = atof(yytext);
		return NUMBER;
	}
[0-9]*\.[0-9]+([eE][-+]?[0-9]+)? {
		yylval.threshold = atof(yytext) ;
		return NUMBER;
	}

[a-zA-Z_][a-zA-Z_0-9.]* {
		yylval.name = new std::string(yytext);
		return ID;
	}

[[:space:]]     {}

.	{
		fprintf(stderr, "unexpected character %s\n", yytext);
		return YYERRCODE;
	}

\"[^"]+\" {
		/* yyleng - 2: 1 to remove trailing ", 1 to remove '\0' */
		yylval.name = new std::string(yytext, 1, yyleng - 2);
		return ID;
	}

<LINE_COMMENT>{
	\n	BEGIN(0);
	.	{}
}
