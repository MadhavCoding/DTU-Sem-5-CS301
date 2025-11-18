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

struct Production grammar[26]; // Grammar storage
char startSymbol; // Start symbol

int nProductions; // number of productions
int firstComputed[26]; // flags for memoization of FIRST
char FIRST[26][MAX_LEN]; 
char FOLLOW[26][MAX_LEN]; 

char parsingTable[26][128][MAX_LEN]; // parsing table

int isNonTerminal(char c) 
{
    return (c >= 'A' && c <= 'Z');
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

// maintain unique items in set
int addToSet(char set[MAX_LEN], char sym) 
{
    if (sym == '\0') return 0;

    // already present
    for (int i = 0; set[i] != '\0'; ++i) 
    {
        if (set[i] == sym) return 0;
    }

    // append
    int len = strlen(set);
    set[len] = sym;
    set[len+1] = '\0';
    return 1;
}

int firstHasEpsilon(int idx) 
{
    for (int i = 0; FIRST[idx][i] != '\0'; ++i)
        if (FIRST[idx][i] == '#') return 1;
    return 0;
}

// compute FIRST sets for all non-terminals using iterative fixed-point method
void computeAllFirst() 
{
    // initialize
    for (int i = 0; i < 26; ++i) FIRST[i][0] = '\0';

    int changed = 1;
    while (changed) 
    {
        changed = 0;

        for (int A = 0; A < 26; ++A) 
        {
            if (grammar[A].lhs == 0) continue;

            // for each production A -> rhs
            for (int r = 0; r < grammar[A].count; ++r) 
            {
                char *rhs = grammar[A].rhs[r];

                if (rhs[0] == '\0') continue;

                // if production is epsilon symbol '#'
                if (rhs[0] == '#') 
                {
                    if (addToSet(FIRST[A], '#')) changed = 1;
                    continue;
                }

                // go through symbols of rhs one by one
                int pos = 0;
                int allEpsilon = 1;
                while (rhs[pos] != '\0') 
                {
                    char X = rhs[pos];

                    // if X is terminal 
                    if (!isNonTerminal(X)) 
                    {
                        // add terminal to FIRST(A)
                        if (addToSet(FIRST[A], X)) changed = 1;
                        allEpsilon = 0;
                        break; 
                    } 
                    else 
                    {
                        // X is non-terminal: add FIRST(X) - {#} to FIRST(A)
                        int idxX = X - 'A';
                        for (int k = 0; FIRST[idxX][k] != '\0'; ++k) 
                        {
                            char t = FIRST[idxX][k];
                            if (t == '#') continue;
                            if (addToSet(FIRST[A], t)) changed = 1;
                        }
                        // if FIRST(X) contains epsilon, continue to next symbol
                        if (firstHasEpsilon(idxX)) 
                        {
                            allEpsilon = 1;
                            pos++;
                            continue;
                        } 
                        else 
                        {
                            allEpsilon = 0;
                            break;
                        }
                    }
                }

                // if all symbols can derive epsilon, add epsilon to FIRST(A)
                if (allEpsilon) 
                {
                    if (addToSet(FIRST[A], '#')) changed = 1;
                }

            } // end for each production
        } // end for each non-terminal
    }
}

/* compute FOLLOW sets for all non-terminals */
void computeFollow() 
{
    // initialize
    for (int i = 0; i < 26; ++i) FOLLOW[i][0] = '\0';

    // start symbol --> '$'
    addToSet(FOLLOW[startSymbol - 'A'], '$');

    int changed = 1;
    while (changed) 
    {
        changed = 0;

        for (int A = 0; A < 26; ++A) 
        {
            if (grammar[A].lhs == 0) continue;

            // for each production A -> rhs
            for (int r = 0; r < grammar[A].count; ++r) 
            {
                char *rhs = grammar[A].rhs[r];

                for (int i = 0; rhs[i] != '\0'; ++i) 
                {
                    char B = rhs[i];

                    // only process non-terminals
                    if (!isNonTerminal(B)) continue;
                    int Bidx = B - 'A';

                    int allEpsilonAfter = 1;

                    // check symbols after B
                    for (int j = i + 1; rhs[j] != '\0'; ++j) 
                    {
                        char X = rhs[j];

                        if (!isNonTerminal(X)) 
                        {
                            if (addToSet(FOLLOW[Bidx], X)) changed = 1;
                            allEpsilonAfter = 0;
                            break;
                        } 
                        else 
                        {
                            int Xidx = X - 'A';

                            // add FIRST(X) - {#} to FOLLOW(B)
                            for (int k = 0; FIRST[Xidx][k] != '\0'; ++k) 
                            {
                                char t = FIRST[Xidx][k];
                                if (t == '#') continue;
                                if (addToSet(FOLLOW[Bidx], t)) changed = 1;
                            }

                            if (firstHasEpsilon(Xidx)) 
                            {
                                allEpsilonAfter = 1;
                                continue;
                            } 
                            else 
                            {
                                allEpsilonAfter = 0;
                                break;
                            }
                        }
                    }

                    // if B is at end or all symbols after B can derive epsilon
                    if (allEpsilonAfter) 
                    {
                        int Aidx = A;
                        for (int k = 0; FOLLOW[Aidx][k] != '\0'; ++k) 
                        {
                            if (addToSet(FOLLOW[Bidx], FOLLOW[Aidx][k])) changed = 1;
                        }
                    }
                } // end for non-terminal in rhs
            } // end for each production
        } // each for each non-terminal in lhs
    }
}

void firstOfString(char *str, char result[])
{
    result[0] = '\0';
    int allEpsilon = 1;

    for (int i = 0; str[i] != '\0'; ++i)
    {
        char X = str[i];
        if (!isNonTerminal(X))
        {
            addToSet(result, X);
            allEpsilon = 0;
            break;
        }
        else
        {
            int idx = X - 'A';
            for (int k = 0; FIRST[idx][k] != '\0'; ++k)
            {
                char t = FIRST[idx][k];
                if (t != '#') addToSet(result, t);
            }
            if (firstHasEpsilon(idx)) continue;
            else { allEpsilon = 0; break; }
        }
    }
    if (allEpsilon) addToSet(result, '#');
}

void constructParsingTable()
{
    // initialize
    for (int i = 0; i < 26; ++i)
        for (int j = 0; j < 128; ++j)
            parsingTable[i][j][0] = '\0';

    for (int A = 0; A < 26; ++A)
    {
        if (grammar[A].lhs == 0) continue;

        for (int r = 0; r < grammar[A].count; ++r)
        {
            char *rhs = grammar[A].rhs[r];
            char firstRHS[MAX_LEN];
            firstOfString(rhs, firstRHS);

            for (int i = 0; firstRHS[i] != '\0'; ++i)
            {
                char a = firstRHS[i];
                if (a == '#') continue;
                strcpy(parsingTable[A][(int)a], rhs);
            }

            if (strchr(firstRHS, '#'))
            {
                for (int i = 0; FOLLOW[A][i] != '\0'; ++i)
                {
                    char b = FOLLOW[A][i];
                    strcpy(parsingTable[A][(int)b], rhs);
                }
            }
        }
    }
}

void printParsingTable()
{
    printf("\nLL(1) Parsing Table:\n");
    printf("-----------------------------------------\n");

    for (int A = 0; A < 26; ++A)
    {
        if (grammar[A].lhs == 0) continue;

        printf("\nFor Non-Terminal %c:\n", grammar[A].lhs);
        for (int t = 0; t < 128; ++t)
        {
            if (parsingTable[A][t][0] != '\0')
            {
                if (t == '$')
                    printf("  %-3c : %c -> %s\n", '$', grammar[A].lhs, parsingTable[A][t]);
                else if (isprint(t))
                    printf("  %-3c : %c -> %s\n", t, grammar[A].lhs, parsingTable[A][t]);
            }
        }
    }
}

int main() 
{
    inputGrammar();
    computeAllFirst();
    computeFollow();
    constructParsingTable();
    printParsingTable();

    return 0;
}
