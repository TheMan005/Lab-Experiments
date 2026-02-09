%{
#include <stdio.h>

int word_count = 0;
int char_count = 0;
int blank_count = 0;
int line_count = 0;
%}

%%

[a-zA-Z]+       { word_count++; char_count += yyleng; }
[ \t]           { blank_count++; }
\n              { line_count++; }
.               { char_count++; }

%%

int yywrap() {
    return 1;
}

int main() {
    printf("Enter text (Press Ctrl+D or Ctrl+Z to end input):\n");
    yylex();
    
    printf("\n=== Statistics ===\n");
    printf("Number of words: %d\n", word_count);
    printf("Number of characters: %d\n", char_count);
    printf("Number of blanks: %d\n", blank_count);
    printf("Number of lines: %d\n", line_count);
    
    return 0;
}
