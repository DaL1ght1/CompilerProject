/*
  _  ___    _          _      _____ _        _  _______  _____ ______ _____
 | |/ / |  | |   /\   | |    |_   _| |      | |/ /  __ \|_   _|  ____|_   _|
 | ' /| |__| |  /  \  | |      | | | |      | ' /| |__) | | | | |__    | |
 |  < |  __  | / /\ \ | |      | | | |      |  < |  _  /  | | |  __|   | |
 | . \| |  | |/ ____ \| |____ _| |_| |____  | . \| | \ \ _| |_| |     _| |_
 |_|\_\_|  |_/_/    \_\______|_____|______| |_|\_\_|  \_\_____|_|    |_____|

  ___ ___ ____         _
 |_ _|_ _|___ \       / \
  | | | |  __) |____ / _ \
  | | | | / __/_____/ ___ \
 |___|___|_____|   /_/   \_\

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define program 1
#define begin 2
#define END 3
#define id 4
#define comment 5
#define nb 6
#define aff 7
#define po 8
#define pf 9
#define pv 10
#define dp 11
#define IF 12
#define THEN 13
#define ENDIF 14
#define VAR 15
#define INT 16
#define oparith 17
#define oprel 18
#define writeln 19
#define readln 20
#define point 21
#define virg 22

#define MAX_LEXEME_LENGTH 50
#define MAX_ERROR_LENGTH 100
#define MAX_ERRORS 8
#define NB_SymboleS 100

struct
{
    const char *keyword;
    int token_code;
} keywords[] = {
    {"program", program},
    {"begin", begin},
    {"end", END},
    {"if", IF},
    {"then", THEN},
    {"endif", ENDIF},
    {"var", VAR},
    {"int", INT},
    {"writeln", writeln},
    {"readln", readln},
    {NULL, 0}};

struct
{
    char *name;
    int code;
} IdentTab[NB_SymboleS] = {
    {"Program", program},
    {"Begin", begin},
    {"End", END},
    {"If", IF},
    {"Then", THEN},
    {"Endif", ENDIF},
    {"Var", VAR},
    {"Int", INT},
    {"Writeln", writeln},
    {"Readln", readln},
    {NULL, 0}};

typedef enum
{
    PUSH,
    VALUE,
    STORE,
    ADD,
    SUB,
    MUL,
    DIV,
    ASSIGN,
    COMP_LT,
    COMP_GT,
    COMP_LE,
    COMP_GE,
    COMP_EQ,
    COMP_NE,
    GO_FALSE,
    GO_TRUE,
    GOTO,
    LABEL,
    READ,
    WRITE,
} InstructionType;

typedef enum
{
    TYPE_INT,
    TYPE_UNKNOWN
} DataType;

typedef struct
{
    int code;
    char name[MAX_LEXEME_LENGTH];
    int value;
} Token;

typedef struct
{
    char *name;
    DataType type;
    int isDeclared;
    int isInitialized;
    int value;
    int line;
} identifierEntry;

typedef struct
{
    identifierEntry *entries;
    int size;
    int capacity;
} IdentifierTable;

typedef struct
{
    InstructionType type;
    char operand[50];
} Instruction;

typedef struct
{
    Instruction *instructions;
    int size;
    int capacity;
    int labelCount;
} StackCode;

// Global variables//
StackCode code;
IdentifierTable identifierTable;
char lexeme[MAX_LEXEME_LENGTH];
int line_number = 1;
int error_count = 0;
char name[MAX_LEXEME_LENGTH];
FILE *input_file = NULL;
Token token;
int currentIdentIndex = 10;

// Additional functions//
char ReadLetter(void);
char SkipWhiteSpace(void);
const char *CodeToKeyword(int);
void Safe_Strcpy(char *, const char *, size_t);
int Isnst(void);
void Error(const char *message);
void AddToSymbolesTable(char *, int);
void resetSymboleTable()
{

    for (int i = 10; i < currentIdentIndex; i++)
    {
        if (IdentTab[i].name != NULL)
        {
            free(IdentTab[i].name);
            IdentTab[i].name = NULL;
            IdentTab[i].code = 0;
        }
    }
    currentIdentIndex = 10;
}
void PrintSymboleTable(void);
InstructionType getComparisonType(const char *op)
{
    if (strcmp(op, "<") == 0)
        return COMP_LT;
    if (strcmp(op, ">") == 0)
        return COMP_GT;
    if (strcmp(op, "<=") == 0)
        return COMP_LE;
    if (strcmp(op, ">=") == 0)
        return COMP_GE;
    if (strcmp(op, "=") == 0)
        return COMP_EQ;
    if (strcmp(op, "<>") == 0)
        return COMP_NE;
    return COMP_EQ;
}

// Grammar functions//
void P(void);
void Dcl(void);
void ListId(void);
void ListIdComp(void);
void ListInst(void);
void ListInstComp(void);
void I(void);
void C(void);
void Exp(void);
void ExpComp(void);

// Accept and Next functions//
void Accept(int expected_token);
Token Next(void);

// identifier table functions//
void initidentifierTable(void);
void freeidentifierTable();
identifierEntry *lookupidentifier(const char *name);
void addidentifier(const char *name, DataType type, int line);
void printidentifierTable();

// Semantic Analysis functions//
void semanticError(const char *message, int line);
void semanticP();
void semanticDcl(const char *varName);
void semanticAssignment(const char *varName);
void semanticExpression(const char *varName);
void semanticReadln(const char *);
void semanticWriteln(const char *);

// Intermediate code functions//
void initStackCode();
char *newStackLabel();
void emitStack(InstructionType type, const char *operand);
void printStackCode();
void generateAssignment(const char *target, const char *arg1, const char *arg2);
void generateIfStatement(const char *condition_var, const char *constant,
                         const char *write_var);
void cleanupStackCode();

// Main function//

int main()
{
    char filename[256];
    char retry = 'y';
    int choice;

    do
    {
        error_count = 0;
        printf("\nEnter the name of your file: ");
        if (scanf("%255s", filename) != 1)
        {
            printf("Error reading filename\n");
            return 1;
        }
        while (getchar() != '\n')
            ;

        input_file = fopen(filename, "r");
        if (input_file == NULL)
        {
            printf("Error: Cannot open file '%s'\n", filename);
            printf("Do you want to try another file? (y/n): ");
            scanf(" %c", &retry);
            while (getchar() != '\n')
                ;

            if (retry != 'y' && retry != 'Y')
            {
                printf("Program terminated.\n");
                return 1;
            }
            continue;
        }

        printf("File '%s' opened successfully!\n", filename);
        line_number = 1;
        initStackCode();
        token = Next();
        if (token.code == -1)
        {
            printf("Error getting first token\n");
            fclose(input_file);
            return 1;
        }

        P();
        fclose(input_file);

        if (error_count == 0)
        {
            printf("\nParsing completed successfully!\n");

            do
            {
                printf("\nMenu:\n");
                printf("1. Show Table of Symboles\n");
                printf("2. Show Table of identifiers\n");
                printf("3. Show Intermediate Code\n");
                printf("4. Compile another file\n");
                printf("5. Exit\n");
                printf("Enter your choice (1-5): ");

                if (scanf("%d", &choice) != 1)
                {
                    while (getchar() != '\n')
                        ;
                    printf("Invalid input. Please enter a number between 1 and 5.\n");
                    continue;
                }
                while (getchar() != '\n')
                    ;

                switch (choice)
                {
                case 1:
                    printf("\nTable of Symboles:\n");
                    PrintSymboleTable();
                    resetSymboleTable();
                    break;

                case 2:
                    printf("\nTable of identifiers:\n");
                    printidentifierTable();
                    freeidentifierTable();
                    break;

                case 3:
                    printStackCode();
                    cleanupStackCode();
                    break;

                case 4:

                    retry = 'y';
                    break;

                case 5:
                    printf("Exiting program...\n");
                    retry = 'n';
                    break;

                default:
                    printf("Invalid choice. Please enter a number between 1 and 5.\n");
                    break;
                }
            } while (choice != 4 && choice != 5);
        }
        else
        {
            printf("\nParsing completed with %d errors.\n", error_count);
            printf("Do you want to try another file? (y/n): ");
            scanf(" %c", &retry);
            while (getchar() != '\n')
                ;
        }

    } while (retry == 'y' || retry == 'Y');

    resetSymboleTable();
    cleanupStackCode();
    freeidentifierTable();
    return 0;
}

// Additional functions implementations//
char ReadLetter()
{
    char c = fgetc(input_file);
    // printf("c=%c \n", c);
    return c;
}
char SkipWhiteSpace(void)
{
    char c;
    while ((c = ReadLetter()) != EOF && isspace(c))
    {
        if (c == '\n')
            line_number++;
    }

    return c;
}
const char *CodeToKeyword(int code)
{
    switch (code)
    {
    case program:
        return "program";
    case begin:
        return "begin";
    case END:
        return "end";
    case id:
        return "Symbole";
    case comment:
        return "comment";
    case nb:
        return "number";
    case aff:
        return ":=";
    case po:
        return "(";
    case pf:
        return ")";
    case pv:
        return ";";
    case dp:
        return ":";
    case IF:
        return "if";
    case THEN:
        return "then";
    case ENDIF:
        return "endif";
    case VAR:
        return "var";
    case INT:
        return "int";
    case oparith:
        return "arithmetic_operator";
    case oprel:
        return "relational_operator";
    case writeln:
        return "writeln";
    case readln:
        return "readln";
    case point:
        return ".";
    case virg:
        return ",";
    default:
        return "unknown";
    }
}
void Safe_Strcpy(char *dest, const char *src, size_t size)
{
    if (size > 0)
    {
        strncpy(dest, src, size - 1);
        dest[size - 1] = '\0';
    }
}
int Isnst()
{
    return token.code == id || token.code == writeln ||
           token.code == readln || token.code == IF;
}
void Error(const char *message)
{
    error_count++;
    fprintf(stderr, "Error at line %d: %s\n", line_number, message);

    if (error_count >= MAX_ERRORS)
    {
        fprintf(stderr, "Too many errors. Stopping compilation.\n");
        if (input_file)
        {
            fclose(input_file);
        }
        exit(1);
    }
}
void AddToSymbolesTable(char *name, int code)
{
    if (currentIdentIndex >= NB_SymboleS - 1)
    {
        printf("Error: Symbole table is full\n");
        return;
    }
    for (int i = 0; i <= currentIdentIndex; i++)
    {
        if (IdentTab[i].name != NULL && strcmp(IdentTab[i].name, name) == 0)
        {
            return;
        }
    }

    IdentTab[currentIdentIndex].name = strdup(name);
    IdentTab[currentIdentIndex].code = code;
    currentIdentIndex++;

    IdentTab[currentIdentIndex].name = NULL;
    IdentTab[currentIdentIndex].code = 0;
}
void PrintSymboleTable(void)
{
    printf("\nSymbole Table:\n");
    printf("    +----------------------+-------+\n");
    printf("    | %-20s | %-5s |\n", "Name", "Code");
    printf("    +----------------------+-------+\n");

    for (int i = 0; IdentTab[i].name != NULL; i++)
    {
        printf("    | %-20s | %-5d |\n", IdentTab[i].name, IdentTab[i].code);
        printf("    +----------------------+-------+\n");
    }
}

// Grammar functions implementation//
void P()
{
    if (token.code == program)
    {
        semanticP();
        Accept(program);
        Accept(id);
        Accept(pv);
        Dcl();
        Accept(begin);
        ListInst();
        Accept(END);
        Accept(point);
    }
}
void Dcl()
{
    if (token.code == VAR)
    {
        Accept(VAR);
        ListId();
        Accept(dp);
        Accept(INT);
        Accept(pv);
        Dcl();
    }
}
void ListId()
{
    if (token.code == id)
    {
        char varName[MAX_LEXEME_LENGTH];
        strncpy(varName, token.name, MAX_LEXEME_LENGTH - 1);
        varName[MAX_LEXEME_LENGTH - 1] = '\0';
        semanticDcl(varName);
        Accept(id);
        ListIdComp();
    }
}
void ListIdComp()
{
    if (token.code == virg)
    {
        Accept(virg);
        ListId();
    }
}
void ListInst()
{
    if (Isnst())
    {

        I();
        ListInstComp();
    }
}
void ListInstComp()
{
    if (token.code != END && token.code != ENDIF)
    {
        ListInst();
    }
}
void I()
{
    switch (token.code)
    {
    case id:
    {
        char varName[MAX_LEXEME_LENGTH];
        strncpy(varName, token.name, MAX_LEXEME_LENGTH - 1);
        varName[MAX_LEXEME_LENGTH - 1] = '\0';
        Accept(id);
        Accept(aff);
        semanticAssignment(varName);
        emitStack(STORE, varName);
        Token peek = token;
        if (peek.code == nb)
        {
            // Case: x := 2
            Accept(nb);
            emitStack(PUSH, peek.name);
        }
        else if (peek.code == id)
        {
            // Case: x := y or start of x := y + z
            Accept(id);
            emitStack(VALUE, peek.name);

            if (token.code == oparith)
            {
                // Case: x := y + z
                char op = token.name[0];
                Accept(oparith);
                peek = token;
                Accept(id);
                emitStack(VALUE, peek.name);
                switch (op)
                {
                case '+':
                    emitStack(ADD, NULL);
                    break;
                case '-':
                    emitStack(SUB, NULL);
                    break;
                case '*':
                    emitStack(MUL, NULL);
                    break;
                case '/':
                    emitStack(DIV, NULL);
                    break;
                }
            }
        }
        emitStack(ASSIGN, NULL);
        Accept(pv);
        break;
    }
    case writeln:
        Accept(writeln);
        Accept(po);
        if (token.code == id)
        {
            char varName[MAX_LEXEME_LENGTH];
            strncpy(varName, token.name, MAX_LEXEME_LENGTH - 1);
            varName[MAX_LEXEME_LENGTH - 1] = '\0';
            printf("%s\n", varName);
            semanticWriteln(varName);
            emitStack(VALUE, varName);
            emitStack(WRITE, NULL);
            Accept(id);
        }
        Accept(pf);
        Accept(pv);
        break;

    case readln:
        Accept(readln);
        Accept(po);
        if (token.code == id)
        {
            char varName[MAX_LEXEME_LENGTH];
            strncpy(varName, token.name, MAX_LEXEME_LENGTH - 1);
            varName[MAX_LEXEME_LENGTH - 1] = '\0';
            printf("%s\n", varName);
            semanticReadln(varName);
            emitStack(READ, varName);
            Accept(id);
        }
        Accept(pf);
        Accept(pv);
        break;

    case IF:
        Accept(IF);
        char *falseLabel = newStackLabel();
        char *endLabel = newStackLabel();

        C();
        emitStack(GO_FALSE, falseLabel);

        Accept(THEN);
        ListInst();

        emitStack(GOTO, endLabel);
        emitStack(LABEL, falseLabel);

        Accept(ENDIF);
        emitStack(LABEL, endLabel);

        free(falseLabel);
        free(endLabel);
        break;
    }
}
void C()
{
    Exp();
    char op[3];
    strncpy(op, token.name, sizeof(op) - 1);
    op[sizeof(op) - 1] = '\0';
    Accept(oprel);
    Exp();
    emitStack(getComparisonType(op), NULL);
}
void Exp()
{
    switch (token.code)
    {
    case id:
    {
        char varName[MAX_LEXEME_LENGTH];
        strncpy(varName, token.name, MAX_LEXEME_LENGTH - 1);
        varName[MAX_LEXEME_LENGTH - 1] = '\0';
        semanticExpression(varName);
        emitStack(VALUE, varName);
        Accept(id);
        ExpComp();
        break;
    }
    case nb:
        emitStack(PUSH, token.name);
        Accept(nb);
        ExpComp();
        break;
    case po:
        Accept(po);
        Exp();
        Accept(pf);
        ExpComp();
        break;
    default:
        Error("Invalid expression");
    }
}
void ExpComp()
{
    if (token.code == oparith)
    {
        char op = token.name[0];
        Accept(oparith);
        Exp();

        switch (op)
        {
        case '+':
            emitStack(ADD, NULL);
            break;
        case '-':
            emitStack(SUB, NULL);
            break;
        case '*':
            emitStack(MUL, NULL);
            break;
        case '/':
            emitStack(DIV, NULL);
            break;
        }

        ExpComp();
    }
}

// identifier table functions implementation//
void initidentifierTable()
{
    identifierTable.capacity = 100;
    identifierTable.size = 0;
    identifierTable.entries = (identifierEntry *)malloc(identifierTable.capacity * sizeof(identifierEntry));
}
void freeidentifierTable()
{
    for (int i = 0; i < identifierTable.size; i++)
    {
        free(identifierTable.entries[i].name);
    }
    free(identifierTable.entries);
    identifierTable.size = 0;
    identifierTable.capacity = 0;
}
identifierEntry *lookupidentifier(const char *name)
{
    for (int i = 0; i < identifierTable.size; i++)
    {
        if (strcmp(identifierTable.entries[i].name, name) == 0)
        {
            return &identifierTable.entries[i];
        }
    }
    return NULL;
}
void addidentifier(const char *name, DataType type, int line)
{
    if (identifierTable.size >= identifierTable.capacity)
    {
        // Expand table if needed
        identifierTable.capacity *= 2;
        identifierTable.entries = (identifierEntry *)realloc(identifierTable.entries,
                                                             identifierTable.capacity * sizeof(identifierEntry));
    }

    identifierEntry *entry = &identifierTable.entries[identifierTable.size++];
    entry->name = strdup(name);
    entry->type = type;
    entry->isDeclared = 1;
    entry->isInitialized = 0;
    entry->value = 0;
    entry->line = line;
}
void printidentifierTable()
{
    printf("\nidentifier Table Contents:\n");
    printf("+-----------------------------------------------------------------+\n");
    printf("| %-20s | %-10s | %-12s | %-12s |\n", "Name", "Type", "Declared", "Initialized");
    printf("+-----------------------------------------------------------------+\n");

    for (int i = 0; i < identifierTable.size; i++)
    {
        printf("| %-20s | %-10s | %-12s | %-12s |\n",
               identifierTable.entries[i].name,
               identifierTable.entries[i].type == TYPE_INT ? "INT" : "UNKNOWN",
               identifierTable.entries[i].isDeclared ? "YES" : "NO",
               identifierTable.entries[i].isInitialized ? "YES" : "NO");
    }

    printf("+-----------------------------------------------------------------+\n");
}

// Semantic Analysis functions implementation//
void semanticError(const char *message, int line)
{
    fprintf(stderr, "Semantic Error at line %d: %s\n", line, message);
    error_count++;
}
void semanticP()
{
    initidentifierTable();
}
void semanticDcl(const char *varName)
{
    if (lookupidentifier(varName) != NULL)
    {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), "Variable '%s' already declared", varName);
        semanticError(error_msg, line_number);
    }
    else
    {
        addidentifier(varName, TYPE_INT, line_number);
    }
}
void semanticAssignment(const char *varName)
{
    identifierEntry *entry = lookupidentifier(varName);
    if (entry == NULL)
    {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), "Variable '%s' used without declaration", varName);
        semanticError(error_msg, line_number);
    }
    else
    {
        entry->isInitialized = 1;
    }
}
void semanticExpression(const char *varName)
{
    if (varName != NULL)
    {
        identifierEntry *entry = lookupidentifier(varName);
        if (entry == NULL)
        {
            char error_msg[100];
            snprintf(error_msg, sizeof(error_msg), "Variable '%s' used without declaration", varName);
            semanticError(error_msg, line_number);
        }
        else if (!entry->isInitialized)
        {
            char error_msg[100];
            snprintf(error_msg, sizeof(error_msg), "Variable '%s' used without initialization", varName);
            semanticError(error_msg, line_number);
        }
    }
}
void semanticReadln(const char *varName)
{

    identifierEntry *entry = lookupidentifier(varName);
    if (entry == NULL)
    {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), "Cannot read into undeclared variable '%s'", varName);
        semanticError(error_msg, line_number);
    }
    else
    {

        entry->isInitialized = 1;
    }
}
void semanticWriteln(const char *varName)
{
    identifierEntry *entry = lookupidentifier(varName);
    if (entry == NULL)
    {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), "Cannot write undeclared variable '%s'", varName);
        semanticError(error_msg, line_number);
    }
    else if (!entry->isInitialized)
    {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), "Cannot write uninitialized variable '%s'", varName);
        semanticError(error_msg, line_number);
    }
}

// Accept and Next functions implementation//
void Accept(int expected_token)
{
    if (token.code == expected_token)
    {
        if (token.code != -5)
        {
            Token next = Next();
            if (next.code == -1)
            {
                Error("Lexical error while getting next token");
            }
            token = next;
        }
    }
    else
    {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), "Syntax error: Expected \" %s \" but got \" %s \"", CodeToKeyword(expected_token), CodeToKeyword(token.code));
        Error(error_msg);
    }
}
Token Next()
{
    Token tempToken;
    tempToken.code = -1;
    tempToken.value = 0;
    tempToken.name[0] = '\0';
    char c = SkipWhiteSpace();

    if (c == EOF)
    {
        printf("%d", EOF);
        tempToken.code = -5;
        return tempToken;
    }

    if (isalpha(c))
    {
        int i = 0;
        tempToken.name[i++] = tolower(c);

        while ((c = ReadLetter()) != EOF && isalnum(c) && i < (int)(sizeof(tempToken.name) - 1))
        {

            tempToken.name[i++] = tolower(c);
        }

        tempToken.name[i] = '\0';

        ungetc(c, input_file);
        for (int i = 0; keywords[i].keyword != NULL; i++)
        {
            if (strcmp(tempToken.name, keywords[i].keyword) == 0)
            {
                tempToken.code = keywords[i].token_code;
                return tempToken;
            }
        }

        tempToken.code = id;
        AddToSymbolesTable(tempToken.name, tempToken.code);
        return tempToken;
    }

    if (isdigit(c))
    {
        char num_buffer[MAX_LEXEME_LENGTH];
        int i = 0;
        do
        {
            if (i < MAX_LEXEME_LENGTH - 1)
            {
                num_buffer[i++] = c;
            }
            c = ReadLetter();

        } while (isdigit(c));

        if (!isspace(c) && c != EOF)
        {
            ungetc(c, input_file);
        }

        num_buffer[i] = '\0';
        tempToken.code = nb;
        Safe_Strcpy(tempToken.name, num_buffer, MAX_LEXEME_LENGTH);
        tempToken.value = atoi(num_buffer);
        return tempToken;
    }

    switch (c)
    {
    case ':':
        if ((c = ReadLetter()) == '=')
        {

            tempToken.code = aff;
            Safe_Strcpy(tempToken.name, ":=", MAX_LEXEME_LENGTH);
        }
        else
        {
            ungetc(c, input_file);
            tempToken.code = dp;
            Safe_Strcpy(tempToken.name, ":", MAX_LEXEME_LENGTH);
        }
        break;

    case '<':
    case '>':
    case '=':
    {
        char op[3] = {c, '\0', '\0'};
        if ((c = ReadLetter()) == '=')
        {
            op[1] = '=';
        }
        else
        {
            ungetc(c, input_file);
        }
        tempToken.code = oprel;
        Safe_Strcpy(tempToken.name, op, MAX_LEXEME_LENGTH);
        break;
    }

    case '+':
    case '-':
    case '*':
    case '/':
        tempToken.code = oparith;
        tempToken.name[0] = c;
        tempToken.name[1] = '\0';
        break;

    case '(':
        tempToken.code = po;
        Safe_Strcpy(tempToken.name, "(", MAX_LEXEME_LENGTH);
        break;

    case ')':
        tempToken.code = pf;
        Safe_Strcpy(tempToken.name, ")", MAX_LEXEME_LENGTH);
        break;

    case ';':
        tempToken.code = pv;
        Safe_Strcpy(tempToken.name, ";", MAX_LEXEME_LENGTH);
        break;

    case '.':
        tempToken.code = point;
        Safe_Strcpy(tempToken.name, ".", MAX_LEXEME_LENGTH);
        break;

    case ',':
        tempToken.code = virg;
        Safe_Strcpy(tempToken.name, ",", MAX_LEXEME_LENGTH);
        break;

    default:
        char error_msg[MAX_ERROR_LENGTH];
        snprintf(error_msg, MAX_ERROR_LENGTH,
                 "Invalid character: '%c' (ASCII: %d)", c, (int)c);
        Error(error_msg);
        return Next();
    }
    if (c == '(')
    {
        char nextChar = ReadLetter();
        if (nextChar == '*')
        {
            char prev = '\0';
            while ((c = ReadLetter()) != EOF)
            {
                if (c == '\n')
                    line_number++;
                if (prev == '*' && c == ')')
                    return Next();
                prev = c;
            }
            Error("Unclosed comment");
            return tempToken;
        }
        else
        {
            ungetc(nextChar, input_file);
        }
    }

    return tempToken;
}

// Intermediate code functions implementation//
void initStackCode()
{
    code.capacity = 100;
    code.size = 0;
    code.labelCount = 0;
    code.instructions = (Instruction *)malloc(code.capacity * sizeof(Instruction));
}
char *newStackLabel()
{
    char *label = (char *)malloc(20);
    sprintf(label, "L%d", ++code.labelCount);
    return label;
}
void emitStack(InstructionType type, const char *operand)
{
    if (code.size >= code.capacity)
    {
        code.capacity *= 2;
        code.instructions = (Instruction *)realloc(code.instructions,
                                                   code.capacity * sizeof(Instruction));
    }

    code.instructions[code.size].type = type;
    if (operand)
    {
        strncpy(code.instructions[code.size].operand, operand, 49);
        code.instructions[code.size].operand[49] = '\0';
    }
    else
    {
        code.instructions[code.size].operand[0] = '\0';
    }
    code.size++;
}
void printStackCode()
{
    // Print header with nice formatting
    printf("\n+-------------------------------+\n");
    printf("|     Stack-Based Instructions  |\n");
    printf("+-------------------------------+\n");
    printf("| %-29s |\n", "Instruction");
    printf("+-------------------------------+\n");

    for (int i = 0; i < code.size; i++)
    {
        Instruction instr = code.instructions[i];
        printf("| "); // Start each line with border

        switch (instr.type)
        {
        // Value operations
        case PUSH:
            printf("%-29s |\n", instr.operand ? strcat(strcat(strcpy(malloc(32), "push "), instr.operand), "") : "push");
            break;
        case VALUE:
            printf("%-29s |\n", instr.operand ? strcat(strcat(strcpy(malloc(32), "value "), instr.operand), "") : "value");
            break;
        case STORE:
            printf("%-29s |\n", instr.operand ? strcat(strcat(strcpy(malloc(32), "store "), instr.operand), "") : "store");
            break;

        // Arithmetic operators
        case ADD:
            printf("%-29s |\n", "+");
            break;
        case SUB:
            printf("%-29s |\n", "-");
            break;
        case MUL:
            printf("%-29s |\n", "*");
            break;
        case DIV:
            printf("%-29s |\n", "/");
            break;
        case ASSIGN:
            printf("%-29s |\n", ":=");
            break;

        // Comparison operators
        case COMP_LT:
            printf("%-29s |\n", "COMP_LT");
            break;
        case COMP_GT:
            printf("%-29s |\n", "COMP_GT");
            break;
        case COMP_LE:
            printf("%-29s |\n", "COMP_LE");
            break;
        case COMP_GE:
            printf("%-29s |\n", "COMP_GE");
            break;
        case COMP_EQ:
            printf("%-29s |\n", "COMP_EQ");
            break;
        case COMP_NE:
            printf("%-29s |\n", "COMP_NE");
            break;

        // Control flow
        case GO_FALSE:
            printf("%-29s |\n", instr.operand ? strcat(strcat(strcpy(malloc(32), "go_false "), instr.operand), "") : "go_false");
            break;
        case GO_TRUE:
            printf("%-29s |\n", instr.operand ? strcat(strcat(strcpy(malloc(32), "go_true "), instr.operand), "") : "go_true");
            break;
        case GOTO:
            printf("%-29s |\n", instr.operand ? strcat(strcat(strcpy(malloc(32), "goto "), instr.operand), "") : "goto");
            break;
        case LABEL:
            printf("%-29s |\n", instr.operand ? strcat(strcat(strcpy(malloc(32), ""), instr.operand), ":") : "label");
            break;

        // I/O operations
        case READ:
            printf("%-29s |\n", instr.operand ? strcat(strcat(strcpy(malloc(32), "read "), instr.operand), "") : "read");
            break;
        case WRITE:
            printf("%-29s |\n", instr.operand ? strcat(strcat(strcpy(malloc(32), "write "), instr.operand), "") : "write");
            break;

        default:
            printf("%-29s |\n", "unknown instruction");
        }
    }

    // Print footer
    printf("+-------------------------------+\n");
    printf("| Total Instructions: %-9d |\n", code.size);
    printf("+-------------------------------+\n\n");
}
void generateAssignment(const char *target, const char *arg1, const char *arg2)
{
    emitStack(VALUE, arg1);
    emitStack(VALUE, arg2);
    emitStack(ADD, NULL);
    emitStack(STORE, target);
}
void generateIfStatement(const char *condition_var, const char *constant,
                         const char *write_var)
{
    char *endLabel = newStackLabel();

    emitStack(VALUE, condition_var);
    emitStack(PUSH, constant);
    emitStack(COMP_GT, NULL);
    emitStack(GO_FALSE, endLabel);
    emitStack(VALUE, write_var);
    emitStack(WRITE, NULL);
    emitStack(LABEL, endLabel);
    free(endLabel);
}
void cleanupStackCode()
{
    free(code.instructions);
    code.size = 0;
    code.capacity = 0;
    code.labelCount = 0;
}