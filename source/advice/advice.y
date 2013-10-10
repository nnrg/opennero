/*
the advice is expected to be in the following grammar:
RULES -> RULE | RULES RULE
RULE -> if CONDS { SETRULES } IFTAIL
IFTAIL -> | elif CONDS { SETRULES } IFTAIL | else { SETRULES }
CONDS -> TERM | CONDS and TERM
TERM -> false | true | variable TERMOP VARVAL
TERMOP -> <= | < | > | >=
VARVAL -> variable | value
SETRULES -> SETVAR | SETRULES SETVAR
SETVAR -> variable = EXPR
EXPR -> ETERM | EXPR + ETERM | EXPR - ETERM
ETERM -> value | variable | -variable | value*variable | variable*value
*/

%{
#include <stdio.h>
#include "advice/advice_rep.h"
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
%token T_IF T_ELIF T_ELSE T_AND
// operations
%token <leq_term> T_LEQ 
%token <lt_term> T_LT 
%token <gt_term> T_GT
%token <geq_term> T_GEQ 
%token T_LBRACE T_RBRACE
%token T_PLUS T_MINUS T_MULT T_ASSIGN 

%type <conds> conds;
%type <rule> rule;
%type <rule> if_tail;
%type <term> term;
%type <set_rules> set_rules;
%type <set_var> set_var;
%type <expr> expr;
%type <eterm> eterm;
%type <rules> rules;
// expect 1 shift/reduce conflict in top-level set_rules
//%expect 1
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
T_IF conds T_LBRACE set_rules T_RBRACE if_tail
{
    $$ = new IfRule($2, $4, $6);
}
;

if_tail: /* on nothing, return NULL */
{
    $$ = NULL;
}
| T_ELIF conds T_LBRACE set_rules T_RBRACE if_tail
{
    $$ = new IfRule($2, $4, $6);
} 
| T_ELSE T_LBRACE set_rules T_RBRACE
{
    $$ = $3;
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
    $$ = new LEQTerm($1,Number::toNetwork($3,$1));
}
| T_VARIABLE T_LEQ T_VARIABLE
{
    $$ = new LEQTerm($1,$3);
}
| T_VARIABLE T_LT T_NUMBER
{
    $$ = new LTTerm($1,Number::toNetwork($3,$1));
}
| T_VARIABLE T_LT T_VARIABLE
{
    $$ = new LTTerm($1,$3);
}
| T_VARIABLE T_GT T_NUMBER
{
    $$ = new GTTerm($1,Number::toNetwork($3,$1));
}
| T_VARIABLE T_GT T_VARIABLE
{
    $$ = new GTTerm($1,$3);
}
| T_VARIABLE T_GEQ T_NUMBER
{
    $$ = new GEQTerm($1,Number::toNetwork($3,$1));
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
    if (Variable::getType($1) == Variable::eSensor) {
        yyerror("cannot set sensor variables");
    }
    $$ = new SetVar($1,$3);
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

// Since the range of sensor variables can be different from network variables, combining
// them in assignment expressions causes problems and is therefore currently not supported,
// i.e. only network variables are allowed in such expressions.
eterm:
T_NUMBER
{
    // Since variables in assignment expressions are guaranteed to be non-sensor variables,
    // we can convert the number without checking its type.
    $$ = new Eterm(Number::toNetwork($1));
}
|
T_VARIABLE
{
    if (Variable::getType($1) == Variable::eSensor) {
        yyerror("sensor variables not allowed in assignment expressions");
    }
    $$ = new Eterm($1);
}
|
T_MINUS T_VARIABLE
{
    if (Variable::getType($2) == Variable::eSensor) {
        yyerror("sensor variables not allowed in assignment expressions");
    }
    $$ = new Eterm($2); $$->negate();
}
|
T_VARIABLE T_MULT T_NUMBER
{
    if (Variable::getType($1) == Variable::eSensor) {
        yyerror("sensor variables not allowed in assignment expressions");
    }
    $$ = new Eterm($1,$3);
}
|
T_NUMBER T_MULT T_VARIABLE
{
    if (Variable::getType($3) == Variable::eSensor) {
        yyerror("sensor variables not allowed in assignment expressions");
    }
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
    oss << s << ", at line number " << Counters::mLine;
    throw std::runtime_error(oss.str());
}

Rules* parsedRules;

Rules* yyParseAdvice(const char* advice, U32 numSensors, U32 numActions, Agent::Type type,
                     const FeatureVectorInfo& sensorBoundsNetwork, const FeatureVectorInfo& sensorBoundsAdvice) {
    Variable::mNumSensors = numSensors;
    Variable::mNumActions = numActions;
    Agent::mType = type;
    Number::mSensorBoundsNetwork = sensorBoundsNetwork;
    Number::mSensorBoundsAdvice = sensorBoundsAdvice;
    Counters::mLine = 1;
    YY_BUFFER_STATE parse_buffer = yy_scan_string (advice);
    parsedRules = NULL;
    yyparse();
    printf("\n");
    yy_delete_buffer(parse_buffer);
    return parsedRules;
}

Rules* yyParseAdvice(const char* advice, U32 numSensors, U32 numActions, Agent::Type type) {
    return yyParseAdvice(advice, numSensors, numActions, type, FeatureVectorInfo(), FeatureVectorInfo());
}
