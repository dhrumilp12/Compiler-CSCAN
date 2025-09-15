/* 
Sources / Tools I used during programming

1. I used github copilot to help me write some parts of the code. 
    For example, I used it to help me write the functions for the string buffer (sbuf) 
    and some parts of the scan_string function. I made sure to review and understand the code it generated. 
2. I used ChatGPT “Study Mode” for learning/explanations while I wrote the code.
3. I used google search to find documentation on C programming.

*/





#include "scan.h" 
#include <ctype.h> // isspace, isalpha, isalnum, isdigit

// fixed lexemes for simple tokens
#define X(FIRST, SECOND) [FIRST] = SECOND,
static const char *FIXED_LEXEMES[] = {
    TOKEN_VALUES
};
#undef X

// scanner state 
int cur_char = 0;      // current character
int has_cur_char = 0; // whether cur_char is valid (1) or needs to be read (0)
int cur_line = 1;      // current line number 
int cur_column = 0;    // current column number 

// simple string buffer for building lexemes
typedef struct {
    char *buf; // dynamically allocated buffer
    size_t len, cap;
} sbuf;

// keyword lookup table
typedef struct { const char *kw; TokenType ty; } kw_entry;

// list of keywords
const kw_entry KW[] = {
    {"char", CHAR},{"int", INT}, {"float", FLOAT}, {"double", DOUBLE},
    {"if", IF},{"else", ELSE}, {"while", WHILE}, {"do", DO}, {"for", FOR},
    {"return", RETURN}, {"break", BREAK}, {"continue", CONTINUE}, {"goto", GOTO} 
};

// function prototypes
int next_char(void); // get next character, updating line/column
void push_char(int c); // push back a character
int peek_char(void); // peek at next character without consuming
void skip_spaces_and_comments(void); // skip whitespace and comments
int decode_escape_sequence(int c); // decode an escape sequence, given the char after '\'
void scanner_error(const char *msg, int offending); // report a scanner error
int is_ident_start(int c) { return isalpha(c) || c == '_'; } // whether c can start an identifier
int is_ident_part(int c) { return isalnum(c) || c == '_'; }  // whether c can be part of an identifier
void sbuf_init(sbuf *b); // initialize a string buffer
void sbuf_push(sbuf *b, char c); // append a character to a string buffer
char *sbuf_steal(sbuf *b); // steal the contents of a string buffer
Token scan_identifier_or_keyword(int first_char); // scan an identifier or keyword
Token scan_number(int first_char); // scan a number (integer or real)
Token scan_string(void); // scan a string literal
Token scan_operator_or_delim(int first); // scan an operator or delimiter
Token make_simple(TokenType ty); // make a simple token with no lexeme

// initialize the scanner state
void init_scanner() {
	cur_char = 0;
    has_cur_char = 0;
	cur_line = 1;
	cur_column = 0;
}

// helper to create a simple token with lexeme
void scanner_error(const char *msg, int offending){
	if (offending == EOF) fprintf(stderr, "scanner error: %s at EOF\n", msg);
	else if (offending == '\n') fprintf(stderr, "scanner error: %s at newline\n", msg);
	else if (offending == '\t') fprintf(stderr, "scanner error: %s at tab\n", msg);
	else if (offending == '\r') fprintf(stderr, "scanner error: %s at carriage return\n", msg);
	else if (offending == '\v') fprintf(stderr, "scanner error: %s at vertical tab\n", msg);
	else if (offending == '\f') fprintf(stderr, "scanner error: %s at form feed\n", msg);
	else if (offending == '\a') fprintf(stderr, "scanner error: %s at bell\n", msg);
	else if (offending == '\b') fprintf(stderr, "scanner error: %s at backspace\n", msg);
	else if (offending == '\\') fprintf(stderr, "scanner error: %s at backslash\n", msg);
	else if (offending == '\"') fprintf(stderr, "scanner error: %s at double quote\n", msg);
	else if (offending == '\'') fprintf(stderr, "scanner error: %s at single quote\n", msg);
	else if (offending < 32 || offending > 126) fprintf(stderr, "scanner error: %s at non-printable char 0x%02x\n", msg, offending & 0xFF);
	else fprintf(stderr, "scanner error: %s at character '%c'\n", msg, offending);
	exit(1);
}


// string buffer functions
void sbuf_init(sbuf *b) {
    b->cap = 64; // initial capacity
    b->len = 0;  // initial length
    b->buf = (char*)malloc(b->cap);
    if (!b->buf) { perror("malloc"); exit(1); }
    b->buf[0] = '\0';
}

// append a character to the string buffer
void sbuf_push(sbuf *b, char c) {
    if (b->len + 1 >= b->cap) {
        b->cap *= 2; // double capacity
        char *nb = (char*)realloc(b->buf, b->cap);
        if (!nb) { perror("realloc"); exit(1); }
        b->buf = nb;
    }
    b->buf[b->len++] = c;
    b->buf[b->len] = '\0';
}

// steal the buffer contents and reset the sbuf
char *sbuf_steal(sbuf *b) {
    char *s = b->buf; // take ownership of the buffer
    b->buf = NULL; b->len = b->cap = 0;
    return s;
}

// read the next character from input, updating line and column numbers
int next_char(void) {
	int c;
	if (has_cur_char) {
		c = cur_char;
		has_cur_char = 0;
	} else {
        c = fgetc(stdin); // Read next character from standard input
	}
	if (c == '\n') {
		cur_line++;
		cur_column = 0; // will increment to 1 when next non-newline consumed
	} else if (c != EOF) {
		cur_column++;
	}
	return c;
}

// push back a character to be read again
void push_char(int c) {
	if( c == EOF) return; // ignore
	has_cur_char = 1;
	cur_char = c;
}

// peek at the next character without consuming it
int peek_char(void) {
	int c = next_char();
	push_char(c);
	return c;
}

// skip whitespace and comments
void skip_spaces_and_comments(void) {
    for (;;) {
        // 1) skip whitespace
        int c = peek_char();
        while (isspace(c)) {
            (void)next_char();
            c = peek_char();
        }

        // 2) handle comments
        if (c == '/') {
            (void)next_char();            // consume '/'
            int d = peek_char();

            if (d == '/') {               // line comment
                (void)next_char();        // consume second '/'
                int e;
                while ((e = next_char()) != EOF && e != '\n') { /* skip */ }
                continue;                 // loop to skip following spaces/comments
            }

            if (d == '*') {               // /* block comment */
                (void)next_char();        // consume '*'
                int prev = 0, e, closed = 0;
                while ((e = next_char()) != EOF) {
                    if (prev == '*' && e == '/') { closed = 1; break; }
                    prev = e;
                }
                if (!closed) {
                    scanner_error("unterminated comment", EOF);
                }
                continue;                 // loop to skip following spaces/comments
            }

            // not a comment → put '/' back for tokenization
            push_char('/');
        }

        // neither whitespace nor comment: stop
        return;
    }
}

int decode_escape_sequence(int c) {
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'v': return '\v';
        case 'f': return '\f';
        case 'a': return '\a';
        case 'b': return '\b';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"': return '\"';
        case '?': return '\?'; 
        case '0': return '\0'; // null character
        default:
            scanner_error("invalid escape sequence", c);
            return c; // never reached
    }
}


// lookup a keyword; returns NULL_TOKEN if not found
TokenType keyword_lookup(const char *s) {
    for (size_t i = 0; i < sizeof(KW)/sizeof(KW[0]); ++i) {
        if (strcmp(s, KW[i].kw) == 0) return KW[i].ty;
    }
    return NULL_TOKEN; // not a keyword
}

// scan an identifier or keyword, given the first character already read
Token scan_identifier_or_keyword(int first_char) {
    sbuf b; sbuf_init(&b);
    sbuf_push(&b, (char)first_char);

    for (;;) {
        int c = peek_char();
        if (!is_ident_part(c)) break;
        (void)next_char();
        sbuf_push(&b, (char)c);
    }

    char *lex = sbuf_steal(&b);
    TokenType kt = keyword_lookup(lex);

    Token t;
    t.line = cur_line;
    t.column = cur_column;
    t.int_literal = 0;

    if (kt != NULL_TOKEN) {
        // keyword: no need to keep a copy of the lexeme for printing
        free(lex);
        t.type = kt;
        t.lexeme = (char*)FIXED_LEXEMES[kt];
    } else {
        t.type = IDENTIFIER;
        t.lexeme = lex; // print_token uses this
    }
    return t;
}

// scan a number (integer or real), given the first character already read
Token scan_number(int first_char) {
    // build the raw text
    sbuf b; sbuf_init(&b);
    sbuf_push(&b, (char)first_char);

    // consume more digits
    for (;;) {
        int c = peek_char();
        if (!isdigit(c)) break;
        (void)next_char();
        sbuf_push(&b, (char)c);
    }

    // optional fractional part: only if we see '.' followed by a digit
    int is_real = 0;
    int c1 = peek_char();
    if (c1 == '.') {
        (void)next_char();                 // tentatively take '.'
        int c2 = peek_char();
        if (isdigit(c2)) {
            is_real = 1;
            sbuf_push(&b, '.');
            // read digits after '.'
            while (isdigit(peek_char())) {
                int d = next_char();
                sbuf_push(&b, (char)d);
            }
        } else {
            // it wasn't a real number; put the '.' back and keep integer
            push_char('.');
        }
    }

    char *text = sbuf_steal(&b);

    Token t = {0};
    t.line = cur_line;
    t.column = cur_column;
    t.lexeme = NULL;

    // semantic checking
    if (is_real) {
        char *endp = NULL;
        double val = strtod(text, &endp);
        if (endp == text) {
            free(text);
            scanner_error("invalid real literal", first_char);
        }
        t.type = REAL_LITERAL;
        t.float_literal = val;
        free(text);

    // integer literal 
    } else {
        char *endp = NULL;
        long long val = strtoll(text, &endp, 10);
        if (endp == text) {
            free(text);
            scanner_error("invalid integer literal", first_char);
        }
        t.type = INT_LITERAL;
        t.int_literal = val;
        free(text);
    }
    return t;
}

// scan a string literal (called after the opening '"' has been consumed)
Token scan_string(void) {
    // sbuf for building the string contents
    sbuf b; sbuf_init(&b);

    for (;;) { // infinite loop, breaks on closing quote or error
        int c = next_char();
        if (c == EOF || c == '\n') {
            scanner_error("unterminated string literal", c);
        }

        if (c == '"') {
            // closing quote — done
            break;
        }

        if (c == '\\') {
            // keep backslash and the next char verbatim (if any)
            int d = next_char();
        if (d == EOF) {
            scanner_error("unterminated string literal after backslash", d);
        }

        // Line continuation: backslash immediately followed by newline
        if (d == '\n') {
            // swallow the newline (continue the string on next line)
            continue;
        }

        // Windows CRLF support for continuation: "\\\r\n"
        if (d == '\r') {
            int e = peek_char();
            if (e == '\n') { (void)next_char(); continue; } // swallow \r\n
            scanner_error("backslash must be immediately before newline in continued string", d);
        }

        // If there is whitespace after backslash that is not a newline -> error
        if (d == ' ' || d == '\t' || d == '\v' || d == '\f') {
            scanner_error("whitespace after backslash before newline in string literal", d);
        }

        // Normal escape sequence: decode and append single char
        sbuf_push(&b, (char)decode_escape_sequence(d));
        continue;
    }

        // regular character
        sbuf_push(&b, (char)c);
    }

    char *lex = sbuf_steal(&b);

    Token t = {0};
    t.type = STRING_LITERAL;
    t.lexeme = lex;       // print_token will display it with escaping
    t.line = cur_line;
    t.column = cur_column;
    t.int_literal = 0;
    return t;
}

// create a simple token with no lexeme
Token make_simple(TokenType ty) {
    Token t = (Token){0};
    t.type = ty;
    t.line = cur_line;
    t.column = cur_column;
    t.lexeme = (char*)FIXED_LEXEMES[ty];
    return t;
}

// scan an operator or delimiter, given the first character already read
Token scan_operator_or_delim(int first) {
    int c = first;
    int d = peek_char();

    /* Shift operators, 2|3 chars */
    if (c == '<' && d == '<') {
        (void)next_char();                 // consume second '<'
        int e = peek_char();
        if (e == '=') { (void)next_char(); return make_simple(ASSIGN_LSH); } // <<=
        return make_simple(LSH);                                             // <<
    }
    if (c == '>' && d == '>') {
        (void)next_char();                 // consume second '>'
        int e = peek_char();
        if (e == '=') { (void)next_char(); return make_simple(ASSIGN_RSH); } // >>=
        return make_simple(RSH);                                             // >>
    }

    /* 2-char sequences */
    if (c == '=' && d == '=') { (void)next_char(); return make_simple(EQ); }    // ==
    if (c == '!' && d == '=') { (void)next_char(); return make_simple(NE); }    // !=
    if (c == '>' && d == '=') { (void)next_char(); return make_simple(GE); }    // >=
    if (c == '<' && d == '=') { (void)next_char(); return make_simple(LE); }    // <=
    if (c == '|' && d == '|') { (void)next_char(); return make_simple(OR); }    // ||
    if (c == '&' && d == '&') { (void)next_char(); return make_simple(AND); }   // &&
    if (c == '+' && d == '=') { (void)next_char(); return make_simple(ASSIGN_ADD); } // +=
    if (c == '-' && d == '=') { (void)next_char(); return make_simple(ASSIGN_SUB); } // -=
    if (c == '*' && d == '=') { (void)next_char(); return make_simple(ASSIGN_MUL); } // *=
    if (c == '/' && d == '=') { (void)next_char(); return make_simple(ASSIGN_DIV); } // /=
    if (c == '%' && d == '=') { (void)next_char(); return make_simple(ASSIGN_MOD); } // %=
    if (c == '^' && d == '=') { (void)next_char(); return make_simple(ASSIGN_XOR); } // ^=
    if (c == '|' && d == '=') { (void)next_char(); return make_simple(ASSIGN_OR); }  // |=
    if (c == '&' && d == '=') { (void)next_char(); return make_simple(ASSIGN_AND); } // &=

    /* 1-char operators & delimiters */
    switch (c) {
        case '=': return make_simple(ASSIGN);
        case '!': return make_simple(NOT);
        case '~': return make_simple(BIT_NOT);
        case '|': return make_simple(BIT_OR);
        case '^': return make_simple(BIT_XOR);
        case '&': return make_simple(BIT_AND);
        case '>': return make_simple(GT);
        case '<': return make_simple(LT);
        case '+': return make_simple(ADD);
        case '-': return make_simple(SUB);
        case '*': return make_simple(MUL);
        case '/': return make_simple(DIV);
        case '%': return make_simple(MOD);
        case ';': return make_simple(SEMICOLON);
        case ':': return make_simple(COLON);
        case ',': return make_simple(COMMA);
        case '(': return make_simple(LPAREN);
        case ')': return make_simple(RPAREN);
        case '[': return make_simple(LBRACKET);
        case ']': return make_simple(RBRACKET);
        case '{': return make_simple(LBRACE);
        case '}': return make_simple(RBRACE);
        default:
            scanner_error("invalid character", c);
            return make_simple(NULL_TOKEN); // never reached
    }
}

// get the next token from input
Token get_token() {
    skip_spaces_and_comments();

    int c = next_char();

    // end of file
    if (c == EOF) {
        Token t = (Token){0};
        t.type = EOF_TOKEN;
        t.line = cur_line;
        t.column = cur_column;
        return t;
    }

    // identifier or keyword
    if (is_ident_start(c)) {
        return scan_identifier_or_keyword(c);
    }

    // numbers
    if (isdigit(c)) {
        return scan_number(c);
    }

    // string literal
    if (c == '"') {
        return scan_string();
    }
    
    // operators and delimiters
    return scan_operator_or_delim(c);
}
