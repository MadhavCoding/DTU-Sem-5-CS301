#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 1000

char code[1000]; // Input Code
int pos = 0;
char token[100]; // Token generated

void input_code() 
{
    printf("Enter program code (end with $):\n\n");
    char ch;
    int i = 0;
    while ((ch = getchar()) != '$' && i < MAX - 1) 
    {
        code[i++] = ch;
    }
    code[i] = '\0';
}

int get_next_token() 
{
    int i = 0;

    // Whitespaces
    while (code[pos] == ' ' || code[pos] == '\t' || code[pos] == '\n') 
    {
        (pos)++;
    }

    // End of Code
    if (code[pos] == '\0') return 0;

    // Preprocessor directive
    if (code[pos] == '#') 
    {
        token[i++] = code[pos++];
        while (isalpha(code[pos])) token[i++] = code[pos++];
        token[i] = '\0';
        return 1;
    }

    // String Literal
    if (code[pos] == '"' || code[pos] == '\'')
    {
        char start = code[pos];
        token[i++] = code[pos++]; 
        while (code[pos] != start && code[pos] != '\0') 
        {
            token[i++] = code[pos++];
        }
        if (code[pos] == start) 
        { 
            token[i++] = code[pos++];
        }
        token[i] = '\0';
        return 1;
    }

    // Identifier or Keyword
    if (isalpha(code[pos]) || code[pos] == '_') 
    {
        while (isalnum(code[pos]) || code[pos] == '_' || code[pos] == '.')
            token[i++] = code[pos++];
        token[i] = '\0';
        return 1;
    }

    // Number
    if (isdigit(code[pos])) 
    {
        while (isdigit(code[pos]))
            token[i++] = code[pos++];
        token[i] = '\0';
        return 1;
    }

    // Operator (single char for now)
    if (strchr("+-*/%=<>!&|;", code[pos])) 
    {
        token[i++] = code[pos];

        if (code[pos + 1] != '\0') 
        {
            if ((code[pos] == '+' && code[pos + 1] == '+') ||
                (code[pos] == '-' && code[pos + 1] == '-') ||
                (code[pos] == '+' && code[pos + 1] == '=') ||
                (code[pos] == '-' && code[pos + 1] == '=') ||
                (code[pos] == '*' && code[pos + 1] == '=') ||
                (code[pos] == '/' && code[pos + 1] == '=') ||
                (code[pos] == '%' && code[pos + 1] == '=') ||
                (code[pos] == '=' && code[pos + 1] == '=') ||
                (code[pos] == '!' && code[pos + 1] == '=') ||
                (code[pos] == '<' && code[pos + 1] == '=') ||
                (code[pos] == '>' && code[pos + 1] == '=') ||
                (code[pos] == '&' && code[pos + 1] == '&') ||
                (code[pos] == '|' && code[pos + 1] == '|'))
            {
                token[i++] = code[++pos];
            }
        }

        token[i] = '\0';
        pos++; 
        return 1;
    }

    // Symbol 
    token[i++] = code[pos++];
    token[i] = '\0';

    return 1;
}

void classify_token(char *token) 
{
    // Keywords list 
    char *keywords[] = {"int", "float", "char", "if", "else", "while", "for", "return", "main", "const", "argc", "argv", "printf", "scanf"};
    int total_keywords = sizeof(keywords) / sizeof(keywords[0]);

    // Check preprocessor
    if (token[0] == '#') 
    {
        printf("%-40s : Preprocessor Directive\n", token);
        return;
    }

    // Check keywords
    for (int i = 0; i < total_keywords; i++) 
    {
        if (strcmp(token, keywords[i]) == 0) 
        {
            printf("%-40s : Keyword\n", token);
            return;
        }
    }

    // Check identifier
    if (isalpha(token[0]) || token[0] == '_') 
    {
        printf("%-40s : Identifier\n", token);
        return;
    }

    // Check number
    if (isdigit(token[0])) 
    {
        printf("%-40s : Number\n", token);
        return;
    }

    // Check string literal
    if (token[0] == '"' || token[0] == '\'')
    {
        printf("%-40s : String\n", token);
        return;
    }

    // Check operator
    if (strchr("+-*/%=<>!&|;", token[0])) 
    {
        printf("%-40s : Operator\n", token);
        return;
    }

    // Otherwise symbol
    printf("%-40s : Symbol\n", token);
}

int main() 
{
    input_code();

    printf("\nTokens:\n");
    while (get_next_token()) 
    {
        classify_token(token);
    }

    return 0;
}
