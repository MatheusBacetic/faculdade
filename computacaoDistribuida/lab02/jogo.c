#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/select.h>
#include <unistd.h>
#include "jogo.h"

// Valida a palavra baseada nas regras do README
int validar_palavra(const char *palavra, char letra_esperada) {
    if (palavra == NULL) return 0;
    
    int tamanho = strlen(palavra);
    
    // Regra 1: Tamanho mínimo (5 caracteres)
    if (tamanho < TAMANHO_MIN_PALAVRA) {
        return 0;
    }
    
    // Regra 2: Começar com a letra da rodada (case insensitive)
    if (tolower(palavra[0]) != tolower(letra_esperada)) {
        return 0;
    }
    
    // Regra 3: Conter apenas letras (sem espaços, números ou símbolos)
    for (int i = 0; i < tamanho; i++) {
        if (!isalpha(palavra[i])) {
            return 0;
        }
    }
    
    return 1; // Passou em todas as validações
}

// Utilitário para limpar o buffer recebido
void limpar_string(char *str) {
    str[strcspn(str, "\r\n")] = 0;
}

// Wrapper para enviar mensagem já formatando com a quebra de linha exigida pelo protocolo
int enviar_mensagem(int socket_fd, const char *mensagem) {
    char buffer_envio[TAMANHO_BUFFER];
    
    // Garante que a mensagem termina com \n
    snprintf(buffer_envio, sizeof(buffer_envio), "%s\n", mensagem);
    
    return send(socket_fd, buffer_envio, strlen(buffer_envio), 0);
}

// Função crucial para o jogo: receber dados, mas sem travar infinitamente
int receber_com_timeout(int socket_fd, char *buffer, int tamanho_buffer, int timeout_seg) {
    fd_set set_leitura;
    struct timeval tempo_limite;
    
    // Limpa o conjunto de descritores de arquivo e adiciona nosso socket
    FD_ZERO(&set_leitura);
    FD_SET(socket_fd, &set_leitura);
    
    // Configura o cronômetro do timeout
    tempo_limite.tv_sec = timeout_seg;
    tempo_limite.tv_usec = 0;
    
    memset(buffer, 0, tamanho_buffer);
    
    // O select vai "travar" aqui, mas no máximo até o tempo_limite acabar
    int resultado_select = select(socket_fd + 1, &set_leitura, NULL, NULL, &tempo_limite);
    
    if (resultado_select == -1) {
        return -1; // Erro no select
    } else if (resultado_select == 0) {
        return 0;  // Timeout! O tempo esgotou antes de recebermos algo
    } else {
        // Temos dados prontos para leitura no socket
        int bytes_recebidos = recv(socket_fd, buffer, tamanho_buffer - 1, 0);
        if (bytes_recebidos > 0) {
            limpar_string(buffer); // Limpa o \n para facilitar o processamento depois
        }
        return bytes_recebidos; // Pode ser 0 se o outro lado desconectou
    }
}
