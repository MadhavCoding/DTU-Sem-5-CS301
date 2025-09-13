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

struct Production grammar[26];
char startSymbol;

int isNonTerminal(char c) 
{
    return (c >= 'A' && c <= 'Z');
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
    return '?'; 
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

int isLeftRecursive(char nonTerminal, char productions[][50], int prodCount) 
{
    for (int i = 0; i < prodCount; i++) 
    {
        if (productions[i][0] == nonTerminal) 
        {
            return 1;  // left recursion
        }
    }
    return 0;  // no left recursion
}

void splitAlphaBeta(int idx,
                    char alphas[][MAX_LEN], int *alphaCount,
                    char betas[][MAX_LEN],  int *betaCount)
{
    // idx: index in grammar
    // alphas: suffixes after the leading LHS
    // betas: productions that do not start with LHS

    struct Production *prod = &grammar[idx];
    *alphaCount = 0;
    *betaCount  = 0;

    if (prod->lhs == 0 || prod->count == 0) return; 

    for (int i = 0; i < prod->count; i++) 
    {
        char *rhs = prod->rhs[i];
        if (rhs[0] == '\0') continue; // empty entries

        if (rhs[0] == prod->lhs) 
        {
            // left-recursive production: Aa -> store a
            if (*alphaCount >= MAX_RULES) 
            {
                // overflow
                continue;
            }
            if (rhs[1] == '\0') 
            {
                // A -> A (no suffix) -> epsilon
                strncpy(alphas[*alphaCount], "#", MAX_LEN-1);
                alphas[*alphaCount][MAX_LEN-1] = '\0';
            } 
            else 
            {
                // copy suffix (rhs + 1)
                strncpy(alphas[*alphaCount], rhs + 1, MAX_LEN-1);
                alphas[*alphaCount][MAX_LEN-1] = '\0';
            }
            (*alphaCount)++;
        } 
        else 
        {
            // non-left-recursive production -> beta
            if (*betaCount >= MAX_RULES) 
            {
                // overflow
                continue;
            }
            strncpy(betas[*betaCount], rhs, MAX_LEN-1);
            betas[*betaCount][MAX_LEN-1] = '\0';
            (*betaCount)++;
        }
    }
}

void eliminateLeftRecursion1(int idx) 
{
    char alphas[MAX_RULES][MAX_LEN], betas[MAX_RULES][MAX_LEN];
    int aCount, bCount;

    splitAlphaBeta(idx, alphas, &aCount, betas, &bCount);

    if (aCount == 0) 
    {
        // no left recursion
        return;
    }

    char A = grammar[idx].lhs;
    char Aprime = getNewNonTerminal();

    // Rewrite A's productions: A → b A'
    struct Production *prodA = &grammar[idx];
    prodA->count = 0;
    for (int i = 0; i < bCount; i++) 
    {
        snprintf(prodA->rhs[prodA->count], MAX_LEN, "%s%c", betas[i], Aprime);
        prodA->count++;
    }

    // New non-terminal A'
    int idxPrime = Aprime - 'A';
    grammar[idxPrime].lhs = Aprime;
    grammar[idxPrime].count = 0;

    // Add aA' rules
    for (int i = 0; i < aCount; i++) 
    {
        snprintf(grammar[idxPrime].rhs[grammar[idxPrime].count], MAX_LEN, "%s%c", alphas[i], Aprime);
        grammar[idxPrime].count++;
    }

    // Add epsilon
    strncpy(grammar[idxPrime].rhs[grammar[idxPrime].count], "#", MAX_LEN-1);
    grammar[idxPrime].rhs[grammar[idxPrime].count][MAX_LEN-1] = '\0';
    grammar[idxPrime].count++;
}

void eliminateLeftRecursion() 
{
    for (int i = 0; i < 26; i++) 
    {
        if (grammar[i].lhs != 0) 
        {
            eliminateLeftRecursion1(i);
        }
    }
}

void printGrammar() 
{
    printf("\nGrammar after removing Left Recursion:\n");
    for (int i = 0; i < 26; i++) 
    {
        if (grammar[i].lhs != 0) 
        {
            printf("%c = ", grammar[i].lhs);
            for (int j = 0; j < grammar[i].count; j++) 
            {
                printf("%s", grammar[i].rhs[j]);
                if (j < grammar[i].count - 1) printf(" | ");
            }
            printf("\n");
        }
    }
}

int main() 
{
    inputGrammar();
    eliminateLeftRecursion();
    printGrammar();
    return 0;
}

