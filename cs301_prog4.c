#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 10000 // Input Code Size
#define MAX_TOKEN_SIZE 1000 // Token Size

char code[MAX]; // Input Program
int pos = 0; // Scanning Position
char token[MAX_TOKEN_SIZE]; // Current Token

// Check Keyword
int isKeyword(const char *word) 
{
    // List of keywords (C99)
    const char *keywords[] = 
    {
        "auto","break","case","char","const","continue","default","do","double",
        "else","enum","extern","float","for","goto","if","int","long","register",
        "return","short","signed","sizeof","static","struct","switch","typedef",
        "union","unsigned","void","volatile","while",
        "_Bool","_Complex","_Imaginary","inline","restrict"
    };

    int keywordCount = sizeof(keywords) / sizeof(keywords[0]);

    for (int i = 0; i < keywordCount; i++) 
    {
        if (strcmp(word, keywords[i]) == 0) 
        {
            return 1;  // Keyword
        }
    }
    return 0;
}

// Check Delimiter
int isDelimiter(char ch) 
{
    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == ',' || ch == ';' || ch == ':' || ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == '[' || ch == ']' ) 
    {
        return 1; // Delimitor
    }
    return 0; 
}

// Check Operator
int isOperator(const char *str)
{
    const char *operators[] = 
    {
        // 3-char
        "...", "<<=", ">>=",
        // 2-char
        "++", "--", "==", "!=", "<=", ">=", "&&", "||", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<", ">>", "->",
        // 1-char
        "+", "-", "*", "/", "%", "=", "<", ">", "!", "&", "|", "^", "~", "?", ":"
    };

    int total = sizeof(operators) / sizeof(operators[0]);

    for (int i = 0; i < total; ++i) 
    {
        if (strcmp(str, operators[i]) == 0) return 1;
    }
    return 0;
}

// Check Hex Digit
int isHexDigit(char c) 
{
    return (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

// Check Number
int isNumber(const char *str) 
{
    int i = 0;

    // Handle optional +/-
    if (str[i] == '+' || str[i] == '-') 
    {
        i++;
    }

    // Hexadecimal: starts with 0x or 0X
    if (str[i] == '0' && str[i+1] != '\0' && (str[i+1] == 'x' || str[i+1] == 'X')) 
    {
        i += 2;
        int hasHex = 0;
        while (str[i] != '\0' && isHexDigit(str[i])) 
        {
            hasHex = 1;
            i++;
        }
        if(!hasHex) return 0;
    }

    // Octal: starts with 0 (and not hex)
    else if (str[i] == '0' && str[i+1] != '\0' && str[i+1] != '.' && str[i+1] != 'e' && str[i+1] != 'E') 
    {
        i++;
        int hasOct = 0;
        while (str[i] != '\0' && isdigit((unsigned char)str[i])) 
        {
            if (str[i] < '0' || str[i] > '7') return 0; 
            hasOct = 1;
            i++;
        }
    }

    else
    {
        int hasDigits = 0, hasDecimal = 0, hasExponent = 0, hasMinus = 0;

        while (str[i] != '\0') 
        {
            if (isdigit(str[i])) 
            {
                hasDigits = 1;
                i++;
                continue;
            }

            else if (str[i] == '.') 
            {
                if (hasDecimal || hasExponent) return 0; // Only one Decimal point (before Exponent)
                hasDecimal = 1;
                i++;
                continue;
            }

            else if (str[i] == 'e' || str[i] == 'E') 
            {
                if (hasExponent || !hasDigits) return 0; // Only one Exponent (digits after that)
                hasExponent = 1;
                i++; 

                // optional sign after e/E
                if (str[i] == '+' || str[i] == '-') i++;

                // atleast one digit after e/E
                if (!isdigit((unsigned char)str[i])) return 0;

                while (isdigit((unsigned char)str[i])) i++;
                
                break;
            }

            break;
        }

        if (!hasDigits && !hasDecimal) return 0;
    }
    

    // Handle Suffix (run after core parsing)
    int u = 0, l = 0, f = 0;
    while (str[i]) 
    {
        if ((str[i] == 'u' || str[i] == 'U') && !u && !f) u = 1;
        else if ((str[i] == 'l' || str[i] == 'L') && l < 2 && !f) 
        {
            // allow single L or LL
            if ((str[i+1] != '\0' && str[i+1] == 'l' || str[i+1] == 'L') && l == 0) 
            {
                l = 2;
                i++; // consume the second L 
            } 
            else 
            {
                l++;
            }
        }
        else if ((str[i] == 'f' || str[i] == 'F') && !l && !u && !f) f = 1;
        else return 0;
        i++;
    }

    return 1; // Number
}

// Check Identifier
int isIdentifier(const char *str) 
{
    // 1st char: letter or '_'
    if (!(isalpha(str[0]) || str[0] == '_')) 
    {
        return 0;
    }

    // Rest char: letters, digits or '_'
    for (int i = 1; str[i] != '\0'; i++) 
    {
        if (!(isalnum(str[i]) || str[i] == '_')) {
            return 0;
        }
    }

    // Not Keyword
    if (isKeyword(str)) 
    {
        return 0;
    }

    return 1; // Identifier
}

// Check String
int isStringLiteral(const char *str) 
{
    int len = strlen(str);

    // Start and End with ""
    if (len < 2 || str[0] != '"' || str[len - 1] != '"') 
    {
        return 0;
    }

    for (int i = 1; i < len - 1; i++) 
    {
        if (str[i] == '"' && str[i - 1] != '\\') 
        {
            // unescaped quote inside string
            return 0;
        }
        if (str[i] == '\n') 
        {
            // newline not allowed inside string
            return 0;
        }
    }

    return 1; // String
}

// Check Character
int isCharLiteral(const char *str) 
{
    int len = strlen(str);

    if (len < 3 || str[0] != '\'' || str[len - 1] != '\'') 
    {
        return 0;
    }

    if (len == 3) 
    {
        // case: 'a'
        return 1;
    } 
    
    else if (len == 4 && str[1] == '\\') 
    {
        // case: '\n', '\t', '\''
        return 1;
    }

    return 0;
}

// Check Preprocessor
int isPreprocessor(char* str) 
{
    if (str[0] == '#') 
    {
        return 1; // Preprocessor directive
    }
    return 0;
}

int getNextToken()
{
    int i = 0;
    int len = (int)strlen(code);

    // helper to append a char safely
    #define APPEND_CH(c) do { if (i < MAX_TOKEN_SIZE - 1) token[i++] = (c); else { /* drop rest */; } } while(0)

    // 0) Skip whitespace and comments
    for (;;) 
    {
        // skip whitespace
        while (pos < len && isspace((unsigned char)code[pos])) pos++;

        if (pos >= len) { token[0] = '\0'; return 0; }

        // skip single-line comment //
        if (code[pos] == '/' && pos + 1 < len && code[pos + 1] == '/') 
        {
            pos += 2;
            while (pos < len && code[pos] != '\n') pos++;
            continue; // skip and restart whitespace skip
        }

        // skip multi-line comment /* ... */
        if (code[pos] == '/' && pos + 1 < len && code[pos + 1] == '*') 
        {
            pos += 2;
            while (pos + 1 < len && !(code[pos] == '*' && code[pos + 1] == '/')) pos++;
            if (pos + 1 < len) pos += 2; // closing */
            continue;
        }

        break;
    }

    if (pos >= len) { token[0] = '\0'; return 0; }

    // 1) Preprocessor directive: consume whole logical line (with backslash continuation)
    if (code[pos] == '#') 
    {
        int isInclude = 0, isDefine = 0;

        if (strncmp(&code[pos + 1], "include", 7) == 0) isInclude = 1;

        else if (strncmp(&code[pos + 1], "define", 6) == 0) isDefine = 1;

        // unknown preprocessor directives
        if (!isInclude && !isDefine) 
        {
            APPEND_CH(code[pos++]);
            token[i] = '\0';
            return 1;
        }

        i = 0;
        while (pos < len) 
        {
            // handle backslash-newline continuation
            if (code[pos] == '\\' && pos + 1 < len && code[pos + 1] == '\n') 
            {
                APPEND_CH(code[pos++]); // '\'
                APPEND_CH(code[pos++]); // '\n'
                continue;
            }
            if (code[pos] == '\n') break;
            APPEND_CH(code[pos++]);
        }
        // consume terminating newline if any
        if (pos < len && code[pos] == '\n') pos++;
        token[i] = '\0';
        return 1;
    }

    // 2) String literal: " ... " (escapes are part of token)
    if (code[pos] == '"')
    {
        i = 0;
        APPEND_CH(code[pos++]); // opening quote
        while (pos < len) 
        {
            if (code[pos] == '\\') 
            {
                // escape sequence: copy both chars if present
                APPEND_CH(code[pos++]);
                if (pos < len) APPEND_CH(code[pos++]);
                continue;
            }
            if (code[pos] == '"') 
            {
                APPEND_CH(code[pos++]); // closing quote
                break;
            }
            // newline in string: include it and break (unterminated)
            if (code[pos] == '\n' || code[pos] == '\0') 
            {
                // treat as unterminated string: include newline and stop
                APPEND_CH(code[pos++]);
                break;
            }
            APPEND_CH(code[pos++]);
        }
        token[i] = '\0';
        return 1;
    }

    // 3) Character literal: 'a'  or '\n'
    if (code[pos] == '\'') {
        i = 0;
        APPEND_CH(code[pos++]); // opening '
        while (pos < len) 
        {
            if (code[pos] == '\\') 
            {
                APPEND_CH(code[pos++]); // backslash
                if (pos < len) APPEND_CH(code[pos++]); // escaped char
                continue;
            }
            if (code[pos] == '\'') 
            {
                APPEND_CH(code[pos++]); // closing '
                break;
            }
            if (code[pos] == '\n' || code[pos] == '\0') 
            {
                // unterminated char literal
                APPEND_CH(code[pos++]); break;
            }
            APPEND_CH(code[pos++]);
        }
        token[i] = '\0';
        return 1;
    }

    // 4) Number literal (starts with digit OR dot followed by digit)
    if (isdigit((unsigned char)code[pos]) || (code[pos] == '.' && pos + 1 < len && isdigit((unsigned char)code[pos + 1]))) 
    {
        i = 0;
        // Hex: 0x or 0X
        if (code[pos] == '0' && pos + 1 < len && (code[pos + 1] == 'x' || code[pos + 1] == 'X')) 
        {
            APPEND_CH(code[pos++]); // 0
            APPEND_CH(code[pos++]); // x/X
            while (pos < len && isHexDigit(code[pos])) APPEND_CH(code[pos++]);
            // optional integer suffixes (u/l/U/L) - collect them if present
            while (pos < len && (isalpha((unsigned char)code[pos]))) APPEND_CH(code[pos++]);
            token[i] = '\0';
            return 1;
        }

        // Decimal / Float / Exponent
        int seenDot = 0, seenExp = 0;
        while (pos < len) {
            char c = code[pos];
            if (isdigit((unsigned char)c)) {
                APPEND_CH(c); pos++; continue;
            }
            if (c == '.' && !seenDot && !seenExp) {
                seenDot = 1; APPEND_CH(c); pos++; continue;
            }
            if ((c == 'e' || c == 'E') && !seenExp) 
            {
                seenExp = 1; APPEND_CH(c); pos++;
                // optional sign after exponent
                if (pos < len && (code[pos] == '+' || code[pos] == '-')) { APPEND_CH(code[pos]); pos++; }
                continue;
            }
            // suffixes like f, F, l, L, u, U - include if directly attached
            if ((c == 'f' || c == 'F' || c == 'l' || c == 'L' || c == 'u' || c == 'U') && (i > 0)) {
                APPEND_CH(c); pos++; continue;
            }
            break;
        }
        token[i] = '\0';
        return 1;
    }

    // 5) Identifier or keyword or preprocessor-like identifier (start with letter or underscore)
    if (isalpha((unsigned char)code[pos]) || code[pos] == '_') 
    {
        i = 0;
        while (pos < len && (isalnum((unsigned char)code[pos]) || code[pos] == '_')) 
        {
            APPEND_CH(code[pos++]);
        }
        token[i] = '\0';
        return 1;
    }

    // 6) Operators / Punctuators
    // 3-char ops
    if (pos + 2 < len) 
    {
        char s3[4] = { code[pos], code[pos+1], code[pos+2], '\0' };
        const char *ops3[] = {"<<=", ">>=", "..."};
        for (int k = 0; k < (int)(sizeof(ops3)/sizeof(ops3[0])); ++k) 
        {
            if (strcmp(s3, ops3[k]) == 0) 
            {
                // copy 3 chars
                i = 0;
                APPEND_CH(code[pos++]); APPEND_CH(code[pos++]); APPEND_CH(code[pos++]);
                token[i] = '\0';
                return 1;
            }
        }
    }

    // 2-char ops
    if (pos + 1 < len) 
    {
        char s2[3] = { code[pos], code[pos+1], '\0' };
        const char *ops2[] = {
            "++","--","==","!=","<=",">=","&&","||","+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<", ">>", "->"
        };
        for (int k = 0; k < (int)(sizeof(ops2)/sizeof(ops2[0])); ++k) 
        {
            if (strcmp(s2, ops2[k]) == 0) {
                i = 0;
                APPEND_CH(code[pos++]); APPEND_CH(code[pos++]);
                token[i] = '\0';
                return 1;
            }
        }
    }

    // single-char token (operator, punctuator or other)
    i = 0;
    APPEND_CH(code[pos++]);
    token[i] = '\0';
    return 1;

    #undef APPEND_CH
}

char* classifyToken(const char *lexeme) 
{
    if (lexeme == NULL || lexeme[0] == '\0') return "Unknown";

    // 1) Preprocessor 
    if (isPreprocessor((char *)lexeme) || lexeme[0] == '#') return "Preprocessor";

    // 2) String
    if (isStringLiteral(lexeme)) return "String";

    // 3) Char
    if (isCharLiteral(lexeme))   return "Char";

    // 4) Number 
    if (isNumber(lexeme)) 
    {
        if (strchr(lexeme, '.') || strchr(lexeme, 'e') || strchr(lexeme, 'E') || strchr(lexeme, 'f') || strchr(lexeme, 'F')) 
        {
            return "Float";
        } 
        else 
        {
            return "Integer";
        }
    }

    // 5) Keyword
    if (isKeyword(lexeme)) return "Keyword";

    // 6) Identifier
    if (isIdentifier(lexeme)) return "Identifier";

    // 7) Operator 
    if (isOperator(lexeme)) return "Operator";

    // 8) Delimiter: 
    if (strlen(lexeme) == 1 && isDelimiter(lexeme[0])) return "Delimiter";

    // 9) Fallback: unknown token
    return "Unknown";
}

int main() 
{
    int c, idx = 0;
    while ((c = getchar()) != EOF && idx < MAX - 1) 
    {
        code[idx++] = (char)c;
    }
    code[idx] = '\0';

    // Lexical analysis loop
    while (getNextToken()) 
    {
        char *type = classifyToken(token);
        char token_modified[MAX_TOKEN_SIZE];
        sprintf(token_modified, "<%s>", token);
        printf("%-45s : <%s>\n", token_modified, type);

        // if (strcmp(type, "Unknown") == 0) 
        // {
        //     fprintf(stderr, "Lexical error: unknown token '%s'\n", token);
        // }
    }

    return 0;
}