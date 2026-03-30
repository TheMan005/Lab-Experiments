%{
#include "postfix.tab.h"
#include <stdlib.h>
%}

%%

[0-9]+(\.[0-9]+)? { 
    yylval = atof(yytext); // Convert string to double and store in yylval
    return NUM; 
}

[ \t]+      { /* Ignore spaces and tabs */ }

\n          { return '\n'; }

.           { return yytext[0]; /* Return operators like '+', '-', '*', '/', '^', 'n' */ }

%%

int yywrap() {
    return 1;
}

