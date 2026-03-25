#ifndef JOGO_H
#define JOGO_H

#include "protocolo.h"
#include <sys/socket.h>

// Valida se a palavra atende aos critérios do jogo
// Retorna: 1 (válida) ou 0 (inválida)
int validar_palavra(const char *palavra, char letra_esperada);

// Remove quebras de linha (\n ou \r) do final de uma string
void limpar_string(char *str);

// Envia uma mensagem pelo socket garantindo a quebra de linha (\n) no final
// Retorna: o número de bytes enviados ou -1 em caso de erro
int enviar_mensagem(int socket_fd, const char *mensagem);

// Aguarda uma mensagem no socket com um tempo limite (timeout)
// Retorna: 
//  > 0 (bytes recebidos)
//    0 (timeout estourou ou conexão fechada)
//   -1 (erro no select ou recv)
int receber_com_timeout(int socket_fd, char *buffer, int tamanho_buffer, int timeout_seg);

#endif // JOGO_H
