/* ===================================================================
 * PROJETO COMPILADORES - FASE 1 (Análise Léxica e Sintática)
 * * Aluno(s): Matheus Veiga Bacetic Joaquim RA: 10425638 | Beatriz Barbosa RA: 10354067
 * * Compilação: gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador
 * =================================================================== */

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
TInfoAtomo lookahead;

// 3. Protótipos das Funções
void iniciarAnalisador(char *nomeArquivo);
TInfoAtomo obter_atomo(); // Nome exigido pelo item OBJETIVO
void imprimirToken(TInfoAtomo token);
void fecharAnalisador();
char* nomeDoTipo(TAtomo tipo);
void erroLexico(char *lexema);
TAtomo classificarLexema(char *lexema);

// --- Protótipos do Analisador Sintático ---
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
void expSimples();
void termo();
void fator();
void estruturaLista();

// --- FUNÇÃO PRINCIPAL ---
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte.py>\n", argv[0]);
        return 1;
    }

    iniciarAnalisador(argv[1]);

    // Pega o primeiro token para iniciar a análise sintática
    lookahead = obter_atomo();
    
    // Inicia a análise sintática pela raiz da gramática
    programa();
    
    // Se a função programa() terminar sem disparar erros, o código está correto!
    if (lookahead.tipo == EOS) {
        printf("Análise Sintática concluída com sucesso! Nenhum erro encontrado.\n");
        fprintf(saida, "Análise Sintática concluída com sucesso!\n");
    } else {
        erroSintatico("Código extra encontrado após o fim do programa principal.");
    }

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
// =========================================================
//               ANALISADOR SINTÁTICO (ETAPA 3)
// =========================================================

void erroSintatico(char *mensagem) {
    printf("ERRO SINTÁTICO\n");
    printf("Linha: %d\n", lookahead.linha);
    printf("Token incorreto: %s\n", lookahead.lexema);
    printf("Detalhe: %s\n", mensagem);
    
    fprintf(saida, "ERRO SINTÁTICO na linha %d: token incorreto '%s'. %s\n", 
            lookahead.linha, lookahead.lexema, mensagem);
            
    fecharAnalisador();
    exit(1); // Encerra o processo imediatamente, conforme especificação [cite: 147]
}

void consome(TAtomo tipo_esperado) {
    if (lookahead.tipo == tipo_esperado) {
        // Se o token é o esperado, "consome" ele e pega o próximo [cite: 8, 16]
        lookahead = obter_atomo();
    } else {
        // Se não for, dispara o erro sintático
        char mensagemErro[100];
        sprintf(mensagemErro, "Esperava token do tipo %s, mas encontrou %s", 
                nomeDoTipo(tipo_esperado), nomeDoTipo(lookahead.tipo));
        erroSintatico(mensagemErro);
    }
}

// Regra: Programa -> Comando Programa | ε
void programa() {
    // Enquanto não chegar no fim do arquivo, tenta ler comandos
    while (lookahead.tipo != EOS) {
        comando();
    }
}

// Regra: Comando -> Atribuicao | Condicional | Repeticao | ComandoPrint | ComandoInput
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
            erroSintatico("Palavra reservada nao esperada no inicio de um comando.");
        }
    } else {
        erroSintatico("Inicio de comando invalido.");
    }
}

// Atualizacao da Atribuicao para aceitar Input
void atribuicao() {
    consome(IDENTIFICADOR);
    if (strcmp(lookahead.lexema, "=") == 0) {
        consome(DELIMITADOR);
        if (lookahead.tipo == PALAVRA_RESERVADA && strcmp(lookahead.lexema, "input") == 0) {
            comandoInput();
        } else {
            expressao();
        }
    } else {
        erroSintatico("Esperava '=' para atribuicao.");
    }
}

// Regra: ComandoPrint -> "print" ListaExpressoes
void comandoPrint() {
    consome(PALAVRA_RESERVADA); // consome o 'print'
    listaExpressoes(); // Precisaremos criar essa função
}

// Regra: Expressao -> ExpSimples ( OperadorRelacional ExpSimples )?
void expressao() {
    expSimples();
    if (lookahead.tipo == OPERADOR_RELACIONAL) {
        consome(OPERADOR_RELACIONAL);
        expSimples();
    }
}

// Regra: ExpSimples -> Termo ( OperadorAritmetico Termo )*
void expSimples() {
    // Tratamento opcional de sinal unário (+ ou -)
    if (lookahead.tipo == OPERADOR_ARITMETICO && 
       (strcmp(lookahead.lexema, "+") == 0 || strcmp(lookahead.lexema, "-") == 0)) {
        consome(OPERADOR_ARITMETICO);
    }
    
    termo();
    // Mudança de IF para WHILE para aceitar x = 1 + 2 + 3
    while (lookahead.tipo == OPERADOR_ARITMETICO) {
        consome(OPERADOR_ARITMETICO);
        termo();
    }
}

// Regra: Termo -> Fator | OperadorLogico Fator
void termo() {
    if (lookahead.tipo == OPERADOR_LOGICO && strcmp(lookahead.lexema, "not") == 0) {
        consome(OPERADOR_LOGICO);
    }
    fator();
    // Opcional: Adicionar loop aqui se quiser suportar multiplicacoes/divisoes em sequencia
}

// Regra: Fator -> IDENTIFICADOR | NUMERO_INTEIRO | BOOLEANO | "[" Lista "]" | "(" Expressao ")" | STRING_LITERAL
void fator() {
    if (lookahead.tipo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);
    } else if (lookahead.tipo == NUMERO_INTEIRO) {
        consome(NUMERO_INTEIRO);
    } else if (lookahead.tipo == BOOLEANO) {
        consome(BOOLEANO);
    } else if (lookahead.tipo == STRING_LITERAL) {
        consome(STRING_LITERAL);
    } else if (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, "[") == 0) {
        estruturaLista(); // CHAMA A NOVA REGRA DE LISTA AQUI!
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

// Regra: ListaExpressoes -> Expressao ( "," Expressao )*
void listaExpressoes() {
    expressao();
    while (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ",") == 0) {
        consome(DELIMITADOR);
        expressao();
    }
}

// Regra: Condicional -> "if" Expressao ":" Comando ( "else" ":" Comando )?
void condicional() {
    consome(PALAVRA_RESERVADA); // if
    expressao();
    if (strcmp(lookahead.lexema, ":") == 0) {
        consome(PONTUACAO);
    } else {
        erroSintatico("Esperava ':' apos a expressao do if.");
    }
    comando(); // No MiniPython, apenas um comando [cite: 61]

    if (lookahead.tipo == PALAVRA_RESERVADA && strcmp(lookahead.lexema, "else") == 0) {
        consome(PALAVRA_RESERVADA); // else
        if (strcmp(lookahead.lexema, ":") == 0) {
            consome(PONTUACAO);
        } else {
            erroSintatico("Esperava ':' apos o else.");
        }
        comando();
    }
}

// Regra: Repeticao -> "while" Expressao ":" Comando | "for" ID "in" "range" "(" Exp ")" ":" Comando
void repeticao() {
    if (strcmp(lookahead.lexema, "while") == 0) {
        consome(PALAVRA_RESERVADA);
        expressao();
        consome(PONTUACAO); // :
        comando();
    } else { // for
        consome(PALAVRA_RESERVADA); // for
        consome(IDENTIFICADOR);
        
        // 'in' é operador lógico, 'range' é palavra reservada
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
        
        consome(DELIMITADOR); // (
        expressao();
        consome(DELIMITADOR); // )
        consome(PONTUACAO);    // :
        comando();
    }
}

// Regra: ComandoInput -> ID "=" "input" "(" STRING_LITERAL ")"
// Nota: Chamada dentro de atribuicao() quando detecta a palavra 'input'
void comandoInput() {
    consome(PALAVRA_RESERVADA); // input
    consome(DELIMITADOR);       // (
    if (lookahead.tipo == STRING_LITERAL) {
        consome(STRING_LITERAL);
    }
    consome(DELIMITADOR);       // )
}

// Adicione esta função junto com as outras do sintático
// Regra: Lista -> "[" ( Expressao ( "," Expressao )* )? "]"
void estruturaLista() {
    consome(DELIMITADOR); // consome o '['
    
    // Se não for lista vazia, tem elementos
    if (strcmp(lookahead.lexema, "]") != 0) {
        expressao();
        while (lookahead.tipo == DELIMITADOR && strcmp(lookahead.lexema, ",") == 0) {
            consome(DELIMITADOR);
            expressao();
        }
    }
    
    // Agora tem que fechar com ']'
    if (strcmp(lookahead.lexema, "]") == 0) {
        consome(DELIMITADOR);
    } else {
        erroSintatico("Esperava ']' para fechar a lista.");
    }
}