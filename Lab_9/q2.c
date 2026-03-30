#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Assumed External Variables and Core Functions ---
char lookahead[256]; 

void advance() {
    // e.g., strcpy(lookahead, getNextToken());
}

void error(const char* expected) {
    printf("Syntax Error: Expected '%s', but found '%s'\n", expected, lookahead);
    exit(1); 
}

void match(const char* expectedToken) {
    if (strcmp(lookahead, expectedToken) == 0) {
        advance();
    } else {
        error(expectedToken);
    }
}

// Forward declarations
void assign_stat();
void expn();
void statement_list();
void decision_stat();


// --- New and Updated Parsing Functions ---

/* * Rule: looping-stat -> while (expn) {statement_list} 
 * | for (assign_stat ; expn ; assign_stat) {statement_list}
 */
void looping_stat() {
    if (strcmp(lookahead, "while") == 0) {
        match("while");
        match("(");
        expn();
        match(")");
        match("{");
        statement_list();
        match("}");
    } 
    else if (strcmp(lookahead, "for") == 0) {
        match("for");
        match("(");
        assign_stat();
        match(";");
        expn();
        match(";");
        assign_stat();
        match(")");
        match("{");
        statement_list();
        match("}");
    } 
    else {
        // If we enter looping_stat but don't see 'while' or 'for', it's an error
        error("while or for");
    }
}

/* * Rule: statement -> assign-stat; | decision_stat | looping-stat 
 */
void statement() {
    // Check FIRST(decision_stat)
    if (strcmp(lookahead, "if") == 0) {
        decision_stat();
    } 
    // Check FIRST(looping-stat)
    else if (strcmp(lookahead, "while") == 0 || strcmp(lookahead, "for") == 0) {
        looping_stat();
    } 
    // Default to assign-stat
    else {
        assign_stat();
        match(";");
    }
}

