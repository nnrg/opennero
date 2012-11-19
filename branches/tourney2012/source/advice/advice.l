/* %option main */
/* for windows */
%option never-interactive
%{
#include <stdio.h>
#include "advice/advice_rep.h"
#include "advice.tab.hh"
void yyerror(const char* );
%}
%%
s[0123456789]+                              ECHO; yylval.variable = Variable::translate(Variable::eSensor, atoi(yytext+1)); return T_VARIABLE;
ea[0123456789]+                             ECHO; yylval.variable = Variable::translate(Variable::eEvolvedAction, atoi(yytext+2)); return T_VARIABLE;
a[0123456789]+                              ECHO; yylval.variable = Variable::translate(Variable::eAction, atoi(yytext+1)); return T_VARIABLE;
g[0123456789]+                              ECHO; yylval.variable = Variable::translate(Variable::eGeneral, atoi(yytext+1)); return T_VARIABLE;
[-+]*[0123456789]+\.[0123456789]+|[-+]*\.[0123456789]+|[-+]*[0123456789]+         ECHO; yylval.number = Number::checkRange(atof(yytext)); return T_NUMBER;
if                                          ECHO; return T_IF;
elif                                        ECHO; return T_ELIF;
else                                        ECHO; return T_ELSE;
\<\=                                        ECHO; return T_LEQ;
\<                                          ECHO; return T_LT;
\>\=                                        ECHO; return T_GEQ;
\>                                          ECHO; return T_GT;
\{                                          ECHO; return T_LBRACE;
\}                                          ECHO; return T_RBRACE;
\=                                          ECHO; return T_ASSIGN;
\+                                          ECHO; return T_PLUS;
\-                                          ECHO; return T_MINUS;
\*                                          ECHO; return T_MULT;
and                                         ECHO; return T_AND;
true                                        ECHO; yylval.bool_term = BooleanTerm::getTrue(); return T_TRUE;
false                                       ECHO; yylval.bool_term = BooleanTerm::getFalse(); return T_FALSE;
\n                                          ECHO; Counters::mLine++; /* increment line count and ignore newline */
[ \t]+                                      ECHO; /* ignore white space */
#.*                                         ECHO; /* ignore comment lines */
.                                           ECHO; yyerror("unexpected character"); /* any other characters produce error */
%%
