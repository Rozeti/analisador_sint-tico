#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_ID_LENGTH 64
#define MAX_VAR 100

// Definição de tokens
typedef enum {
    TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_KEYWORD, TOKEN_OPERATOR,
    TOKEN_DELIMITER, TOKEN_END_OF_FILE, TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[MAX_ID_LENGTH];
    int line, column;
} Token;

// Variáveis globais para análise léxica
FILE *file;
int line = 1, column = 1;
Token current_token;

// Variáveis para controle de escopo e tipos
typedef struct {
    char name[MAX_ID_LENGTH];
    char type[MAX_ID_LENGTH];
} Variable;

Variable variables[MAX_VAR];
int var_count = 0;

// Funções para análise léxica
Token get_next_token();
void add_variable(char *name, char *type);
int is_variable_declared(char *name);
int is_valid_identifier(char *str);
void error_message(const char *msg, Token token);
void CasaToken(TokenType expected);
TokenType check_keyword(char *str);
TokenType get_next_token();
void parse_program();
void parse_block();
void parse_statement();
void parse_expression();
void parse_term();
void parse_factor();
void check_balanced_parentheses(char *expr);
void parse_type_checking(char *type_expected, char *expr_type);

// Função para adicionar variável ao escopo
void add_variable(char *name, char *type) {
    if (var_count < MAX_VAR) {
        strcpy(variables[var_count].name, name);
        strcpy(variables[var_count].type, type);
        var_count++;
    }
}

int is_variable_declared(char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) return 1;
    }
    return 0;
}

// Função para verificar identificador
int is_valid_identifier(char *str) {
    if (!isalpha(str[0])) return 0;
    for (int i = 1; i < strlen(str); i++) {
        if (!isalnum(str[i]) && str[i] != '_') return 0;
    }
    return 1;
}

// Função de erro
void error_message(const char *msg, Token token) {
    printf("%d:%d: %s [%s]\n", token.line, token.column, msg, token.lexeme);
    exit(1);
}

// Função para comparação de tokens
void CasaToken(TokenType expected) {
    if (current_token.type == expected) {
        current_token = get_next_token();
    } else {
        error_message("token nao esperado", current_token);
    }
}

// Analisador Léxico com verificação de identificadores e números
Token get_next_token() {
    Token token = {TOKEN_UNKNOWN, "", line, column};
    int ch = fgetc(file);

    // Atualizar a posição
    while (isspace(ch)) {
        if (ch == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        ch = fgetc(file);
    }

    if (isalpha(ch)) {
        int i = 0;
        do {
            token.lexeme[i++] = ch;
            ch = fgetc(file);
            column++;
        } while (isalnum(ch) || ch == '_');
        token.lexeme[i] = '\0';
        ungetc(ch, file);

        // Palavra reservada ou identificador
        token.type = check_keyword(token.lexeme);
        if (token.type == TOKEN_UNKNOWN) token.type = TOKEN_IDENTIFIER;
        
        // Validação de identificador
        if (!is_valid_identifier(token.lexeme)) error_message("identificador invalido", token);
    }
    else if (isdigit(ch)) {
        int i = 0;
        do {
            token.lexeme[i++] = ch;
            ch = fgetc(file);
            column++;
        } while (isdigit(ch));
        token.lexeme[i] = '\0';
        token.type = TOKEN_NUMBER;
        ungetc(ch, file);
    } 
    else if (ch == EOF) {
        token.type = TOKEN_END_OF_FILE;
    }
    else {
        token.lexeme[0] = ch;
        token.lexeme[1] = '\0';
        column++;
    }

    return token;
}

// Funções do Analisador Sintático
void parse_program() {
    CasaToken(TOKEN_KEYWORD);  // Assumindo que o programa começa com 'program'
    CasaToken(TOKEN_IDENTIFIER);
    CasaToken(TOKEN_DELIMITER);
    parse_block();
    CasaToken(TOKEN_DELIMITER);  // Assumindo ';' para o fim do programa
}

void parse_block() {
    CasaToken(TOKEN_KEYWORD); // 'begin'
    while (current_token.type == TOKEN_IDENTIFIER || current_token.type == TOKEN_KEYWORD) {
        parse_statement();
    }
    CasaToken(TOKEN_KEYWORD); // 'end'
}

void parse_statement() {
    if (current_token.type == TOKEN_IDENTIFIER) {
        // Exemplo de declaração de variável
        char var_name[MAX_ID_LENGTH];
        strcpy(var_name, current_token.lexeme);
        if (!is_variable_declared(var_name)) {
            add_variable(var_name, "int"); // Aqui definimos o tipo de exemplo
        } else {
            error_message("variável já declarada", current_token);
        }
        CasaToken(TOKEN_IDENTIFIER);
        CasaToken(TOKEN_OPERATOR); // Exemplo '='
        parse_expression();
        CasaToken(TOKEN_DELIMITER); // ';'
    } else if (current_token.type == TOKEN_KEYWORD) {
        if (strcmp(current_token.lexeme, "while") == 0) {
            CasaToken(TOKEN_KEYWORD); // 'while'
            parse_expression();
            CasaToken(TOKEN_KEYWORD); // 'do'
            parse_statement();
        } else {
            error_message("palavra-chave inesperada", current_token);
        }
    }
}

void parse_expression() {
    parse_term();
    while (current_token.type == TOKEN_OPERATOR) {
        CasaToken(TOKEN_OPERATOR);
        parse_term();
    }
}

void parse_term() {
    parse_factor();
    while (current_token.type == TOKEN_OPERATOR) {
        CasaToken(TOKEN_OPERATOR);
        parse_factor();
    }
}

void parse_factor() {
    if (current_token.type == TOKEN_IDENTIFIER) {
        if (!is_variable_declared(current_token.lexeme)) {
            error_message("variável não declarada", current_token);
        }
        CasaToken(TOKEN_IDENTIFIER);
    } else if (current_token.type == TOKEN_NUMBER) {
        CasaToken(TOKEN_NUMBER);
    } else {
        error_message("fator inválido na expressão", current_token);
    }
}

int main() {
    file = fopen("teste.txt", "r");
    if (!file) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    current_token = get_next_token();
    parse_program();

    if (current_token.type == TOKEN_END_OF_FILE) {
        printf("Compilação concluída com sucesso.\n");
    } else {
        error_message("fim de arquivo não esperado", current_token);
    }

    fclose(file);
    return 0;
}
