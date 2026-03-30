#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Assumed External Variables and Functions ---
// These would be defined in your lexical analyzer (lexer)

char lookahead[256]; // Holds the current token string (e.g., "if", "(", "id")

// Advances to the next token from the input
void advance() {
    // Implementation depends on your specific lexer
    // e.g., strcpy(lookahead, getNextToken());
}

// Error reporting function
void error(const char* expected) {
    printf("Syntax Error: Expected '%s', but found '%s'\n", expected, lookahead);
    exit(1); // Standard panic-mode exit for basic parsers
}

// Helper function to match terminal symbols
void match(const char* expectedToken) {
    if (strcmp(lookahead, expectedToken) == 0) {
        advance();
    } else {
        error(expectedToken);
    }
}

// Forward declarations for non-terminals defined elsewhere in grammar 7.1
void assign_stat();
void expn();
void statement_list();
void dprime();
void decision_stat();


// --- Recursive Descent Parsing Functions ---

/* * Rule: dprime -> else {statement_list} | ε 
 */
void dprime() {
    // If the next token is 'else', process the else block
    if (strcmp(lookahead, "else") == 0) {
        match("else");
        match("{");
        statement_list();
        match("}");
    }
    // Epsilon (ε) transition: 
    // If lookahead is not "else", we do nothing and return, 
    // allowing the parser to naturally fall back up the recursive chain.
}

/* * Rule: decision-stat -> if (expn) {statement_list} dprime 
 */
void decision_stat() {
    match("if");
    match("(");
    expn();
    match(")");
    match("{");
    statement_list();
    match("}");
    dprime();
}

/* * Rule: statement -> assign-stat; | decision_stat 
 */
void statement() {
    // We use the FIRST sets to decide which production to use.
    // FIRST(decision_stat) is obviously "if".
    if (strcmp(lookahead, "if") == 0) {
        decision_stat();
    } 
    // Assuming assign-stat starts with an identifier, we default to it here.
    // In a complete parser, you might specifically check for 'id' or other terminals.
    else {
        assign_stat();
        match(";");
    }
}



