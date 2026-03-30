
%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Required for pow()

// Declare YYSTYPE as double to handle floating-point calculations
#define YYSTYPE double

// Function prototypes
int yylex();
void yyerror(const char *s);
%}

/* Token definition */
%token NUM

%%

/* input -> input line | epsilon */
input:
    /* empty */
    | input line
    ;

/* line -> '\n' | exp '\n' */
line:
    '\n'
    | exp '\n' { printf("Result: %g\n", $1); }
    ;

/* exp -> num | exp exp '+' | exp exp '-' | ... | exp 'n' */
exp:
    NUM            { $$ = $1; }
    | exp exp '+'  { $$ = $1 + $2; }
    | exp exp '-'  { $$ = $1 - $2; }if (a > b) { x = 1; } else { x = 0; }
    | exp exp '*'  { $$ = $1 * $2; }
    | exp exp '/'  { 
        if ($2 == 0.0) {
            yyerror("Divide by zero error");
            $$ = 0; // Prevent crash, assign default value
        } else {
            $$ = $1 / $2; 
        }
    }
    | exp exp '^'  { $$ = pow($1, $2); }
    | exp 'n'      { $$ = -$1; } /* 'n' is the unary minus operator */
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    printf("Enter a postfix expression (e.g., '3 4 +', '5 2 n *', '2 3 ^'): \n");
    printf("Press Ctrl+C to exit.\n\n");
    yyparse();
    return 0;
}
