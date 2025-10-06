#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKEN_LEN 100
#define MAX_VARS 26  // a-z variables

// Token types
typedef enum {
    TOKEN_INT, TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_ASSIGN,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_IF, TOKEN_EQUAL, TOKEN_LPAREN, TOKEN_RPAREN,
    TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_SEMICOLON, TOKEN_EOF, TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char text[MAX_TOKEN_LEN];
    int value;
} Token;

// AST Node types
typedef enum {
    NODE_PROGRAM, NODE_VAR_DECL, NODE_ASSIGN, NODE_BINARY_OP,
    NODE_NUMBER, NODE_IDENTIFIER, NODE_IF
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char varName[MAX_TOKEN_LEN];
    int value;
    char op;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *condition;
    struct ASTNode *ifBody;
    struct ASTNode *next;
} ASTNode;

// Global variables
FILE *inputFile;
Token currentToken;
char varTable[MAX_VARS][MAX_TOKEN_LEN];
int varCount = 0;

// Function prototypes
void getNextToken();
ASTNode* parseProgram();
ASTNode* parseStatement();
ASTNode* parseVarDecl();
ASTNode* parseAssignment();
ASTNode* parseIf();
ASTNode* parseExpression();
ASTNode* parseTerm();
void generateCode(ASTNode *node, FILE *output);
int getVarOffset(const char *varName);

// Lexer implementation
void getNextToken() {
    int c;
    
    // Skip whitespace and comments
    while ((c = fgetc(inputFile)) != EOF) {
        if (isspace(c)) continue;
        if (c == '/') {
            int next = fgetc(inputFile);
            if (next == '/') {
                while ((c = fgetc(inputFile)) != EOF && c != '\n');
                continue;
            }
            ungetc(next, inputFile);
        }
        break;
    }
    
    if (c == EOF) {
        currentToken.type = TOKEN_EOF;
        currentToken.text[0] = '\0';
        return;
    }
    
    // Parse identifiers and keywords
    if (isalpha(c)) {
        int len = 0;
        currentToken.text[len++] = c;
        while (isalnum(c = fgetc(inputFile)) && len < MAX_TOKEN_LEN - 1) {
            currentToken.text[len++] = c;
        }
        ungetc(c, inputFile);
        currentToken.text[len] = '\0';
        
        if (strcmp(currentToken.text, "int") == 0)
            currentToken.type = TOKEN_INT;
        else if (strcmp(currentToken.text, "if") == 0)
            currentToken.type = TOKEN_IF;
        else
            currentToken.type = TOKEN_IDENTIFIER;
        return;
    }
    
    // Parse numbers
    if (isdigit(c)) {
        int len = 0;
        currentToken.text[len++] = c;
        while (isdigit(c = fgetc(inputFile)) && len < MAX_TOKEN_LEN - 1) {
            currentToken.text[len++] = c;
        }
        ungetc(c, inputFile);
        currentToken.text[len] = '\0';
        currentToken.type = TOKEN_NUMBER;
        currentToken.value = atoi(currentToken.text);
        return;
    }
    
    // Parse operators and symbols
    currentToken.text[0] = c;
    currentToken.text[1] = '\0';
    
    switch (c) {
        case '=':
            c = fgetc(inputFile);
            if (c == '=') {
                currentToken.type = TOKEN_EQUAL;
                currentToken.text[1] = '=';
                currentToken.text[2] = '\0';
            } else {
                ungetc(c, inputFile);
                currentToken.type = TOKEN_ASSIGN;
            }
            break;
        case '+': currentToken.type = TOKEN_PLUS; break;
        case '-': currentToken.type = TOKEN_MINUS; break;
        case '(': currentToken.type = TOKEN_LPAREN; break;
        case ')': currentToken.type = TOKEN_RPAREN; break;
        case '{': currentToken.type = TOKEN_LBRACE; break;
        case '}': currentToken.type = TOKEN_RBRACE; break;
        case ';': currentToken.type = TOKEN_SEMICOLON; break;
        default: currentToken.type = TOKEN_UNKNOWN; break;
    }
}

// Parser implementation
ASTNode* createNode(NodeType type) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->left = node->right = node->condition = node->ifBody = node->next = NULL;
    node->varName[0] = '\0';
    node->value = 0;
    node->op = '\0';
    return node;
}

ASTNode* parseProgram() {
    ASTNode *root = createNode(NODE_PROGRAM);
    ASTNode *current = NULL;
    
    getNextToken();
    
    while (currentToken.type != TOKEN_EOF) {
        ASTNode *stmt = parseStatement();
        if (stmt) {
            if (current == NULL) {
                root->next = stmt;
                current = stmt;
            } else {
                current->next = stmt;
                current = stmt;
            }
        }
    }
    
    return root;
}

ASTNode* parseStatement() {
    if (currentToken.type == TOKEN_INT) {
        return parseVarDecl();
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        return parseAssignment();
    } else if (currentToken.type == TOKEN_IF) {
        return parseIf();
    }
    getNextToken();
    return NULL;
}

ASTNode* parseVarDecl() {
    getNextToken(); // consume 'int'
    
    if (currentToken.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Error: Expected identifier\n");
        exit(1);
    }
    
    ASTNode *node = createNode(NODE_VAR_DECL);
    strcpy(node->varName, currentToken.text);
    strcpy(varTable[varCount++], currentToken.text);
    
    getNextToken(); // consume identifier
    
    if (currentToken.type != TOKEN_SEMICOLON) {
        fprintf(stderr, "Error: Expected semicolon\n");
        exit(1);
    }
    
    getNextToken(); // consume semicolon
    return node;
}

ASTNode* parseAssignment() {
    ASTNode *node = createNode(NODE_ASSIGN);
    strcpy(node->varName, currentToken.text);
    
    getNextToken(); // consume identifier
    
    if (currentToken.type != TOKEN_ASSIGN) {
        fprintf(stderr, "Error: Expected '='\n");
        exit(1);
    }
    
    getNextToken(); // consume '='
    node->right = parseExpression();
    
    if (currentToken.type != TOKEN_SEMICOLON) {
        fprintf(stderr, "Error: Expected semicolon\n");
        exit(1);
    }
    
    getNextToken(); // consume semicolon
    return node;
}

ASTNode* parseExpression() {
    ASTNode *left = parseTerm();
    
    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        ASTNode *node = createNode(NODE_BINARY_OP);
        node->op = (currentToken.type == TOKEN_PLUS) ? '+' : '-';
        node->left = left;
        getNextToken();
        node->right = parseTerm();
        left = node;
    }
    
    return left;
}

ASTNode* parseTerm() {
    ASTNode *node;
    
    if (currentToken.type == TOKEN_NUMBER) {
        node = createNode(NODE_NUMBER);
        node->value = currentToken.value;
        getNextToken();
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        node = createNode(NODE_IDENTIFIER);
        strcpy(node->varName, currentToken.text);
        getNextToken();
    } else {
        fprintf(stderr, "Error: Expected number or identifier\n");
        exit(1);
    }
    
    return node;
}

ASTNode* parseIf() {
    getNextToken(); // consume 'if'
    
    if (currentToken.type != TOKEN_LPAREN) {
        fprintf(stderr, "Error: Expected '('\n");
        exit(1);
    }
    getNextToken();
    
    ASTNode *node = createNode(NODE_IF);
    node->condition = parseExpression();
    
    if (currentToken.type != TOKEN_EQUAL) {
        fprintf(stderr, "Error: Only '==' supported in conditions\n");
        exit(1);
    }
    getNextToken();
    
    ASTNode *rightCond = parseExpression();
    ASTNode *condNode = createNode(NODE_BINARY_OP);
    condNode->op = '=';
    condNode->left = node->condition;
    condNode->right = rightCond;
    node->condition = condNode;
    
    if (currentToken.type != TOKEN_RPAREN) {
        fprintf(stderr, "Error: Expected ')'\n");
        exit(1);
    }
    getNextToken();
    
    if (currentToken.type != TOKEN_LBRACE) {
        fprintf(stderr, "Error: Expected '{'\n");
        exit(1);
    }
    getNextToken();
    
    node->ifBody = parseStatement();
    
    if (currentToken.type != TOKEN_RBRACE) {
        fprintf(stderr, "Error: Expected '}'\n");
        exit(1);
    }
    getNextToken();
    
    return node;
}

// Code generator
int getVarOffset(const char *varName) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(varTable[i], varName) == 0) {
            return i;
        }
    }
    return -1;
}

void generateExpression(ASTNode *node, FILE *output) {
    if (node->type == NODE_NUMBER) {
        fprintf(output, "    LDI R0, %d       ; Load immediate %d\n", node->value, node->value);
    } else if (node->type == NODE_IDENTIFIER) {
        int offset = getVarOffset(node->varName);
        fprintf(output, "    LD R0, [%d]      ; Load variable %s\n", offset, node->varName);
    } else if (node->type == NODE_BINARY_OP) {
        generateExpression(node->left, output);
        fprintf(output, "    PUSH R0          ; Save left operand\n");
        generateExpression(node->right, output);
        fprintf(output, "    MOV R1, R0       ; Move right to R1\n");
        fprintf(output, "    POP R0           ; Restore left operand\n");
        
        if (node->op == '+') {
            fprintf(output, "    ADD R0, R1       ; Add R0 + R1\n");
        } else if (node->op == '-') {
            fprintf(output, "    SUB R0, R1       ; Subtract R0 - R1\n");
        }
    }
}

void generateCode(ASTNode *node, FILE *output) {
    if (node == NULL) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
            fprintf(output, "; SimpleLang compiled code for 8-bit CPU\n");
            fprintf(output, "; Variable memory starts at address 0\n\n");
            generateCode(node->next, output);
            fprintf(output, "\n    HLT              ; Halt execution\n");
            break;
            
        case NODE_VAR_DECL:
            fprintf(output, "; Declare variable: %s\n", node->varName);
            generateCode(node->next, output);
            break;
            
        case NODE_ASSIGN:
            fprintf(output, "\n; Assignment: %s = ...\n", node->varName);
            generateExpression(node->right, output);
            fprintf(output, "    ST R0, [%d]      ; Store to variable %s\n", 
                    getVarOffset(node->varName), node->varName);
            generateCode(node->next, output);
            break;
            
        case NODE_IF:
            fprintf(output, "\n; If statement\n");
            generateExpression(node->condition->left, output);
            fprintf(output, "    PUSH R0          ; Save left side\n");
            generateExpression(node->condition->right, output);
            fprintf(output, "    MOV R1, R0       ; Move right to R1\n");
            fprintf(output, "    POP R0           ; Restore left side\n");
            fprintf(output, "    CMP R0, R1       ; Compare\n");
            fprintf(output, "    JNE skip_%p      ; Jump if not equal\n", (void*)node);
            generateCode(node->ifBody, output);
            fprintf(output, "skip_%p:\n", (void*)node);
            generateCode(node->next, output);
            break;
            
        default:
            generateCode(node->next, output);
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.sl> <output.asm>\n", argv[0]);
        return 1;
    }
    
    inputFile = fopen(argv[1], "r");
    if (!inputFile) {
        perror("Failed to open input file");
        return 1;
    }
    
    FILE *outputFile = fopen(argv[2], "w");
    if (!outputFile) {
        perror("Failed to open output file");
        fclose(inputFile);
        return 1;
    }
    
    printf("Compiling %s...\n", argv[1]);
    
    ASTNode *ast = parseProgram();
    generateCode(ast, outputFile);
    
    printf("Assembly code generated in %s\n", argv[2]);
    
    fclose(inputFile);
    fclose(outputFile);
    
    return 0;
}
