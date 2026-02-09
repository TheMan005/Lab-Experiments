#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 50

typedef struct {
    char name[MAX];
    int line;
} Symbol;

Symbol global[MAX], local[MAX];
int g_count = 0, l_count = 0, pos = 0, line = 1, scope = 0;
char input[1000];

char* keywords[] = {"int", "float", "char", "if", "else", "while", "for", "return", "void"};

int isKeyword(char* str) {
    for (int i = 0; i < 9; i++)
        if (strcmp(str, keywords[i]) == 0) return 1;
    return 0;
}

void addSymbol(char* name) {
    if (scope == 0) {
        for (int i = 0; i < g_count; i++)
            if (strcmp(global[i].name, name) == 0) return;
        strcpy(global[g_count].name, name);
        global[g_count++].line = line;
    } else {
        for (int i = 0; i < l_count; i++)
            if (strcmp(local[i].name, name) == 0) return;
        strcpy(local[l_count].name, name);
        local[l_count++].line = line;
    }
}

char* getNextToken() {
    static char token[MAX];
    
    // Skip whitespace
    while (input[pos] && isspace(input[pos])) {
        if (input[pos] == '\n') line++;
        pos++;
    }
    
    if (!input[pos]) return NULL;
    
    int i = 0;
    
    // Identifier or Keyword
    if (isalpha(input[pos]) || input[pos] == '_') {
        while (isalnum(input[pos]) || input[pos] == '_')
            token[i++] = input[pos++];
        token[i] = '\0';
        
        if (!isKeyword(token)) addSymbol(token);
        return token;
    }
    
    // Number
    if (isdigit(input[pos])) {
        while (isdigit(input[pos]) || input[pos] == '.')
            token[i++] = input[pos++];
        token[i] = '\0';
        return token;
    }
    
    // Operators
    if (strchr("+-*/%=<>!&|", input[pos])) {
        token[0] = input[pos++];
        if (strchr("=&|", input[pos]) && input[pos] == token[0])
            token[1] = input[pos++], token[2] = '\0';
        else
            token[1] = '\0';
        return token;
    }
    
    // Delimiters
    if (strchr("(){}[];,", input[pos])) {
        if (input[pos] == '{') scope = 1;
        if (input[pos] == '}') scope = 0;
        token[0] = input[pos++];
        token[1] = '\0';
        return token;
    }
    
    token[0] = input[pos++];
    token[1] = '\0';
    return token;
}

void displayTable(Symbol* table, int count, char* name) {
    printf("\n=== %s SYMBOL TABLE ===\n", name);
    printf("%-20s %s\n", "Name", "Line");
    printf("------------------------\n");
    if (count == 0) printf("(Empty)\n");
    for (int i = 0; i < count; i++)
        printf("%-20s %d\n", table[i].name, table[i].line);
}

int main() {
    char* code = 
        "int main() {\n"
        "    int x, y;\n"
        "    float result;\n"
        "    x = 10;\n"
        "    if (x > 5) {\n"
        "        int temp;\n"
        "    }\n"
        "}\n"
        "int globalVar;\n";
    
    strcpy(input, code);
    printf("Source Code:\n%s\n", code);
    
    printf("\n=== TOKENS ===\n");
    char* token;
    while ((token = getNextToken()) != NULL)
        printf("%s ", token);
    
    displayTable(global, g_count, "GLOBAL");
    displayTable(local, l_count, "LOCAL");
    
    return 0;
}
