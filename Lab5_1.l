%{
#include <stdio.h>

int vowel_count = 0;
int consonant_count = 0;
%}

%%

[aeiouAEIOU]    { vowel_count++; }
[a-zA-Z]        { consonant_count++; }
.               { /* Ignore other characters */ }
\n              { /* Ignore newlines */ }

%%

int yywrap() {
    return 1;
}

int main() {
    printf("Enter text (Press Ctrl+D or Ctrl+Z to end input):\n");
    yylex();
    
    printf("\n=== Results ===\n");
    printf("Number of vowels: %d\n", vowel_count);
    printf("Number of consonants: %d\n", consonant_count);
    printf("Total alphabetic characters: %d\n", vowel_count + consonant_count);
    
    return 0;
}
