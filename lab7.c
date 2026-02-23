/*
 * Recursive Descent Parser in C
 *
 * Grammar:
 *   Program      -> main () { declarations statement_list }
 *   Declarations -> data-type identifier-list; declarations | e
 *   data-type    -> int | char
 *   identifier-list -> id | id, identifier-list | id[number], identifier-list | id[number]
 *   statement_list -> statement statement_list | e
 *   statement    -> assign_stat; | decision_stat | looping_stat
 *   assign_stat  -> id = expn
 *   expn         -> simple-expn eprime
 *   eprime       -> relop simple-expn | e
 *   simple-expn  -> term seprime
 *   seprime      -> addop term seprime | e
 *   term         -> factor tprime
 *   tprime       -> mulop factor tprime | e
 *   factor       -> id | num
 *   decision_stat-> if ( expn ) { statement_list } dprime
 *   dprime       -> else { statement_list } | e
 *   looping_stat -> while ( expn ) { statement_list }
 *                 | for ( assign_stat ; expn ; assign_stat ) { statement_list }
 *   relop        -> == | != | <= | >= | > | <
 *   addop        -> + | -
 *   mulop        -> * | / | %
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ─────────────────────────────────────────────
   TOKEN TYPES
   ───────────────────────────────────────────── */
typedef enum {
    TOK_MAIN, TOK_INT, TOK_CHAR, TOK_IF, TOK_ELSE, TOK_WHILE, TOK_FOR,
    TOK_ID, TOK_NUMBER,
    TOK_ASSIGN,                        /* =  */
    TOK_RELOP,                         /* == != <= >= > < */
    TOK_ADDOP,                         /* + - */
    TOK_MULOP,                         /* * / % */
    TOK_LPAREN, TOK_RPAREN,            /* ( ) */
    TOK_LBRACE, TOK_RBRACE,            /* { } */
    TOK_LBRACKET, TOK_RBRACKET,        /* [ ] */
    TOK_SEMICOLON,                     /* ; */
    TOK_COMMA,                         /* , */
    TOK_EOF,
    TOK_UNKNOWN
} TokenType;

/* ─────────────────────────────────────────────
   TOKEN STRUCTURE
   ───────────────────────────────────────────── */
#define MAX_VAL 64

typedef struct {
    TokenType type;
    char      value[MAX_VAL];
    int       row;
    int       col;
} Token;

/* ─────────────────────────────────────────────
   LEXER STATE
   ───────────────────────────────────────────── */
#define MAX_TOKENS 4096

static Token  tokens[MAX_TOKENS];
static int    token_count = 0;
static int    token_pos   = 0;

/* ─────────────────────────────────────────────
   LEXER
   ───────────────────────────────────────────── */
static void add_token(TokenType type, const char *val, int row, int col) {
    if (token_count >= MAX_TOKENS) {
        fprintf(stderr, "Too many tokens\n");
        exit(1);
    }
    tokens[token_count].type = type;
    strncpy(tokens[token_count].value, val, MAX_VAL - 1);
    tokens[token_count].value[MAX_VAL - 1] = '\0';
    tokens[token_count].row  = row;
    tokens[token_count].col  = col;
    token_count++;
}

static TokenType keyword_type(const char *word) {
    if (strcmp(word, "main")  == 0) return TOK_MAIN;
    if (strcmp(word, "int")   == 0) return TOK_INT;
    if (strcmp(word, "char")  == 0) return TOK_CHAR;
    if (strcmp(word, "if")    == 0) return TOK_IF;
    if (strcmp(word, "else")  == 0) return TOK_ELSE;
    if (strcmp(word, "while") == 0) return TOK_WHILE;
    if (strcmp(word, "for")   == 0) return TOK_FOR;
    return TOK_ID;
}

/* Returns 1 on success, 0 on lex error */
static int tokenize(const char *src) {
    int i   = 0;
    int row = 1;
    int col = 1;
    int len = (int)strlen(src);

    while (i < len) {
        /* Skip whitespace */
        if (src[i] == '\n') { row++; col = 1; i++; continue; }
        if (src[i] == '\r' || src[i] == ' ' || src[i] == '\t') { col++; i++; continue; }

        int tok_col = col;

        /* Identifiers / keywords */
        if (isalpha((unsigned char)src[i]) || src[i] == '_') {
            char buf[MAX_VAL];
            int  j = 0;
            while (i < len && (isalnum((unsigned char)src[i]) || src[i] == '_')) {
                if (j < MAX_VAL - 1) buf[j++] = src[i];
                i++; col++;
            }
            buf[j] = '\0';
            add_token(keyword_type(buf), buf, row, tok_col);
            continue;
        }

        /* Numbers */
        if (isdigit((unsigned char)src[i])) {
            char buf[MAX_VAL];
            int  j = 0;
            while (i < len && isdigit((unsigned char)src[i])) {
                if (j < MAX_VAL - 1) buf[j++] = src[i];
                i++; col++;
            }
            buf[j] = '\0';
            add_token(TOK_NUMBER, buf, row, tok_col);
            continue;
        }

        /* Two-char operators */
        if (i + 1 < len) {
            char two[3] = { src[i], src[i+1], '\0' };
            if (strcmp(two, "==") == 0 || strcmp(two, "!=") == 0 ||
                strcmp(two, "<=") == 0 || strcmp(two, ">=") == 0) {
                add_token(TOK_RELOP, two, row, tok_col);
                i += 2; col += 2; continue;
            }
        }

        /* Single-char tokens */
        char one[2] = { src[i], '\0' };
        switch (src[i]) {
            case '=': add_token(TOK_ASSIGN,    one, row, tok_col); break;
            case '<': add_token(TOK_RELOP,     one, row, tok_col); break;
            case '>': add_token(TOK_RELOP,     one, row, tok_col); break;
            case '+': add_token(TOK_ADDOP,     one, row, tok_col); break;
            case '-': add_token(TOK_ADDOP,     one, row, tok_col); break;
            case '*': add_token(TOK_MULOP,     one, row, tok_col); break;
            case '/': add_token(TOK_MULOP,     one, row, tok_col); break;
            case '%': add_token(TOK_MULOP,     one, row, tok_col); break;
            case '(': add_token(TOK_LPAREN,    one, row, tok_col); break;
            case ')': add_token(TOK_RPAREN,    one, row, tok_col); break;
            case '{': add_token(TOK_LBRACE,    one, row, tok_col); break;
            case '}': add_token(TOK_RBRACE,    one, row, tok_col); break;
            case '[': add_token(TOK_LBRACKET,  one, row, tok_col); break;
            case ']': add_token(TOK_RBRACKET,  one, row, tok_col); break;
            case ';': add_token(TOK_SEMICOLON, one, row, tok_col); break;
            case ',': add_token(TOK_COMMA,     one, row, tok_col); break;
            default:
                fprintf(stderr, "✗ Unexpected character '%c' at row %d, col %d\n",
                        src[i], row, tok_col);
                return 0;
        }
        i++; col++;
    }

    add_token(TOK_EOF, "", row, col);
    return 1;
}

/* ─────────────────────────────────────────────
   PARSER UTILITIES
   ───────────────────────────────────────────── */
static int parse_error_flag = 0;

static Token *current(void) {
    return &tokens[token_pos];
}

static Token *advance(void) {
    Token *t = &tokens[token_pos];
    if (token_pos < token_count - 1) token_pos++;
    return t;
}

/* Print error and set flag; does NOT call exit so we can see one clean message */
static void parse_error(const char *expected, Token *got) {
    if (!parse_error_flag) {
        fprintf(stderr,
            "✗ Error at row %d, col %d: Expected '%s' but found '%s'\n",
            got->row, got->col, expected,
            (got->type == TOK_EOF) ? "<EOF>" : got->value);
        parse_error_flag = 1;
    }
}

/* Consume token of expected type, or report error */
static Token *expect(TokenType type, const char *expected_str) {
    Token *t = current();
    if (t->type != type) {
        parse_error(expected_str, t);
        return t;   /* still return so caller can try to continue */
    }
    return advance();
}

static int match(TokenType type) {
    return current()->type == type;
}

/* ─────────────────────────────────────────────
   FORWARD DECLARATIONS
   ───────────────────────────────────────────── */
static void parse_program(void);
static void parse_declarations(void);
static void parse_data_type(void);
static void parse_identifier_list(void);
static void parse_statement_list(void);
static void parse_statement(void);
static void parse_assign_stat(void);
static void parse_expn(void);
static void parse_eprime(void);
static void parse_simple_expn(void);
static void parse_seprime(void);
static void parse_term(void);
static void parse_tprime(void);
static void parse_factor(void);
static void parse_decision_stat(void);
static void parse_dprime(void);
static void parse_looping_stat(void);

/* ─────────────────────────────────────────────
   GRAMMAR RULES
   ───────────────────────────────────────────── */

/* Program -> main () { declarations statement_list } */
static void parse_program(void) {
    expect(TOK_MAIN,    "main");        if (parse_error_flag) return;
    expect(TOK_LPAREN,  "(");           if (parse_error_flag) return;
    expect(TOK_RPAREN,  ")");           if (parse_error_flag) return;
    expect(TOK_LBRACE,  "{");           if (parse_error_flag) return;
    parse_declarations();               if (parse_error_flag) return;
    parse_statement_list();             if (parse_error_flag) return;
    expect(TOK_RBRACE,  "}");           if (parse_error_flag) return;
    expect(TOK_EOF,     "end of file"); if (parse_error_flag) return;
    printf("✓ Parsing completed successfully — no syntax errors found.\n");
}

/* Declarations -> data-type identifier-list; declarations | e */
static void parse_declarations(void) {
    if (match(TOK_INT) || match(TOK_CHAR)) {
        parse_data_type();              if (parse_error_flag) return;
        parse_identifier_list();        if (parse_error_flag) return;
        expect(TOK_SEMICOLON, ";");     if (parse_error_flag) return;
        parse_declarations();
    }
    /* else epsilon */
}

/* data-type -> int | char */
static void parse_data_type(void) {
    Token *t = current();
    if (match(TOK_INT) || match(TOK_CHAR)) {
        advance();
    } else {
        parse_error("data-type ('int' or 'char')", t);
    }
}

/* identifier-list -> id[number], identifier-list | id[number] | id, identifier-list | id */
static void parse_identifier_list(void) {
    expect(TOK_ID, "identifier");       if (parse_error_flag) return;
    if (match(TOK_LBRACKET)) {
        advance();
        expect(TOK_NUMBER,   "array size (number)"); if (parse_error_flag) return;
        expect(TOK_RBRACKET, "]");                   if (parse_error_flag) return;
    }
    if (match(TOK_COMMA)) {
        advance();
        parse_identifier_list();
    }
}

/* statement_list -> statement statement_list | e */
static void parse_statement_list(void) {
    while (!parse_error_flag &&
           (match(TOK_ID) || match(TOK_IF) || match(TOK_WHILE) || match(TOK_FOR))) {
        parse_statement();
    }
}

/* statement -> assign_stat; | decision_stat | looping_stat */
static void parse_statement(void) {
    Token *t = current();
    if (t->type == TOK_ID) {
        parse_assign_stat();            if (parse_error_flag) return;
        expect(TOK_SEMICOLON, ";");
    } else if (t->type == TOK_IF) {
        parse_decision_stat();
    } else if (t->type == TOK_WHILE || t->type == TOK_FOR) {
        parse_looping_stat();
    } else {
        parse_error("statement (assignment, if, while, or for)", t);
    }
}

/* assign_stat -> id = expn */
static void parse_assign_stat(void) {
    expect(TOK_ID,     "identifier");   if (parse_error_flag) return;
    expect(TOK_ASSIGN, "=");            if (parse_error_flag) return;
    parse_expn();
}

/* expn -> simple-expn eprime */
static void parse_expn(void) {
    parse_simple_expn();                if (parse_error_flag) return;
    parse_eprime();
}

/* eprime -> relop simple-expn | e */
static void parse_eprime(void) {
    if (match(TOK_RELOP)) {
        advance();
        parse_simple_expn();
    }
    /* else epsilon */
}

/* simple-expn -> term seprime */
static void parse_simple_expn(void) {
    parse_term();                       if (parse_error_flag) return;
    parse_seprime();
}

/* seprime -> addop term seprime | e */
static void parse_seprime(void) {
    if (match(TOK_ADDOP)) {
        advance();
        parse_term();                   if (parse_error_flag) return;
        parse_seprime();
    }
    /* else epsilon */
}

/* term -> factor tprime */
static void parse_term(void) {
    parse_factor();                     if (parse_error_flag) return;
    parse_tprime();
}

/* tprime -> mulop factor tprime | e */
static void parse_tprime(void) {
    if (match(TOK_MULOP)) {
        advance();
        parse_factor();                 if (parse_error_flag) return;
        parse_tprime();
    }
    /* else epsilon */
}

/* factor -> id | num */
static void parse_factor(void) {
    Token *t = current();
    if (match(TOK_ID) || match(TOK_NUMBER)) {
        advance();
    } else {
        parse_error("identifier or number in expression", t);
    }
}

/* decision_stat -> if ( expn ) { statement_list } dprime */
static void parse_decision_stat(void) {
    expect(TOK_IF,     "if");           if (parse_error_flag) return;
    expect(TOK_LPAREN, "(");            if (parse_error_flag) return;
    parse_expn();                       if (parse_error_flag) return;
    expect(TOK_RPAREN, ")");            if (parse_error_flag) return;
    expect(TOK_LBRACE, "{");            if (parse_error_flag) return;
    parse_statement_list();             if (parse_error_flag) return;
    expect(TOK_RBRACE, "}");            if (parse_error_flag) return;
    parse_dprime();
}

/* dprime -> else { statement_list } | e */
static void parse_dprime(void) {
    if (match(TOK_ELSE)) {
        advance();
        expect(TOK_LBRACE, "{");        if (parse_error_flag) return;
        parse_statement_list();         if (parse_error_flag) return;
        expect(TOK_RBRACE, "}");
    }
    /* else epsilon */
}

/* looping_stat -> while ( expn ) { statement_list }
 *               | for ( assign_stat ; expn ; assign_stat ) { statement_list } */
static void parse_looping_stat(void) {
    Token *t = current();
    if (t->type == TOK_WHILE) {
        advance();
        expect(TOK_LPAREN, "(");        if (parse_error_flag) return;
        parse_expn();                   if (parse_error_flag) return;
        expect(TOK_RPAREN, ")");        if (parse_error_flag) return;
        expect(TOK_LBRACE, "{");        if (parse_error_flag) return;
        parse_statement_list();         if (parse_error_flag) return;
        expect(TOK_RBRACE, "}");

    } else if (t->type == TOK_FOR) {
        advance();
        expect(TOK_LPAREN,    "(");     if (parse_error_flag) return;
        parse_assign_stat();            if (parse_error_flag) return;
        expect(TOK_SEMICOLON, ";");     if (parse_error_flag) return;
        parse_expn();                   if (parse_error_flag) return;
        expect(TOK_SEMICOLON, ";");     if (parse_error_flag) return;
        parse_assign_stat();            if (parse_error_flag) return;
        expect(TOK_RPAREN,    ")");     if (parse_error_flag) return;
        expect(TOK_LBRACE,    "{");     if (parse_error_flag) return;
        parse_statement_list();         if (parse_error_flag) return;
        expect(TOK_RBRACE,    "}");

    } else {
        parse_error("'while' or 'for'", t);
    }
}

/* ─────────────────────────────────────────────
   RUN ONE TEST
   ───────────────────────────────────────────── */
static void run_test(const char *name, const char *source) {
    /* Print separator and source */
    printf("────────────────────────────────────────────────────────────\n");
    printf("TEST: %s\n", name);
    printf("────────────────────────────────────────────────────────────\n");
    printf("Source:\n");

    /* Print source with line numbers */
    int line = 1;
    printf("  %3d: ", line);
    for (int i = 0; source[i]; i++) {
        if (source[i] == '\n') {
            printf("\n");
            line++;
            if (source[i+1]) printf("  %3d: ", line);
        } else {
            putchar(source[i]);
        }
    }
    printf("\n\n");

    /* Reset global state */
    token_count      = 0;
    token_pos        = 0;
    parse_error_flag = 0;

    if (tokenize(source)) {
        parse_program();
    }
    printf("\n");
}

/* ─────────────────────────────────────────────
   MAIN
   ───────────────────────────────────────────── */
int main(int argc, char *argv[]) {

    /* ── If a filename is given, parse that file ── */
    if (argc > 1) {
        FILE *fp = fopen(argv[1], "r");
        if (!fp) { fprintf(stderr, "Cannot open file: %s\n", argv[1]); return 1; }
        fseek(fp, 0, SEEK_END);
        long sz = ftell(fp);
        rewind(fp);
        char *buf = malloc(sz + 1);
        if (!buf) { fprintf(stderr, "Out of memory\n"); fclose(fp); return 1; }
        fread(buf, 1, sz, fp);
        buf[sz] = '\0';
        fclose(fp);

        token_count = 0; token_pos = 0; parse_error_flag = 0;
        if (tokenize(buf)) parse_program();
        free(buf);
        return 0;
    }

    /* ── Built-in test cases ── */

    run_test("Valid Program",
        "main () {\n"
        "    int x, y, arr[10];\n"
        "    char c;\n"
        "    x = 5 + 3;\n"
        "    y = x * 2 - 1;\n"
        "    if (x > y) {\n"
        "        x = x + 1;\n"
        "    } else {\n"
        "        y = y - 1;\n"
        "    }\n"
        "    while (x != 0) {\n"
        "        x = x - 1;\n"
        "    }\n"
        "    for (x = 0 ; x < 10 ; x = x + 1) {\n"
        "        y = arr;\n"
        "    }\n"
        "}"
    );

    run_test("Missing ';' after declaration",
        "main () {\n"
        "    int x\n"
        "    x = 5;\n"
        "}"
    );

    run_test("Missing '=' in assignment",
        "main () {\n"
        "    int x;\n"
        "    x 5;\n"
        "}"
    );

    run_test("Missing '}' at end",
        "main () {\n"
        "    int x;\n"
        "    x = 10;\n"
    );

    run_test("Invalid token in expression",
        "main () {\n"
        "    int x;\n"
        "    x = 5 + ;\n"
        "}"
    );

    run_test("Missing '(' after if",
        "main () {\n"
        "    int x;\n"
        "    x = 3;\n"
        "    if x > 0 {\n"
        "        x = 0;\n"
        "    }\n"
        "}"
    );

    run_test("Missing ';' in for-loop",
        "main () {\n"
        "    int x;\n"
        "    for (x = 0 , x < 10 ; x = x + 1) {\n"
        "        x = x + 2;\n"
        "    }\n"
        "}"
    );

    run_test("Invalid data-type 'float'",
        "main () {\n"
        "    float x;\n"
        "    x = 3;\n"
        "}"
    );

    return 0;
}
