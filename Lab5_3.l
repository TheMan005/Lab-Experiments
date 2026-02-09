%{
#include <stdio.h>

int positive_int = 0;
int negative_int = 0;
int positive_float = 0;
int negative_float = 0;
%}

%%

\+?[0-9]+                { positive_int++; printf("Positive Integer: %s\n", yytext); }
-[0-9]+                  { negative_int++; printf("Negative Integer: %s\n", yytext); }
\+?[0-9]+\.[0-9]+        { positive_float++; printf("Positive Float: %s\n", yytext); }
-[0-9]+\.[0-9]+          { negative_float++; printf("Negative Float: %s\n", yytext); }
[ \t\n]+                 { /* Ignore whitespace */ }
.                        { /* Ignore other characters */ }

%%

int yywrap() {
    return 1;
}

int main() {
    printf("Enter numbers (Press Ctrl+D or Ctrl+Z to end input):\n");
    printf("Examples: 10, -5, 3.14, -2.5, +100, +45.67\n\n");
    
    yylex();
    
    printf("\n=== Results ===\n");
    printf("Positive Integers: %d\n", positive_int);
    printf("Negative Integers: %d\n", negative_int);
    printf("Positive Floating-Point Numbers: %d\n", positive_float);
    printf("Negative Floating-Point Numbers: %d\n", negative_float);
    printf("Total Numbers: %d\n", positive_int + negative_int + positive_float + negative_float);
    
    return 0;
}
