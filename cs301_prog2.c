#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STATES 20
#define MAX_SYMBOLS 26

int n, m; // n = number of DFA states, m = number of symbols
int start_state;
int nf;

// DFA storage
int dfa_trans[MAX_STATES][MAX_SYMBOLS]; 
int visited_dfa[MAX_STATES];
int final_dfa[MAX_STATES];

// Input String
char *input_str;
int input_sz;

void print_table() 
{
    int rows = n;

    printf("\nDFA Transition Table:\n");
    printf("%-15s", "State");
    for (int sym = 0; sym < m; sym++)
        printf("%-15c", sym + 'a');
    printf("%-15s\n", "Final");

    for (int i = 0; i < rows; i++) 
    {
        char buf[64];
        sprintf(buf, "q%d", i);
        printf("%-15s", buf);

        for (int sym = 0; sym < m; sym++) 
        {
            char cell[64];
            sprintf(cell, "q%d", dfa_trans[i][sym]);
            printf("%-15s", cell);
        }
        printf("%-15s\n", final_dfa[i] ? "Yes" : "No");
    }
}

// Input DFA
void input_dfa() 
{
    printf("No. of states in DFA: ");
    scanf("%d", &n);
    printf("No. of input symbols: ");
    scanf("%d", &m);

    memset(dfa_trans, 0, sizeof(dfa_trans));
    memset(final_dfa, 0, sizeof(final_dfa));

    printf("\nDFA Transitions:\n");
    for (int i = 0; i < n; i++) 
    {
        printf("State q%d:\n", i);
        for (int j = 0; j < m; j++) 
        {
            printf("Transitions (q%d, %d): ", i, j);
            int ns;
            scanf("%d", &ns);
            dfa_trans[i][j] = ns;
        }
    }

    printf("\nStart State: ");
    scanf("%d", &start_state);

    printf("\nNo. of final states: ");
    scanf("%d", &nf);
    printf("Final states: ");
    for (int i = 0; i < nf; i++) 
    {
        int fs;
        scanf("%d", &fs);
        final_dfa[fs] = 1;
    }
}

void input_string()
{
    printf("\nSize of Input String : ");
    scanf("%d", &input_sz);

    input_str = (char *)malloc((input_sz + 1) * sizeof(char));

    int pos = 0;
    for(int i = 0; i < input_sz; i++)
    {
        char c;
        scanf(" %c", &c);
        input_str[pos++] = c;
    }

    input_str[pos] = '\0';
}

int is_string_accepted()
{
    int curr_state = start_state;
    for(int i = 0; i < input_sz; i++)
    {
        int sym = input_str[i] - 'a';

        if(sym < 0 || sym >= m)
        {
            return 0;
        }

        int next_state = dfa_trans[curr_state][sym];
        curr_state = next_state;
    }

    if(final_dfa[curr_state] == 1) return 1;
    else return 0;
}

void check_string_accepted()
{
    if(is_string_accepted() == 1)
    {
        printf("\nThe given string is accepted by the DFA\n");
    }
    else
    {
        printf("\nThe given string is not accepted by the DFA\n");
    }
}

int main() 
{
    input_dfa();
    print_table();
    input_string();
    check_string_accepted();
    return 0;
}
