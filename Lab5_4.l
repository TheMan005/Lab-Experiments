%{
#include <stdio.h>

int scanf_count = 0;
int printf_count = 0;
FILE *output_file;
%}

%%

"scanf"     { 
                scanf_count++; 
                fprintf(output_file, "READ");
                fprintf(yyout, "READ");
            }

"printf"    { 
                printf_count++; 
                fprintf(output_file, "WRITE");
                fprintf(yyout, "WRITE");
            }

.           { fprintf(output_file, "%s", yytext); }

\n          { fprintf(output_file, "\n"); }

%%

int yywrap() {
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file.c> [output_file.c]\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) {
        printf("Error: Cannot open input file %s\n", argv[1]);
        return 1;
    }

    char output_filename[256];
    if (argc >= 3) {
        strcpy(output_filename, argv[2]);
    } else {
        strcpy(output_filename, "output.c");
    }

    output_file = fopen(output_filename, "w");
    if (!output_file) {
        printf("Error: Cannot create output file %s\n", output_filename);
        fclose(input_file);
        return 1;
    }

    yyin = input_file;
    
    printf("Processing file: %s\n", argv[1]);
    printf("Output file: %s\n\n", output_filename);
    
    yylex();
    
    fclose(input_file);
    fclose(output_file);
    
    printf("\n=== Results ===\n");
    printf("Number of scanf statements: %d\n", scanf_count);
    printf("Number of printf statements: %d\n", printf_count);
    printf("Total replacements: %d\n", scanf_count + printf_count);
    printf("\nReplacement complete! Check %s\n", output_filename);
    
    return 0;
}
