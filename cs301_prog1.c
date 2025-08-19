#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STATES 20
#define MAX_SYMBOLS 26

int n, m; // n = number of NFA states, m = number of symbols
int start_state;
int nf;

// NFA storage
int nfa_trans[MAX_STATES][MAX_SYMBOLS]; 
int visited_nfa[MAX_STATES];
int final_nfa[MAX_STATES];

// DFA storage
int dfa_trans[1 << MAX_STATES][MAX_SYMBOLS]; 
int visited_dfa[1 << MAX_STATES];
int final_dfa[1 << MAX_STATES];

void make_set(int mask, char *out, int n) {
    int first = 1, pos = 0;
    out[pos++] = '{';
    for (int k = 0; k < n; k++) {
        if (mask & (1 << k)) {
            if (!first) out[pos++] = ',';
            pos += sprintf(out + pos, "q%d", k);
            first = 0;
        }
    }
    out[pos++] = '}';
    out[pos] = '\0';
}

void print_table(int is_nfa) 
{
    int rows = is_nfa ? n : (1 << n);
    int (*trans)[MAX_SYMBOLS] = is_nfa ? nfa_trans : dfa_trans;
    int *visited_arr = is_nfa ? visited_nfa : visited_dfa;
    int *final_arr   = is_nfa ? final_nfa   : final_dfa;

    printf("\n%s Transition Table:\n", is_nfa ? "NFA" : "DFA");
    printf("%-15s", "State");
    for (int sym = 0; sym < m; sym++)
        printf("%-15d", sym);
    printf("%-15s\n", "Final");

    for (int i = 0; i < rows; i++) 
    {
        if (!visited_arr[i]) continue;

        char buf[64];
        make_set(is_nfa ? (1 << i) : i, buf, n);
        printf("%-15s", buf);

        for (int sym = 0; sym < m; sym++) 
        {
            char cell[64];
            make_set(trans[i][sym], cell, n);
            printf("%-15s", cell);
        }
        printf("%-15s\n", final_arr[i] ? "Yes" : "No");
    }
}

int is_final_mask(int mask) 
{
    for (int i = 0; i < n; i++)
        if ((mask & (1 << i)) && final_nfa[i]) return 1;
    return 0;
}

// Input NFA
void input_nfa() 
{
    printf("No. of states in NFA: ");
    scanf("%d", &n);
    printf("No. of input symbols: ");
    scanf("%d", &m);

    memset(nfa_trans, 0, sizeof(nfa_trans));
    memset(visited_nfa, 0, sizeof(visited_nfa));
    memset(final_nfa, 0, sizeof(final_nfa));

    printf("\nNFA Transitions:\n");
    for (int i = 0; i < n; i++) 
    {
        visited_nfa[i] = 1;
        printf("State q%d:\n", i);
        for (int j = 0; j < m; j++) 
        {
            printf("No. of transitions for (q%d, %d): ", i, j);
            int k;
            scanf("%d", &k);
            printf("Transitions: ");
            for (int x = 0; x < k; x++) 
            {
                int ns;
                scanf("%d", &ns);
                nfa_trans[i][j] |= (1 << ns);
            }
            if (k == 0) printf("\n");
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
        final_nfa[fs] = 1;
    }
}

void nfa_to_dfa() 
{
    memset(visited_dfa, 0, sizeof(visited_dfa));
    memset(final_dfa, 0, sizeof(final_dfa));

    int queue[1 << MAX_STATES], front = 0, rear = 0;

    int start_mask = 1 << start_state;
    queue[rear++] = start_mask;
    visited_dfa[start_mask] = 1;
    if (is_final_mask(start_mask)) final_dfa[start_mask] = 1;

    while (front < rear) 
    {
        int curr_mask = queue[front++];
        for (int sym = 0; sym < m; sym++) 
        {
            int next_mask = 0;
            for (int i = 0; i < n; i++)
                if (curr_mask & (1 << i))
                    next_mask |= nfa_trans[i][sym];

            dfa_trans[curr_mask][sym] = next_mask;

            if (next_mask && !visited_dfa[next_mask]) 
            {
                visited_dfa[next_mask] = 1;
                if (is_final_mask(next_mask)) final_dfa[next_mask] = 1;
                queue[rear++] = next_mask;
            }
        }
    }
}

int main() 
{
    input_nfa();
    print_table(1); // NFA table
    nfa_to_dfa();
    print_table(0); // DFA table
    return 0;
}