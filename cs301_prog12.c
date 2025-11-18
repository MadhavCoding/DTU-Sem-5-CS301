#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_MAX 200
#define TOK_MAX 64
#define TOKLEN  64

int tempCount = 0;

char *newTemp() 
{
    char *b = malloc(16);
    sprintf(b, "t%d", ++tempCount);
    return b;
}

void trim(char *s) 
{
    // remove leading/trailing whitespace
    char *p = s;
    while (isspace((unsigned char)*p)) p++;
    memmove(s, p, strlen(p) + 1);
    while (strlen(s) && isspace((unsigned char)s[strlen(s)-1]))
        s[strlen(s)-1] = '\0';
}

// Symbol Table
typedef struct 
{
    char name[64];
    char type[16];
    char value[128]; // store initializer or expression text
} Symbol;

Symbol symTable[200];
int symCount = 0;

int findSymbol(const char *name) 
{
    for (int i = 0; i < symCount; ++i) 
    {
        if (strcmp(symTable[i].name, name) == 0) return i;
    }
    return -1;
}

void addOrUpdateSymbol(char *name, char *type, char *value) 
{
    int idx = findSymbol(name);
    if (idx >= 0) 
    {
        if (type && type[0]) strncpy(symTable[idx].type, type, sizeof(symTable[idx].type)-1);
        if (value && value[0]) strncpy(symTable[idx].value, value, sizeof(symTable[idx].value)-1);
    } 
    else if (symCount < (int)(sizeof(symTable)/sizeof(symTable[0]))) 
    {
        strncpy(symTable[symCount].name, name, sizeof(symTable[symCount].name)-1);
        symTable[symCount].name[sizeof(symTable[symCount].name)-1] = '\0';

        if (type) strncpy(symTable[symCount].type, type, sizeof(symTable[symCount].type)-1);
        else symTable[symCount].type[0] = '\0';

        if (value) strncpy(symTable[symCount].value, value, sizeof(symTable[symCount].value)-1);
        else symTable[symCount].value[0] = '\0';

        symCount++;
    }
}

void printSymbolTable() 
{
    printf("\nSymbol Table:\n");
    printf("-------------------------------------------------\n");
    printf("%-15s %-15s %-15s\n", "Name", "Type", "Value");
    printf("-------------------------------------------------\n");
    for (int i = 0; i < symCount; ++i) 
    {
        char *val = symTable[i].value[0] ? symTable[i].value : "-";
        char *ty  = symTable[i].type[0] ? symTable[i].type : "-";
        printf("%-15s %-15s %-15s\n", symTable[i].name, ty, val);
    }
    printf("\n");
}

// Expression tokenization
int tokenize_expr(const char *expr, char tokens[][TOKLEN]) 
{
    int idx = 0;
    int i = 0, n = strlen(expr);
    while (i < n) {
        if (isspace((unsigned char)expr[i])) { i++; continue; }
        if (isalpha((unsigned char)expr[i]) || expr[i] == '_') 
        {
            int j = 0;
            while (i < n && (isalnum((unsigned char)expr[i]) || expr[i] == '_'))
                tokens[idx][j++] = expr[i++];
            tokens[idx][j] = '\0'; idx++;
            continue;
        }
        if (isdigit((unsigned char)expr[i])) 
        {
            int j = 0;
            while (i < n && (isdigit((unsigned char)expr[i]) || expr[i]=='.'))
                tokens[idx][j++] = expr[i++];
            tokens[idx][j] = '\0'; idx++;
            continue;
        }
        tokens[idx][0] = expr[i++]; tokens[idx][1] = '\0'; idx++;
    }
    return idx;
}

// Operator helpers
int prec(char op) 
{
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

int is_op_char(const char *t) 
{
    return (strlen(t) == 1) && (strchr("+-*/", t[0]) != NULL);
}

// infix to RPN
int infix_to_rpn(char tokens[][TOKLEN], int ntok, char out[][TOKLEN]) 
{
    char stack[TOK_MAX][TOKLEN];
    int top = -1;
    int outc = 0;
    for (int i = 0; i < ntok; ++i) 
    {
        char *tk = tokens[i];
        if (is_op_char(tk)) 
        {
            while (top >= 0 && is_op_char(stack[top]) && (prec(stack[top][0]) >= prec(tk[0]))) 
            {
                strcpy(out[outc++], stack[top--]);
            }
            strcpy(stack[++top], tk);
        } 
        else if (strcmp(tk, "(") == 0) 
        {
            strcpy(stack[++top], tk);
        } 
        else if (strcmp(tk, ")") == 0) 
        {
            while (top >= 0 && strcmp(stack[top], "(") != 0) 
            {
                strcpy(out[outc++], stack[top--]);
            }
            if (top >= 0 && strcmp(stack[top], "(") == 0) top--;
        } 
        else 
        {
            strcpy(out[outc++], tk);
        }
    }
    while (top >= 0) 
    {
        strcpy(out[outc++], stack[top--]);
    }
    return outc;
}

// Evaluate RPN & emit 3AC
char *rpn_eval_emit(char rpn[][TOKLEN], int rcount, FILE *out) 
{
    char stack[TOK_MAX][TOKLEN];
    int top = -1;
    for (int i = 0; i < rcount; ++i) 
    {
        char *tk = rpn[i];
        if (is_op_char(tk)) 
        {
            char op2[TOKLEN], op1[TOKLEN];
            strcpy(op2, stack[top--]);
            strcpy(op1, stack[top--]);
            char *t = newTemp();
            const char *opcode =
                (tk[0] == '+') ? "ADD" :
                (tk[0] == '-') ? "SUB" :
                (tk[0] == '*') ? "MUL" :
                (tk[0] == '/') ? "DIV" : "OP";
            fprintf(out, "%s %s, %s, %s\n", opcode, t, op1, op2);
            strcpy(stack[++top], t);
        } 
        else 
        {
            strcpy(stack[++top], tk);
        }
    }
    return strdup(stack[top]);
}

// Handlers
void handle_declaration(char *line, FILE *out) 
{
    char *p = line;
    char type[32], name[64];
    if (sscanf(p, "%31s %63[^;= \t\n]", type, name) < 1) return;

    char *eq = strchr(p, '=');
    if (!eq) 
    {
        trim(name);
        // add to symbol table as uninitialized
        addOrUpdateSymbol(name, type, "-");
        return;
    }

    char rhs[LINE_MAX];
    char *semi = strchr(eq, ';');
    if (!semi) semi = p + strlen(p);
    int len = semi - (eq + 1);
    if (len <= 0) {
        trim(name);
        addOrUpdateSymbol(name, type, "-");
        return;
    }
    strncpy(rhs, eq + 1, len);
    rhs[len] = '\0'; trim(rhs);

    char toks[TOK_MAX][TOKLEN], rpn[TOK_MAX][TOKLEN];
    int nt = tokenize_expr(rhs, toks);
    int nr = infix_to_rpn(toks, nt, rpn);
    char *val = rpn_eval_emit(rpn, nr, out);
    trim(name);
    fprintf(out, "MOV %s, %s\n", name, val);

    // add to symbol table
    addOrUpdateSymbol(name, type, rhs);
    free(val);
}

void handle_assignment(char *line, FILE *out) 
{
    char left[64];
    char *eq = strchr(line, '=');
    if (!eq) return;
    int i = 0;
    while (i < (eq - line) && isspace((unsigned char)line[i])) i++;
    int j = 0;
    while (i < (eq - line) && !isspace((unsigned char)line[i]))
        left[j++] = line[i++];
    left[j] = '\0';

    char rhs[LINE_MAX];
    char *semi = strchr(eq, ';');
    if (!semi) semi = line + strlen(line);
    int len = semi - (eq + 1);
    if (len <= 0) return;
    strncpy(rhs, eq + 1, len);
    rhs[len] = '\0'; trim(rhs);

    char toks[TOK_MAX][TOKLEN], rpn[TOK_MAX][TOKLEN];
    int nt = tokenize_expr(rhs, toks);
    int nr = infix_to_rpn(toks, nt, rpn);
    char *val = rpn_eval_emit(rpn, nr, out);
    trim(left);
    fprintf(out, "MOV %s, %s\n", left, val);

    // update symbol table, if exists update, else add with unknown type
    addOrUpdateSymbol(left, "-", rhs);
    free(val);
}

void handle_printf(char *line, FILE *out) 
{
    char var[64];
    char *p = strstr(line, "printf");
    if (!p) return;
    char *comma = strchr(p, ',');
    if (!comma) return;
    char *rp = strchr(comma, ')');
    if (!rp) rp = line + strlen(line);
    int len = rp - (comma + 1);
    if (len <= 0) return;
    strncpy(var, comma + 1, len);
    var[len] = '\0';
    trim(var);
    if (var[strlen(var)-1] == ';') var[strlen(var)-1] = '\0';
    char first[64]; sscanf(var, "%63[^, \t]", first);
    trim(first);
    fprintf(out, "PRINT %s\n", first);
}

// each line proccessing
void process_line(char *line, FILE *out) 
{
    trim(line);
    if (strlen(line) == 0) return;
    if (strstr(line, "main") != NULL) return;

    if (strncmp(line, "int ", 4) == 0 || strncmp(line, "char ", 5) == 0 || strncmp(line, "float ", 6) == 0) 
    {
        handle_declaration(line, out);
    } 
    else if (strncmp(line, "printf", 6) == 0) 
    {
        handle_printf(line, out);
    } 
    else if (strchr(line, '=') ) 
    {
        handle_assignment(line, out);
    }
}

int main() 
{
    char lines[100][LINE_MAX];
    int n = 0;

    printf("Enter a C program :\n");
    while (fgets(lines[n], sizeof(lines[n]), stdin)) 
    {
        n++;
        if (n >= 100) break;
    }

    printf("\n\n--- 3 Address Code ---\n");
    FILE *out = stdout;
    for (int i = 0; i < n; i++) 
    {
        process_line(lines[i], out);
    }

    printSymbolTable();

    return 0;
}
