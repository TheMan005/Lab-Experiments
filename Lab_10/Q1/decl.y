%{
#include <stdio.h>
#include <stdlib.h>

// Function prototypes
void yyerror(const char *s);
int yylex();
%}

/* Token definitions matching the Flex file */
%token TYPE ID

%%

/* Starting rule */
program: 
    declaration { printf("Result: Valid declaration statement.\n"); exit(0); }
    ;

/* A declaration is a TYPE followed by a list of IDs, ending with a semicolon */
declaration: 
    TYPE id_list ';'
    ;

/* An id_list is either a single ID, or an ID followed by a comma and another id_list */
id_list: 
    ID
    | ID ',' id_list
    ;

%%

void yyerror(const char *s) {
    printf("Result: Invalid declaration statement.\n");
    exit(1);
}

int main() {
    printf("Enter a declaration statement (e.g., int a, b;): \n");
    yyparse();
    return 0;
}
