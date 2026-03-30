%{
#include <stdio.h>
#include <stdlib.h>

// Function prototypes
void yyerror(const char *s);
int yylex();
%}

/* Token definitions matching the Flex file */
%token IF ELSE ID NUMBER RELOP

%%

/* Starting rule */
program:
    decision_stat { printf("Result: Valid decision making statement.\n"); exit(0); }
    ;

/* Main decision statement rule */
decision_stat:
    IF '(' expn ')' '{' statement_list '}' dprime
    ;

/* Optional else block (epsilon production handled implicitly by the empty rule) */
dprime:
    ELSE '{' statement_list '}'
    | /* empty */
    ;

/* A simple expression for the condition (e.g., a > b, x == 5, or just a variable/number) */
expn:
    ID RELOP ID
    | ID RELOP NUMBER
    | NUMBER RELOP ID
    | NUMBER RELOP NUMBER
    | ID
    | NUMBER
    ;

/* A list of statements inside the braces */
statement_list:
    statement statement_list
    | /* empty */
    ;

/* A statement can be a simple assignment or another nested decision statement */
statement:
    ID '=' expn ';'
    | decision_stat
    ;

%%

void yyerror(const char *s) {
    printf("Result: Invalid decision making statement.\n");
    exit(1);
}

int main() {
    printf("Enter a decision making statement:\n");
    printf("(e.g., if (a > b) { x = 1; } else { x = 0; } )\n\n");
    yyparse();
    return 0;
}
