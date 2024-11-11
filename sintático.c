#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_TOKEN_LENGTH 100
#define MAX_VARIAVEIS 100

typedef enum {
    TOKEN_PROGRAM, TOKEN_BEGIN, TOKEN_END, TOKEN_VAR, TOKEN_INTEGER, TOKEN_REAL,
    TOKEN_ASSIGN, TOKEN_SEMICOLON, TOKEN_COLON, TOKEN_PERIOD, TOKEN_IF, TOKEN_THEN,
    TOKEN_ID, TOKEN_NUMBER, TOKEN_EOF, TOKEN_ERROR, TOKEN_UNKNOWN,
    TOKEN_LESS_THAN, TOKEN_GREATER_THAN, TOKEN_EQUAL, TOKEN_NOT_EQUAL,
    TOKEN_LESS_EQUAL, TOKEN_GREATER_EQUAL
} TipoToken;

typedef struct {
    TipoToken tipo;
    char lexema[MAX_TOKEN_LENGTH];
    int linha;
    int coluna;
} Token;

Token tokenAtual;
int linhaAtual = 1;
int colunaAtual = 1;
FILE *arquivoFonte;
FILE *arquivoSaida;

typedef struct {
    char nome[MAX_TOKEN_LENGTH];
    TipoToken tipo;
} Variavel;

Variavel variaveis[MAX_VARIAVEIS];
int numVariaveis = 0;

// Declaração de funções
Token obterProximoToken();
void verificarToken(TipoToken esperado);
void programa();
void declaracoes();
void comando();
void comandoComposto();
void atribuir();
void ifComando();
void expressao();
void bloco();
void analisar();
void erro();
void imprimirRegraProducao(const char *regraProducao);
int buscarVariavel(const char *nome);

// Função de erro
void erro() {
    if (tokenAtual.tipo == TOKEN_EOF) {
        printf("Linha %d: fim de arquivo nao esperado.\n", tokenAtual.linha);
        if (arquivoSaida) {
            fprintf(arquivoSaida, "Linha %d: fim de arquivo nao esperado.\n", tokenAtual.linha);
        }
    } else {
        printf("Linha %d: token nao esperado [%s].\n", tokenAtual.linha, tokenAtual.lexema);
        if (arquivoSaida) {
            fprintf(arquivoSaida, "Linha %d: token nao esperado [%s].\n", tokenAtual.linha, tokenAtual.lexema);
        }
    }
    fclose(arquivoFonte);
    fclose(arquivoSaida);
    exit(1);
}

// Função para imprimir as regras de produção usadas até o momento
void imprimirRegraProducao(const char *regra) {
    fprintf(arquivoSaida, "%s\n", regra);
}

// Função para obter o nome do token
const char* obterNomeToken(TipoToken tipo) {
    switch (tipo) {
        case TOKEN_PROGRAM: return "programa";
        case TOKEN_BEGIN: return "inicio";
        case TOKEN_END: return "fim";
        case TOKEN_VAR: return "variavel";
        case TOKEN_INTEGER: return "inteiro";
        case TOKEN_REAL: return "real";
        case TOKEN_ASSIGN: return ":=";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_COLON: return ":";
        case TOKEN_PERIOD: return ".";
        case TOKEN_IF: return "se";
        case TOKEN_THEN: return "entao";
        case TOKEN_ID: return "identificador";
        case TOKEN_NUMBER: return "numero";
        case TOKEN_EOF: return "EOF";
        case TOKEN_LESS_THAN: return "<";
        case TOKEN_GREATER_THAN: return ">";
        case TOKEN_EQUAL: return "=";
        case TOKEN_NOT_EQUAL: return "<>";
        case TOKEN_LESS_EQUAL: return "<=";
        case TOKEN_GREATER_EQUAL: return ">=";
        default: return "erro";
    }
}

// Função para obter o próximo token do arquivo
Token obterProximoToken() {
    Token token;
    int c;

    // Implementação de leitura de tokens com atualização de linha e coluna
    while ((c = fgetc(arquivoFonte)) != EOF) {
        colunaAtual++;

        if (c == '\n') {
            linhaAtual++;
            colunaAtual = 1;
            continue;
        }

        if (isspace(c)) continue;

        if (isalpha(c)) {
            int i = 0;
            token.lexema[i++] = c;
            while (isalnum(c = fgetc(arquivoFonte))) {
                colunaAtual++;
                if (i < MAX_TOKEN_LENGTH - 1) token.lexema[i++] = c;
            }
            ungetc(c, arquivoFonte);
            token.lexema[i] = '\0';
            token.linha = linhaAtual;
            token.coluna = colunaAtual - i;

            if (strcmp(token.lexema, "program") == 0) token.tipo = TOKEN_PROGRAM;
            else if (strcmp(token.lexema, "begin") == 0) token.tipo = TOKEN_BEGIN;
            else if (strcmp(token.lexema, "end") == 0) token.tipo = TOKEN_END;
            else if (strcmp(token.lexema, "var") == 0) token.tipo = TOKEN_VAR;
            else if (strcmp(token.lexema, "integer") == 0) token.tipo = TOKEN_INTEGER;
            else if (strcmp(token.lexema, "real") == 0) token.tipo = TOKEN_REAL;
            else if (strcmp(token.lexema, "if") == 0) token.tipo = TOKEN_IF;
            else if (strcmp(token.lexema, "then") == 0) token.tipo = TOKEN_THEN;
            else token.tipo = TOKEN_ID;

            return token;
        }

        if (isdigit(c)) {
            int i = 0;
            token.lexema[i++] = c;
            while (isdigit(c = fgetc(arquivoFonte))) {
                colunaAtual++;
                if (i < MAX_TOKEN_LENGTH - 1) token.lexema[i++] = c;
            }
            ungetc(c, arquivoFonte);
            token.lexema[i] = '\0';
            token.linha = linhaAtual;
            token.coluna = colunaAtual - i;

            token.tipo = TOKEN_NUMBER;
            return token;
        }

        // Processamento de símbolos especiais e operadores
        if (c == ':') {
            if ((c = fgetc(arquivoFonte)) == '=') {
                token.tipo = TOKEN_ASSIGN;
                strcpy(token.lexema, ":=");
            } else {
                ungetc(c, arquivoFonte);
                token.tipo = TOKEN_COLON;
                strcpy(token.lexema, ":");
            }
            token.linha = linhaAtual;
            token.coluna = colunaAtual;
            return token;
        }

        if (c == ';') {
            token.tipo = TOKEN_SEMICOLON;
            strcpy(token.lexema, ";");
            token.linha = linhaAtual;
            token.coluna = colunaAtual;
            return token;
        }

        if (c == '.') {
            token.tipo = TOKEN_PERIOD;
            strcpy(token.lexema, ".");
            token.linha = linhaAtual;
            token.coluna = colunaAtual;
            return token;
        }

        if (c == '<') {
            c = fgetc(arquivoFonte);
            if (c == '>') {
                token.tipo = TOKEN_NOT_EQUAL;
                strcpy(token.lexema, "<>");
            } else if (c == '=') {
                token.tipo = TOKEN_LESS_EQUAL;
                strcpy(token.lexema, "<=");
            } else {
                ungetc(c, arquivoFonte);
                token.tipo = TOKEN_LESS_THAN;
                strcpy(token.lexema, "<");
            }
            token.linha = linhaAtual;
            token.coluna = colunaAtual;
            return token;
        }

        if (c == '>') {
            c = fgetc(arquivoFonte);
            if (c == '=') {
                token.tipo = TOKEN_GREATER_EQUAL;
                strcpy(token.lexema, ">=");
            } else {
                ungetc(c, arquivoFonte);
                token.tipo = TOKEN_GREATER_THAN;
                strcpy(token.lexema, ">");
            }
            token.linha = linhaAtual;
            token.coluna = colunaAtual;
            return token;
        }

        if (c == '=') {
            token.tipo = TOKEN_EQUAL;
            strcpy(token.lexema, "=");
            token.linha = linhaAtual;
            token.coluna = colunaAtual;
            return token;
        }

        token.tipo = TOKEN_ERROR;
        token.lexema[0] = c;
        token.lexema[1] = '\0';
        token.linha = linhaAtual;
        token.coluna = colunaAtual;
        return token;
    }

    token.tipo = TOKEN_EOF;
    strcpy(token.lexema, "EOF");
    token.linha = linhaAtual;
    token.coluna = colunaAtual;
    return token;
}

// Função para verificar se o token atual é o esperado
void verificarToken(TipoToken esperado) {
    if (tokenAtual.tipo == esperado) {
        tokenAtual = obterProximoToken();
    } else {
        erro();
    }
}

// Função para buscar uma variável na lista de variáveis
int buscarVariavel(const char *nome) {
    for (int i = 0; i < numVariaveis; i++) {
        if (strcmp(variaveis[i].nome, nome) == 0) {
            return i;
        }
    }
    return -1;
}

// Funções do analisador sintático (programa, bloco, comandos, etc.)

void programa() {
    imprimirRegraProducao("Regra de producao: programa -> 'program' <identificador> ';' <bloco> '.'");
    verificarToken(TOKEN_PROGRAM);
    verificarToken(TOKEN_ID);
    verificarToken(TOKEN_SEMICOLON);
    bloco();
    verificarToken(TOKEN_PERIOD);
}

// Função para processar declarações de variáveis
void declaracoes() {
    imprimirRegraProducao("Regra de producao: declaracoes -> 'var' <identificador> ':' <tipo> ';' { <identificador> ':' <tipo> ';' }");
    verificarToken(TOKEN_VAR);
    while (tokenAtual.tipo == TOKEN_ID) {
        char nomeVar[MAX_TOKEN_LENGTH];
        strcpy(nomeVar, tokenAtual.lexema);
        verificarToken(TOKEN_ID);
        verificarToken(TOKEN_COLON);

        if (tokenAtual.tipo == TOKEN_INTEGER || tokenAtual.tipo == TOKEN_REAL) {
            Variavel var;
            strcpy(var.nome, nomeVar);
            var.tipo = tokenAtual.tipo;
            variaveis[numVariaveis++] = var;
            verificarToken(tokenAtual.tipo);
        } else {
            erro();
        }

        verificarToken(TOKEN_SEMICOLON);
    }
}

// Função para processar comandos compostos
void comandoComposto() {
    imprimirRegraProducao("Regra de producao: comando -> 'begin' <comando> { <comando> } 'end'");
    verificarToken(TOKEN_BEGIN);
    comando();
    while (tokenAtual.tipo != TOKEN_END) {
        comando();
    }
    verificarToken(TOKEN_END);
}

// Função para processar um comando
void comando() {
    if (tokenAtual.tipo == TOKEN_BEGIN) {
        comandoComposto();
    } else if (tokenAtual.tipo == TOKEN_ID) {
        atribuir();
    } else {
        erro();
    }
}

// Função para processar uma atribuição
void atribuir() {
    verificarToken(TOKEN_ID);
    int idx = buscarVariavel(tokenAtual.lexema);
    if (idx == -1) erro();

    verificarToken(TOKEN_ASSIGN);
    expressao();
    verificarToken(TOKEN_SEMICOLON);
}

// Função para processar expressões (simplificação)
void expressao() {
    if (tokenAtual.tipo == TOKEN_NUMBER) {
        verificarToken(TOKEN_NUMBER);
    } else {
        erro();
    }
}

// Função para processar o bloco
void bloco() {
    imprimirRegraProducao("Regra de producao: bloco -> <declaracoes> <comandos>");
    declaracoes();
    comando();
}

int main() {
    arquivoFonte = fopen("teste.txt", "r");
    arquivoSaida = fopen("regras_producao.txt", "w");

    if (!arquivoFonte) {
        printf("Erro ao abrir o arquivo fonte.\n");
        return 1;
    }

    tokenAtual = obterProximoToken();
    programa();

    fclose(arquivoFonte);
    fclose(arquivoSaida);

    return 0;
}
