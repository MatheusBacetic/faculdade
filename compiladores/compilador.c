#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 1. Definição do Enum conforme a especificação do projeto
typedef enum {
    ERRO,
    IDENTIFICADOR,
    NUMERO_INTEIRO,
    BOOLEANO,
    OPERADOR_ARITMETICO,
    OPERADOR_RELACIONAL,
    OPERADOR_LOGICO,
    DELIMITADOR,
    PONTUACAO,
    PALAVRA_RESERVADA,
    STRING_LITERAL,
    EOS // End Of Stream (Fim de Arquivo)
} TAtomo;

// 2. Definição da Estrutura (Struct) do Token
typedef struct {
    TAtomo tipo;       // O tipo enumerado definido acima
    char lexema[100];  // A string original do código
    int linha;         // Número da linha onde o token aparece
} Token;

// Variáveis Globais
FILE *fonte;
int linhaAtual = 1;

// 3. Protótipos das Funções Principais
void iniciarAnalisador(char *nomeArquivo);
Token proximoToken();
void imprimirToken(Token token);
void fecharAnalisador();

// Protótipos das Funções Auxiliares (Para classificar os caracteres)
char* nomeDoTipo(TAtomo tipo);
void erroLexico(char *lexema);

// --- FUNÇÃO PRINCIPAL ---
int main(int argc, char *argv[]) {
    // O projeto exige que o nome do arquivo seja passado por linha de comando
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte.py>\n", argv[0]);
        return 1;
    }

    iniciarAnalisador(argv[1]);

    Token token;
    
    // Loop principal: pede tokens até encontrar o fim do arquivo (EOS) ou um ERRO
    do {
        token = proximoToken();
        
        if (token.tipo == ERRO) {
            erroLexico(token.lexema);
            break; // O processo deve ser finalizado em caso de erro
        } else if (token.tipo != EOS) {
            imprimirToken(token);
        }
        
    } while (token.tipo != EOS);

    fecharAnalisador();
    return 0;
}

// --- IMPLEMENTAÇÃO DAS FUNÇÕES BÁSICAS ---

void iniciarAnalisador(char *nomeArquivo) {
    fonte = fopen(nomeArquivo, "r");
    if (fonte == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", nomeArquivo);
        exit(1);
    }
}

void fecharAnalisador() {
    if (fonte != NULL) {
        fclose(fonte);
    }
}

void erroLexico(char *lexema) {
    // Formatação de erro exigida pelo projeto
    printf("ERRO LÉXICO\n");
    printf("Linha: %d\n", linhaAtual);
    printf("Sequência inválida: %s\n", lexema);
}

void imprimirToken(Token token) {
    // Formato de saída: Número da Linha do Átomo # NomeToken | Atributo
    printf("%d# %s | %s\n", token.linha, nomeDoTipo(token.tipo), token.lexema);
}

// Função utilitária para converter o enum em texto para o print
char* nomeDoTipo(TAtomo tipo) {
    switch(tipo) {
        case IDENTIFICADOR: return "IDENTIFICADOR";
        case NUMERO_INTEIRO: return "NUMERO_INTEIRO";
        case BOOLEANO: return "BOOLEANO";
        case OPERADOR_ARITMETICO: return "OPERADOR_ARITMETICO";
        case OPERADOR_RELACIONAL: return "OPERADOR_RELACIONAL";
        case OPERADOR_LOGICO: return "OPERADOR_LOGICO";
        case DELIMITADOR: return "DELIMITADOR";
        case PONTUACAO: return "PONTUACAO";
        case PALAVRA_RESERVADA: return "PALAVRA_RESERVADA";
        case STRING_LITERAL: return "STRING_LITERAL";
        default: return "DESCONHECIDO";
    }
}

// A função mais importante (O coração do Analisador Léxico)
Token proximoToken() {
    Token token;
    token.tipo = EOS;
    token.lexema[0] = '\0';
    token.linha = linhaAtual;
    
    // TODO: Implementar a lógica de leitura dos caracteres e formar os tokens
    // Essa função será a tradução exata do autômato que desenhamos no JFLAP.
    
    return token;
}
