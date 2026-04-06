/* ===================================================================
 * PROJETO COMPILADORES - FASE 1 (Análise Léxica e Sintática)
 * Alunos: Matheus Veiga Bacetic RA: 10425638 | Beatriz Barbosa RA: 10354067 | Gabriel Pereira Faravola RA: 10427189
 * Compilação: gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador
 * =================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Definição dos tipos de átomos (tokens) reconhecidos pelo analisador léxico
typedef enum {
    ERRO,                    // Token de erro
    IDENTIFICADOR,           // Nomes de variáveis ou funções
    NUMERO_INTEIRO,          // Números inteiros
    BOOLEANO,                // Valores booleanos True/False
    OPERADOR_ARITMETICO,     // Operadores +, -, *, /, %
    OPERADOR_RELACIONAL,     // Operadores ==, !=, <, >, <=, >=
    OPERADOR_LOGICO,         // Operadores and, or, not, in, is
    DELIMITADOR,             // Delimitadores (, ), [, ], {, }, ,, =
    PONTUACAO,               // Pontuação :
    PALAVRA_RESERVADA,       // Palavras reservadas como if, while, etc.
    STRING_LITERAL,          // Strings entre aspas
    EOS                      // Fim do arquivo
} TAtomo;

// Estrutura para representar um token, contendo tipo, lexema e linha
typedef struct {
    TAtomo tipo;       // Tipo do token
    char lexema[100];  // Texto do token
    int linha;         // Linha onde o token foi encontrado
} TInfoAtomo;

// Variáveis globais para gerenciar arquivos e estado do analisador
FILE *fonte;           // Arquivo fonte a ser analisado
FILE *saida;           // Arquivo de saída para tokens
int linhaAtual = 1;    // Contador de linhas atual
TInfoAtomo lookahead;  // Token atual sendo analisado

// Protótipos das funções léxicas
void iniciarAnalisador(char *nomeArquivo);
TInfoAtomo motor_lexico(); // Função principal do analisador léxico
TInfoAtomo obter_atomo(); // Wrapper que imprime e retorna o token
void imprimirToken(TInfoAtomo token);
void fecharAnalisador();
char* nomeDoTipo(TAtomo tipo);
void erroLexico(char *lexema);
TAtomo classificarLexema(char *lexema);

// Protótipos das funções sintáticas
void consome(TAtomo tipo_esperado);
void erroSintatico(char *mensagem);
void programa();
void comando();
void atribuicao();
void comandoPrint();
void comandoInput();
void condicional();
void repeticao();
void listaExpressoes();
void expressao();
void expRelacional(); // Adicione isso junto aos outros protótipos
void expSimples();
void termo();
void fator();
void estruturaLista();

// Função principal: inicializa o analisador, processa o arquivo e finaliza
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte.py>\n", argv[0]);
        return 1;
    }

    iniciarAnalisador(argv[1]);

    // Obtém o primeiro token
    lookahead = obter_atomo();
    
    // Inicia a análise sintática a partir da regra programa
    programa();
    
    // Verifica se a análise terminou corretamente
    if (lookahead.tipo == EOS) {
        printf("Análise Sintática concluída com sucesso! Nenhum erro encontrado.\n");
        fprintf(saida, "Análise Sintática concluída com sucesso!\n");
    } else {
        erroSintatico("Código extra encontrado após o fim do programa principal.");
    }

    fecharAnalisador();
    return 0;
}

// Inicializa o analisador abrindo os arquivos necessários
void iniciarAnalisador(char *nomeArquivo) {
    fonte = fopen(nomeArquivo, "r");
    if (fonte == NULL) {
        printf("Erro ao abrir o arquivo fonte: %s\n", nomeArquivo);
        exit(1);
    }
    
    saida = fopen("saida_lexica.txt", "w");
    if (saida == NULL) {
        printf("Erro ao criar o arquivo de saida_lexica.txt\n");
        fclose(fonte);
        exit(1);
    }
}

// Fecha os arquivos abertos
void fecharAnalisador() {
    if (fonte != NULL) fclose(fonte);
    if (saida != NULL) fclose(saida);
}

// Trata erros léxicos, imprimindo mensagem e encerrando o programa
void erroLexico(char *lexema) {
    printf("ERRO LÉXICO\n");
    printf("Linha: %d\n", linhaAtual);
    printf("Sequência inválida: %s\n", lexema);
    
    fprintf(saida, "ERRO LÉXICO na linha %d: %s\n", linhaAtual, lexema);
    
    fecharAnalisador();
    exit(1);
}

// Imprime um token no formato especificado
void imprimirToken(TInfoAtomo atomo) {
    char* nomeTipo = nomeDoTipo(atomo.tipo);
    
    printf("%d# %s | %s\n", atomo.linha, nomeTipo, atomo.lexema);
    fprintf(saida, "%d# %s | %s\n", atomo.linha, nomeTipo, atomo.lexema);
}

// Converte o enum do tipo para string legível
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

// Função principal do analisador léxico: lê caracteres e identifica tokens
TInfoAtomo motor_lexico() {
    TInfoAtomo atomo;
    atomo.tipo = ERRO; 
    atomo.lexema[0] = '\0';
    atomo.linha = linhaAtual;
    
    char c;
    int i = 0;

    // Ignora espaços, quebras de linha e comentários
    while ((c = fgetc(fonte)) != EOF) {
        if (c == '\n') {
            linhaAtual++; 
        } else if (isspace(c)) {
            continue; 
        } else if (c == '#') {
            // Ignora comentários até o fim da linha
            while ((c = fgetc(fonte)) != '\n' && c != EOF);
            if (c == '\n') linhaAtual++; 
        } else {
            break; 
        }
    }

    atomo.linha = linhaAtual;

    // Fim do arquivo
    if (c == EOF) {
        atomo.tipo = EOS;
        strcpy(atomo.lexema, "EOF");
        return atomo;
    }

    // Reconhece números inteiros
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

    // Reconhece identificadores e palavras reservadas
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

    // Reconhece strings literais
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

    // Reconhece operadores relacionais e atribuição
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

    // Reconhece operadores aritméticos
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

    // Reconhece delimitadores
    if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',') {
        atomo.lexema[i++] = c;
        atomo.lexema[i] = '\0';
        atomo.tipo = DELIMITADOR;
        return atomo;
    }
    
    // Reconhece pontuação
    if (c == ':') {
        atomo.lexema[i++] = c;
        atomo.lexema[i] = '\0';
        atomo.tipo = PONTUACAO;
        return atomo;
    }

    // Caractere inválido
    atomo.lexema[0] = c;
    atomo.lexema[1] = '\0';
    atomo.tipo = ERRO;
    
    erroLexico(atomo.lexema);
    return atomo;
}

// Wrapper para obter token, imprimir se válido e retornar
TInfoAtomo obter_atomo() {
    TInfoAtomo atomo = motor_lexico();
    
    if (atomo.tipo != EOS && atomo.tipo != ERRO) {
        imprimirToken(atomo);
    }
    
    return atomo;
}

// Classifica lexemas como identificadores, palavras reservadas ou booleanos
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

// Seção do Analisador Sintático

// Trata erros sintáticos
void erroSintatico(char *mensagem) {
    printf("ERRO SINTÁTICO\n");
    printf("Linha: %d\n", lookahead.linha);
    printf("Token incorreto: %s\n", lookahead.lexema);
    printf("Detalhe: %s\n", mensagem);
    
    fprintf(saida, "ERRO SINTÁTICO na linha %d: token incorreto '%s'. %s\n", 
            lookahead.linha, lookahead.lexema, mensagem);
            
    fecharAnalisador();
    exit(1);
}

// Consome um token esperado, avançando para o próximo
void consome(TAtomo tipo_esperado) {
    if (lookahead.tipo == tipo_esperado) {
        lookahead = obter_atomo();
    } else {
        char mensagemErro[100];
        sprintf(mensagemErro, "Esperava token do tipo %s, mas encontrou %s", 
                nomeDoTipo(tipo_esperado), nomeDoTipo(lookahead.tipo));
        erroSintatico(mensagemErro);
    }
}

// Regra gramatical: Programa -> Comando*
void programa() {
    while (lookahead.tipo != EOS) {
        comando();
    }
}

// Regra: Comando -> Atribuição | Condicional | Repetição | Print | Input | Return
void comando() {
    if (lookahead.tipo == IDENTIFICADOR) {
        atribuicao();
    } else if (lookahead.tipo == PALAVRA_RESERVADA) {
        if (strcmp(lookahead.lexema, "print") == 0) {
            comandoPrint();
        } else if (strcmp(lookahead.lexema, "if") == 0) {
            condicional();
        } else if (strcmp(lookahead.lexema, "while") == 0 || strcmp(lookahead.lexema, "for") == 0) {
            repeticao();
        } else if (strcmp(lookahead.lexema, "return") == 0) {
            consome(PALAVRA_RESERVADA);
            expressao();
        } else {
            erroSintatico("Palavra reservada não esperada no início de um comando.");
        }
    } else {
        erroSintatico("Início de comando inválido.");
    }
}

// Regra para atribuições, incluindo índices e chamadas de função
void atribuicao() {
    consome(IDENTIFICADOR);

    // Verifica se é chamada de função solta
    if (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "(") == 0) {
        consome(DELIMITADOR);
        if (strcmp(lookahead.lexema, ")") != 0) {
            listaExpressoes();
        }
        if (strcmp(lookahead.lexema, ")") == 0) {
            consome(DELIMITADOR);
        } else {
            erroSintatico("Esperava ')' no fim da chamada de função.");
        }
        return;
    }

    // Verifica se é atribuição em índice de lista
    if (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "[") == 0) {
        consome(DELIMITADOR);
        expressao();
        if (strcmp(lookahead.lexema, "]") == 0) {
            consome(DELIMITADOR);
        } else {
            erroSintatico("Esperava ']' no acesso à lista.");
        }
    }

    // Consome o operador de atribuição
    if (strcmp(lookahead.lexema, "=") == 0) {
        consome(DELIMITADOR);
        if (lookahead.tipo == PALAVRA_RESERVADA && strcmp(lookahead.lexema, "input") == 0) {
            comandoInput();
        } else {
            expressao();
        }
    } else {
        erroSintatico("Esperava '=' para atribuição.");
    }
}

// Regra para comando print
void comandoPrint() {
    consome(PALAVRA_RESERVADA);
    listaExpressoes();
}

// Regra: ExpRelacional -> ExpSimples ( OperadorRelacional ExpSimples )?
void expRelacional() {
    expSimples();
    if (lookahead.tipo == OPERADOR_RELACIONAL) {
        consome(OPERADOR_RELACIONAL);
        expSimples();
    }
}

// Nova regra: Expressao -> ExpRelacional ( ("and" | "or") ExpRelacional )*
void expressao() {
    expRelacional(); // Chama a função que acabamos de renomear
    
    // Enquanto encontrar 'and' ou 'or', continua lendo condições
    while (lookahead.tipo == OPERADOR_LOGICO && 
          (strcmp(lookahead.lexema, "and") == 0 || strcmp(lookahead.lexema, "or") == 0)) {
        consome(OPERADOR_LOGICO);
        expRelacional();
    }
}

// Regra: ExpSimples -> Termo (OperadorAritmético Termo)*
void expSimples() {
    if (lookahead.tipo == OPERADOR_ARITMETICO && 
       (strcmp(lookahead.lexema, "+") == 0 || strcmp(lookahead.lexema, "-") == 0)) {
        consome(OPERADOR_ARITMETICO);
    }
    
    termo();
    while (lookahead.tipo == OPERADOR_ARITMETICO) {
        consome(OPERADOR_ARITMETICO);
        termo();
    }
}

// Regra: Termo -> (not)? Fator
void termo() {
    if (lookahead.tipo == OPERADOR_LOGICO && strcmp(lookahead.lexema, "not") == 0) {
        consome(OPERADOR_LOGICO);
    }
    fator();
}

// Regra para fatores, incluindo identificadores, números, booleanos, listas, etc.
void fator() {
    if (lookahead.tipo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);
        
        // Verifica acesso a índice
        if (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "[") == 0) {
            consome(DELIMITADOR);
            expressao();
            if (strcmp(lookahead.lexema, "]") == 0) {
                consome(DELIMITADOR);
            } else {
                erroSintatico("Esperava ']' após índice do vetor.");
            }
        }
    } 
    else if (lookahead.tipo == PALAVRA_RESERVADA && strcmp(lookahead.lexema, "len") == 0) {
        consome(PALAVRA_RESERVADA);
        consome(DELIMITADOR);
        expressao();
        if (strcmp(lookahead.lexema, ")") == 0) {
            consome(DELIMITADOR);
        } else {
            erroSintatico("Esperava ')' após o argumento do len.");
        }
    }
    else if (lookahead.tipo == NUMERO_INTEIRO) {
        consome(NUMERO_INTEIRO);
    } else if (lookahead.tipo == BOOLEANO) {
        consome(BOOLEANO);
    } else if (lookahead.tipo == STRING_LITERAL) {
        consome(STRING_LITERAL);
    } else if (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "[") == 0) {
        estruturaLista();
    } else if (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "(") == 0) {
        consome(DELIMITADOR); 
        expressao();
        if (strcmp(lookahead.lexema, ")") == 0) {
            consome(DELIMITADOR); 
        } else {
            erroSintatico("Esperava ')' para fechar expressão.");
        }
    } else {
        erroSintatico("Fator inválido na expressão.");
    }
}

// Regra para lista de expressões
void listaExpressoes() {
    expressao();
    while (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ",") == 0) {
        consome(DELIMITADOR);
        expressao();
    }
}

// Regra para condicional if-else
void condicional() {
    consome(PALAVRA_RESERVADA);
    expressao();
    if (strcmp(lookahead.lexema, ":") == 0) {
        consome(PONTUACAO);
    } else {
        erroSintatico("Esperava ':' após a expressão do if.");
    }
    comando();

    if (lookahead.tipo == PALAVRA_RESERVADA && strcmp(lookahead.lexema, "else") == 0) {
        consome(PALAVRA_RESERVADA);
        if (strcmp(lookahead.lexema, ":") == 0) {
            consome(PONTUACAO);
        } else {
            erroSintatico("Esperava ':' após o else.");
        }
        comando();
    }
}

// Regra para repetições while e for
void repeticao() {
    if (strcmp(lookahead.lexema, "while") == 0) {
        consome(PALAVRA_RESERVADA);
        expressao();
        consome(PONTUACAO);
        comando();
    } else {
        consome(PALAVRA_RESERVADA);
        consome(IDENTIFICADOR);
        
        if (lookahead.tipo == OPERADOR_LOGICO && strcmp(lookahead.lexema, "in") == 0) {
            consome(OPERADOR_LOGICO); 
        } else {
            erroSintatico("Esperava 'in' no comando for.");
        }
        
        if (strcmp(lookahead.lexema, "range") == 0) {
            consome(PALAVRA_RESERVADA);
        } else {
            erroSintatico("Esperava 'range' no comando for.");
        }
        
        consome(DELIMITADOR);
        expressao();
        consome(DELIMITADOR);
        consome(PONTUACAO);
        comando();
    }
}

// Regra para comando input
void comandoInput() {
    consome(PALAVRA_RESERVADA);
    consome(DELIMITADOR);
    if (lookahead.tipo == STRING_LITERAL) {
        consome(STRING_LITERAL);
    }
    consome(DELIMITADOR);
}

// Regra para estrutura de lista
void estruturaLista() {
    consome(DELIMITADOR);
    
    if (strcmp(lookahead.lexema, "]") != 0) {
        expressao();
        while (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ",") == 0) {
            consome(DELIMITADOR);
            expressao();
        }
    }
    
    if (strcmp(lookahead.lexema, "]") == 0) {
        consome(DELIMITADOR);
    } else {
        erroSintatico("Esperava ']' para fechar a lista.");
    }
}

