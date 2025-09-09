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

struct Production grammar[26]; // Production Rules
int startSymbol; // Start Symbol

int isNonTerminal(char c) 
{
    return (c >= 'A' && c <= 'Z');
}

int commonPrefixLen(char *a, char *b) 
{
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return i;
}

char getNewNonTerminal() 
{
    for (char c = 'A'; c <= 'Z'; c++) 
    {
        if (grammar[c - 'A'].lhs == 0) 
        {
            return c;
        }
    }
    return '?'; // if none available
}

void leftFactorOne(int idx) 
{
    struct Production *prod = &grammar[idx];

    for (int i = 0; i < prod->count; i++) 
    {
        for (int j = i + 1; j < prod->count; j++) 
        {
            int prefix = commonPrefixLen(prod->rhs[i], prod->rhs[j]);
            if (prefix > 0) 
            {
                char newNT = getNewNonTerminal();
                grammar[newNT - 'A'].lhs = newNT;

                // Save common prefix
                char prefixStr[MAX_LEN];
                strncpy(prefixStr, prod->rhs[i], prefix);
                prefixStr[prefix] = '\0';

                // New RHS for original production
                char newRHS[MAX_LEN];
                snprintf(newRHS, sizeof(newRHS), "%s%c", prefixStr, newNT);

                // copy all original RHS into temp
                char oldRHS[MAX_RULES][MAX_LEN];
                int oldCount = prod->count;
                for (int k = 0; k < oldCount; k++) {
                    strcpy(oldRHS[k], prod->rhs[k]);
                }

                // reset original production
                int newCountMain = 0;
                strcpy(prod->rhs[newCountMain++], newRHS);

                // move factored suffixes to newNT
                int newCount = 0;
                for (int k = 0; k < oldCount; k++) 
                {
                    if (strncmp(oldRHS[k], prefixStr, prefix) == 0) 
                    {
                        char *suffix = oldRHS[k] + prefix;
                        if (*suffix == '\0')
                            strcpy(grammar[newNT - 'A'].rhs[newCount], "#");
                        else
                            strcpy(grammar[newNT - 'A'].rhs[newCount], suffix);
                        newCount++;
                    }
                    else 
                    {
                        // keep unrelated RHS in the original production
                        strcpy(prod->rhs[newCountMain++], oldRHS[k]);
                    }
                }
                prod->count = newCountMain;
                grammar[newNT - 'A'].count = newCount;

                return; 
            }
        }
    }
}

void leftFactor() 
{
    int changed = 1;
    while (changed) 
    {
        changed = 0;
        for (int i = 0; i < 26; i++) 
        {
            if (grammar[i].lhs != 0 && grammar[i].count > 1) 
            {
                int before = grammar[i].count;
                leftFactorOne(i);
                if (grammar[i].count != before) changed = 1;
            }
        }
    }
}

void printGrammar() 
{
    printf("\nLeft Factored Grammar:\n");
    for (int i = 0; i < 26; i++) 
    {
        if (grammar[i].lhs != 0) 
        {
            printf("%c = ", grammar[i].lhs);
            for (int j = 0; j < grammar[i].count; j++) 
            {
                if (j > 0) printf(" | ");
                printf("%s", grammar[i].rhs[j]);
            }
            printf("\n");
        }
    }
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
    leftFactor();
    printGrammar();

    return 0;
}