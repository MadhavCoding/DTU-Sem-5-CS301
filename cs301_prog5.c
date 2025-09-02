#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_RULES 10
#define MAX_LEN   100

struct Production 
{
    char lhs;
    char rhs[MAX_RULES][MAX_LEN];
    int count;  
};

char input[100]; // Input String (for Parsing)
int pos = 0; // Current Index of the Input String
struct Production grammar[26]; // Production Rules
int startSymbol; // Start Symbol

int isNonTerminal(char c) 
{
    return (c >= 'A' && c <= 'Z');
}

int parseNonTerminal(char nt) 
{
    struct Production rule = grammar[nt - 'A'];

    for (int i = 0; i < rule.count; i++) 
    {
        int backup = pos; // Save Current Position
        int success = 1; 

        for (int j = 0; rule.rhs[i][j] != '\0'; j++) 
        {
            char symbol = rule.rhs[i][j];

            // Non Terminal
            if (isNonTerminal(symbol)) 
            {
                if (!parseNonTerminal(symbol)) 
                { 
                    success = 0; 
                    break; 
                }
            }

            // Terminal
            else 
            {
                if (input[pos] == symbol) pos++;
                else 
                { 
                    success = 0; 
                    break; 
                }
            }
        }

        if (success) return 1;
        pos = backup; // Back Track
    }
    return 0;
}

void inputGrammar() 
{
    // initialize grammar table
    for (int i = 0; i < 26; i++) 
    {
        grammar[i].lhs = 0;
        grammar[i].count = 0;
        for (int j = 0; j < MAX_RULES; j++) grammar[i].rhs[j][0] = '\0';
    }

    int n;
    printf("Enter number of productions: ");
    scanf("%d", &n);
    getchar(); // Consume Newline
    printf("\n");

    for (int i = 0; i < n; i++) 
    {
        char buffer[MAX_LEN];
        printf("Enter production %d (e.g. A = B | C): ", i + 1);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) { printf("Input error.\n"); exit(1); }

        buffer[strcspn(buffer, "\n")] = '\0'; // Remove Newline

        // Remove Spaces
        char nospace[MAX_LEN];
        int p = 0;
        for (int t = 0; buffer[t] != '\0' && p < MAX_LEN-1; t++) 
        {
            if (!isspace((unsigned char)buffer[t])) nospace[p++] = buffer[t];
        }
        nospace[p] = '\0';

        char lhs = '\0';
        char *rhsPart = NULL;

        // First Char Uppercase Non Terminal
        if (nospace[0] != '\0' && isNonTerminal(nospace[0])) 
        {
            lhs = nospace[0];
        } 
        else 
        {
            printf("Invalid LHS in production: %s\n", buffer);
            i--;
            continue;
        }

        char *eq = strchr(nospace, '=');
        if (eq) rhsPart = eq + 1;

        if (rhsPart == NULL || rhsPart[0] == '\0') 
        {
            printf("Invalid production format (no RHS): %s\n", buffer);
            i--;
            continue;
        }

        int idx = lhs - 'A';
        grammar[idx].lhs = lhs;
    
        char *token = strtok(rhsPart, "|");

        while (token != NULL) 
        {
            strncpy(grammar[idx].rhs[grammar[idx].count], token, MAX_LEN-1);
            grammar[idx].rhs[grammar[idx].count][MAX_LEN-1] = '\0';
            grammar[idx].count++;
            token = strtok(NULL, "|");
        }
    }

    // Start Symbol
    printf("\nEnter start symbol: ");
    char start;
    scanf(" %c", &start);
    startSymbol = start;
    getchar(); // Consume Newline
}

int main() 
{
    inputGrammar();

    // Input String
    printf("\nEnter input string: ");
    scanf("%s", input);

    // Reset Position
    pos = 0;

    // Parsing from Start Symbol
    if (parseNonTerminal(startSymbol) && input[pos] == '\0') 
    {
        printf("\nString is ACCEPTED!\n");
    } 
    else 
    {
        printf("\nString is REJECTED!\n");
    }

    return 0;
}
