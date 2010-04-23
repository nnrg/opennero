/*
the advice is expected to be in the following grammar:
RULES -> RULE |
         RULES RULE
RULE -> if CONDS then RULE [ else RULE ] endif |
        { SETRULES }
CONDS -> TERM |
         CONDS and TERM
TERM -> false | true | variable TERMOP VARVAL
TERMOP -> <= | < | > | >=
VARVAL -> variable | value
SETRULES -> SETVAR | SETRULES SETVAR
SETVAR -> variable = EXPR | variable += EXPR
EXPR -> ETERM | EXPR + ETERM | EXPR - ETERM
ETERM -> value | variable | -variable | value*variable | variable*value
*/

%{
#include <stdio.h>
#include "advice_rep.h"
typedef struct yy_buffer_state *YY_BUFFER_STATE;
int yyparse(void);
int yylex(void);  
extern "C" int yywrap(void);
void yyerror(const char* );
YY_BUFFER_STATE yy_scan_string(const char*);
void yy_delete_buffer(YY_BUFFER_STATE);
extern Rules* parsedRules;
%}

%union {
    double number;
    unsigned int variable;
    ANode* node;
    Rule* rule;
    Conds* conds;
    SetRules* set_rules;
    SetVar* set_var;
    Term* term;
    BooleanTerm* bool_term;
    LEQTerm* leq_term;
    LTTerm* lt_term;
    GEQTerm* geq_term;
    GTTerm* gt_term;
    Expr* expr;
    Eterm* eterm;
    Rules* rules;
}

%start rules
// values
%token <number> T_NUMBER
%token <bool_term> T_FALSE
%token <bool_term> T_TRUE
%token <variable> T_VARIABLE
// keywords
%token T_IF T_THEN T_ELSE T_ENDIF T_ASSIGN T_ACCUMULATE T_AND
// punctuation
%token <leq_term> T_LEQ 
%token <lt_term> T_LT 
%token <gt_term> T_GT
%token <geq_term> T_GEQ 
%token T_LBRACE T_RBRACE
%token T_PLUS T_MINUS T_MULT

%type <conds> conds;
%type <rule> rule;
%type <rule> if_tail;
%type <term> term;
%type <set_rules> set_rules;
%type <set_var> set_var;
%type <expr> expr;
%type <eterm> eterm;
%type <rules> rules;

%%
rules: 
rule 
{
    $$ = new Rules(); $$->append($1); parsedRules = $$;
}
| rules rule
{
    $$ = $1; $$->append($2);
}
;

rule: 
T_IF conds T_THEN rule if_tail T_ENDIF
{
    $$ = new IfRule($2, $4, $5);
}
| T_LBRACE set_rules T_RBRACE
{
    $$ = $2;
}
;

if_tail: /* on nothing, return NULL */
{
    $$ = NULL;
}
| T_ELSE rule
{
    $$ = $2;
}
;

conds:
term 
{
    $$ = new Conds(); $$->append($1);
}
| conds T_AND term
{
    $1->append($3); $$ = $1;
}
;

term:
T_FALSE 
{
    $$ = $1;
}
| T_TRUE 
{
    $$ = $1;
}
| T_VARIABLE T_LEQ T_NUMBER
{
    $$ = new LEQTerm($1,Number::scale($3,$1));
}
| T_VARIABLE T_LEQ T_VARIABLE
{
    $$ = new LEQTerm($1,$3);
}
| T_VARIABLE T_LT T_NUMBER
{
    $$ = new LTTerm($1,Number::scale($3,$1));
}
| T_VARIABLE T_LT T_VARIABLE
{
    $$ = new LTTerm($1,$3);
}
| T_VARIABLE T_GT T_NUMBER
{
    $$ = new GTTerm($1,Number::scale($3,$1));
}
| T_VARIABLE T_GT T_VARIABLE
{
    $$ = new GTTerm($1,$3);
}
| T_VARIABLE T_GEQ T_NUMBER
{
    $$ = new GEQTerm($1,Number::scale($3,$1));
}
| T_VARIABLE T_GEQ T_VARIABLE
{
    $$ = new GEQTerm($1,$3);
}
;

set_rules: 
set_var
{
    $$ = new SetRules(); $$->append($1);
}
| set_rules set_var
{
    $1->append($2); $$ = $1;
}
;

set_var:
T_VARIABLE T_ASSIGN expr
{
    if (Agent::mType == Agent::eEvolved) {
        yyerror("assignment to network variables is not supported; use accumulation");
    }
    if (Variable::getType($1) == Variable::eSensor) {
        yyerror("assignment to sensor variables is not supported");
    }
    $$ = new SetVar($1,$3,SetVar::eAssignment);
}
| T_VARIABLE T_ACCUMULATE expr
{
    if (Variable::getType($1) == Variable::eSensor) {
        yyerror("accumulation to sensor variables is not supported");
    }
    $$ = new SetVar($1,$3,SetVar::eAccumulation);
}
;

expr:
eterm 
{
    $$ = new Expr(); $$->append($1);
}
| expr T_PLUS eterm
{
    $1->append($3); $$ = $1;
}
| expr T_MINUS eterm
{
    $3->negate(); $1->append($3); $$ = $1;
}
;

eterm:
T_NUMBER
{
    // Terms are in the right-hand side of assignment/accumulation expressions.
    // Since the variable on the left-hand side of such expressions is guaranteed
    // to be a non-sensor variable, we can scale the number without checking the
    // variable type.
    $$ = new Eterm(Number::scale($1));
}
|
T_VARIABLE
{
    $$ = new Eterm($1);
}
|
T_MINUS T_VARIABLE
{
    $$ = new Eterm($2); $$->negate();
}
|
T_VARIABLE T_MULT T_NUMBER
{
    $$ = new Eterm($1,$3);
}
|
T_NUMBER T_MULT T_VARIABLE
{
    $$ = new Eterm($3,$1);
}
;


%%

int yywrap()
{
    return 1;
}

void yyerror(const char *s) {
    printf("\n");
    std::ostringstream oss;
    oss << s << " at line number " << Counters::mLine;
    throw std::runtime_error(oss.str());
}

Rules* parsedRules;

Rules* yyParseAdvice(const char* advice, U32 numSensors, U32 numActions, Agent::Type type) {
    Variable::mNumSensors = numSensors;
    Variable::mNumActions = numActions;
    Counters::mLine = 1;
    Agent::mType = type;
    YY_BUFFER_STATE parse_buffer = yy_scan_string (advice);
    parsedRules = NULL;
    yyparse();
    printf("\n");
    yy_delete_buffer(parse_buffer);
    return parsedRules;
}
