#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINES 1000
#define MAX_LINE_LEN 500

typedef struct 
{
    int line;
    char type[30];
    char message[200];
} Error;

Error errors[MAX_LINES];
int error_count = 0;

void add_error(int line, const char *type, const char *message) 
{
    if (error_count < MAX_LINES) 
    {
        errors[error_count].line = line;
        strcpy(errors[error_count].type, type);
        strcpy(errors[error_count].message, message);
        error_count++;
    }
}

void check_code(char code[MAX_LINES][MAX_LINE_LEN], int total_lines) 
{
    int brace = 0, paren = 0;
    int in_string = 0;

    for (int i = 0; i < total_lines; i++) 
    {
        char *line = code[i];
        int len = strlen(line);
        int has_semicolon = 0;

        // Track braces, parentheses, and string literals
        for (int j = 0; j < len; j++) 
        {
            if (line[j] == '"') in_string = in_string ^ 1;
            if (!in_string) 
            {
                if (line[j] == '{') brace++;
                if (line[j] == '}') brace--;
                if (line[j] == '(') paren++;
                if (line[j] == ')') paren--;
            }
        }

        // Skip empty or non-statement lines
        if (len == 0 || strstr(line, "{") || strstr(line, "}") ||
            strstr(line, "if(") || strstr(line, "for(") || strstr(line, "while(") ||
            strstr(line, "#include") || strstr(line, "else"))
            continue;

        // Check for missing semicolon
        if (len > 0 && !in_string && line[len - 1] != ';')
            add_error(i + 1, "Syntax Error", "Possible missing semicolon");

        // Check unclosed string literal
        if (in_string)
            add_error(i + 1, "Lexical Error", "Unterminated string literal");
    }

    if (brace != 0)
        add_error(total_lines, "Syntax Error", "Unbalanced braces");
    if (paren != 0)
        add_error(total_lines, "Syntax Error", "Unbalanced parentheses");
}

void print_errors() 
{
    if (error_count == 0) 
    {
        printf("\nNo errors found!\n");
        return;
    }

    printf("\n\n");
    printf("\n%-10s %-15s %-40s\n", "Line No.", "Error Type", "Description");
    printf("-------------------------------------------------------------\n");

    for (int i = 0; i < error_count; i++) 
    {
        printf("%-10d %-15s %-40s\n",
               errors[i].line,
               errors[i].type,
               errors[i].message);
    }
    
    printf("\n\n");
}

int main() 
{
    char code[MAX_LINES][MAX_LINE_LEN];
    int line_count = 0;

    printf("Enter your C code :\n\n");

    // Read full code till EOF
    while (fgets(code[line_count], MAX_LINE_LEN, stdin)) 
    {
        code[line_count][strcspn(code[line_count], "\n")] = '\0'; // remove newline
        line_count++;
        if (line_count >= MAX_LINES) break;
    }

    check_code(code, line_count);
    print_errors();

    return 0;
}
