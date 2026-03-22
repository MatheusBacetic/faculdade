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

// 2. Definição da Estrutura (Struct) do Token (TInfoAtomo)
typedef struct {
    TAtomo tipo;       
    char lexema[100];  
    int linha;         
} TInfoAtomo;

// Variáveis Globais
FILE *fonte;
FILE *saida; // Arquivo para gravar os tokens gerados
int linhaAtual = 1;

// 3. Protótipos das Funções
void iniciarAnalisador(char *nomeArquivo);
TInfoAtomo obter_atomo(); // Nome exigido pelo item OBJETIVO
void imprimirToken(TInfoAtomo token);
void fecharAnalisador();
char* nomeDoTipo(TAtomo tipo);
void erroLexico(char *lexema);
TAtomo classificarLexema(char *lexema);

// --- FUNÇÃO PRINCIPAL ---
int main(int argc, char *argv[]) {
    // O projeto exige que o nome do arquivo seja passado por linha de comando
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte.py>\n", argv[0]);
        return 1;
    }

    iniciarAnalisador(argv[1]);

    TInfoAtomo atomo;
    
    // Loop principal: pede tokens até encontrar o fim do arquivo (EOS) ou um ERRO
    do {
        atomo = obter_atomo();
        
        if (atomo.tipo == ERRO) {
            erroLexico(atomo.lexema);
            break; // O processo deve ser finalizado em caso de erro léxico
        } else if (atomo.tipo != EOS) {
            imprimirToken(atomo);
        }
        
    } while (atomo.tipo != EOS);

    fecharAnalisador();
    return 0;
}

// --- IMPLEMENTAÇÃO DAS FUNÇÕES BÁSICAS ---

void iniciarAnalisador(char *nomeArquivo) {
    fonte = fopen(nomeArquivo, "r");
    if (fonte == NULL) {
        printf("Erro ao abrir o arquivo fonte: %s\n", nomeArquivo);
        exit(1);
    }
    
    // Abre o arquivo de saída para gravar os tokens (cria ou sobrescreve)
    saida = fopen("saida_lexica.txt", "w");
    if (saida == NULL) {
        printf("Erro ao criar o arquivo de saida_lexica.txt\n");
        fclose(fonte);
        exit(1);
    }
}

void fecharAnalisador() {
    if (fonte != NULL) fclose(fonte);
    if (saida != NULL) fclose(saida);
}

void erroLexico(char *lexema) {
    // Formatação de erro exigida pelo projeto e print na tela
    printf("ERRO LÉXICO\n");
    printf("Linha: %d\n", linhaAtual);
    printf("Sequência inválida: %s\n", lexema);
    
    // Grava o erro também no arquivo de saída
    fprintf(saida, "ERRO LÉXICO na linha %d: %s\n", linhaAtual, lexema);
}

void imprimirToken(TInfoAtomo atomo) {
    // Formato de saída: Número da Linha do Átomo # NomeToken | Atributo
    char* nomeTipo = nomeDoTipo(atomo.tipo);
    
    // 1. Apresenta na tela
    printf("%d# %s | %s\n", atomo.linha, nomeTipo, atomo.lexema);
    
    // 2. Produz arquivo de saída
    fprintf(saida, "%d# %s | %s\n", atomo.linha, nomeTipo, atomo.lexema);
}

// Função utilitária para converter o enum em texto
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

// --- O CORAÇÃO DO ANALISADOR LÉXICO (Módulo exigido: obter_atomo) ---
TInfoAtomo obter_atomo() {
    TInfoAtomo atomo;
    atomo.tipo = ERRO; 
    atomo.lexema[0] = '\0';
    atomo.linha = linhaAtual;
    
    char c;
    int i = 0;

    // 1. Ignorar espaços, tabulações, quebras de linha e comentários
    while ((c = fgetc(fonte)) != EOF) {
        if (c == '\n') {
            linhaAtual++; 
        } else if (isspace(c)) {
            continue; 
        } else if (c == '#') {
            while ((c = fgetc(fonte)) != '\n' && c != EOF);
            if (c == '\n') linhaAtual++; 
        } else {
            break; 
        }
    }

    atomo.linha = linhaAtual;

    // 2. Fim de Arquivo (EOF)
    if (c == EOF) {
        atomo.tipo = EOS;
        strcpy(atomo.lexema, "EOF");
        return atomo;
    }

    // 3. Identificação de Números Inteiros
    if (isdigit(c)) {
        atomo.lexema[i++] = c;
        while (isdigit(c = fgetc(fonte))) {
            atomo.lexema[i++] = c;
        }
        ungetc(c, fonte); 
        atomo.lexema[i] = '\0'; 
        atomo.tipo = NUMERO_INTEIRO;
        return atomo;
    }

    // 4. Identificação de Identificadores e Palavras Reservadas
    if (isalpha(c) || c == '_') {
        atomo.lexema[i++] = c;
        while (isalnum(c = fgetc(fonte)) || c == '_') {
            atomo.lexema[i++] = c;
        }
        ungetc(c, fonte); 
        atomo.lexema[i] = '\0'; 
        
        atomo.tipo = classificarLexema(atomo.lexema);
        return atomo;
    }

    // 5. Strings Literais
    if (c == '"' || c == '\'') {
        char delimitador_string = c; 
        atomo.lexema[i++] = c;
        
        while ((c = fgetc(fonte)) != delimitador_string && c != EOF && c != '\n') {
            atomo.lexema[i++] = c;
        }
        
        if (c == delimitador_string) {
            atomo.lexema[i++] = c; 
            atomo.lexema[i] = '\0';
            atomo.tipo = STRING_LITERAL;
        } else {
            atomo.lexema[i] = '\0';
            atomo.tipo = ERRO;
        }
        return atomo;
    }

    // 6. Operadores Relacionais e Atribuição
    if (c == '=' || c == '<' || c == '>' || c == '!') {
        atomo.lexema[i++] = c;
        char prox = fgetc(fonte); 
        
        if (prox == '=') {
            atomo.lexema[i++] = prox;
            atomo.lexema[i] = '\0';
            atomo.tipo = OPERADOR_RELACIONAL; 
            return atomo;
        }
        
        ungetc(prox, fonte);
        atomo.lexema[i] = '\0';
        
        if (c == '!') {
            atomo.tipo = ERRO; 
        } else if (c == '=') {
            atomo.tipo = DELIMITADOR; 
        } else {
            atomo.tipo = OPERADOR_RELACIONAL; 
        }
        return atomo;
    }

    // 7. Operadores Aritméticos
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%') {
        atomo.lexema[i++] = c;
        
        if (c == '*') {
            char prox = fgetc(fonte);
            if (prox == '*') {
                atomo.lexema[i++] = prox; 
            } else {
                ungetc(prox, fonte); 
            }
        }
        
        atomo.lexema[i] = '\0';
        atomo.tipo = OPERADOR_ARITMETICO;
        return atomo;
    }

    // 8. Delimitadores e Pontuação
    if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',') {
        atomo.lexema[i++] = c;
        atomo.lexema[i] = '\0';
        atomo.tipo = DELIMITADOR;
        return atomo;
    }
    
    if (c == ':') {
        atomo.lexema[i++] = c;
        atomo.lexema[i] = '\0';
        atomo.tipo = PONTUACAO;
        return atomo;
    }

    // 9. Se chegou até aqui, é um caractere inválido
    atomo.lexema[0] = c;
    atomo.lexema[1] = '\0';
    atomo.tipo = ERRO;
    
    return atomo;
}

// Função para diferenciar Identificadores de Palavras Reservadas e Booleanos
TAtomo classificarLexema(char *lexema) {
    const char *reservadas[] = {
        "return", "from", "while", "as", "elif", "with", "else", "if", 
        "break", "len", "input", "print", "exec", "raise", "continue", 
        "range", "def", "for"
    };
    int numReservadas = sizeof(reservadas) / sizeof(reservadas[0]);
    
    for (int i = 0; i < numReservadas; i++) {
        if (strcmp(lexema, reservadas[i]) == 0) return PALAVRA_RESERVADA;
    }

    if (strcmp(lexema, "True") == 0 || strcmp(lexema, "False") == 0) return BOOLEANO;

    if (strcmp(lexema, "and") == 0 || strcmp(lexema, "or") == 0 || 
        strcmp(lexema, "not") == 0 || strcmp(lexema, "in") == 0 || 
        strcmp(lexema, "is") == 0) return OPERADOR_LOGICO;

    return IDENTIFICADOR;
}