#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "jogo.h"

// Função para ler do teclado com limite de tempo
int ler_teclado_com_timeout(char *buffer, int tamanho, int timeout_seg) {
    fd_set set;
    struct timeval tempo_limite;

    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set); // Monitora a entrada padrão (teclado)

    tempo_limite.tv_sec = timeout_seg;
    tempo_limite.tv_usec = 0;

    // Aguarda o usuário digitar ou o tempo acabar
    int res = select(STDIN_FILENO + 1, &set, NULL, NULL, &tempo_limite);
    
    if (res > 0) {
        if (fgets(buffer, tamanho, stdin) != NULL) {
            limpar_string(buffer); // Remove o \n do final
            return 1; // Sucesso
        }
    }
    return 0; // Timeout ou erro
}

int main(int argc, char *argv[]) {
    char *ip_servidor = "127.0.0.1";
    int porta = PORTA_PADRAO;

    // Permite customizar IP e porta via linha de comando
    if (argc > 1) ip_servidor = argv[1];
    if (argc > 2) porta = atoi(argv[2]);

    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro na criação do socket");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(porta);

    // Converte IPv4 e IPv6 de texto para binário
    if (inet_pton(AF_INET, ip_servidor, &serv_addr.sin_addr) <= 0) {
        perror("Endereço inválido ou não suportado");
        return -1;
    }

    printf("╔══════════════════════════════════════╗\n");
    printf("║     BATALHA DE PALAVRAS — Cliente    ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("  Conectando a %s:%d...\n", ip_servidor, porta);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Falha na conexão com o servidor");
        return -1;
    }
    printf("  Conectado!\n\n");

    char buffer_rx[TAMANHO_BUFFER * 2]; // Buffer para receber mensagens

    // Loop principal de comunicação com o servidor
    while (1) {
        memset(buffer_rx, 0, sizeof(buffer_rx));
        int bytes_recebidos = recv(sock, buffer_rx, sizeof(buffer_rx) - 1, 0);
        
        if (bytes_recebidos <= 0) {
            printf("\n❌ Conexão com o servidor encerrada.\n");
            break;
        }

        // TCP é um fluxo contínuo. O servidor pode enviar duas mensagens muito rápido
        // e elas chegarem coladas. O strtok separa as mensagens usando o '\n'.
        char *linha = strtok(buffer_rx, "\n");
        
        while (linha != NULL) {
            // Analisa o prefixo da mensagem e toma a ação correspondente
            if (strncmp(linha, TIPO_MSG, 4) == 0) {
                printf("%s\n", linha + 4);
            
            } else if (strncmp(linha, TIPO_NOME_REQ, 5) == 0) {
                printf("  Digite seu nome: ");
                fflush(stdout); // Garante que a frase apareça antes de ler
                char nome[MAX_NOME];
                fgets(nome, sizeof(nome), stdin);
                limpar_string(nome);
                
                char msg_envio[TAMANHO_BUFFER];
                snprintf(msg_envio, sizeof(msg_envio), TIPO_NOME_RESP"%s", nome);
                enviar_mensagem(sock, msg_envio);
            
            } else if (strncmp(linha, TIPO_AGUARDE, 8) == 0) {
                printf("  ⏳ %s\n", linha + 8);
            
            } else if (strncmp(linha, TIPO_RODADA, 7) == 0) {
                int rodada, tempo;
                char letra;
                // Extrai os dados da string RODADA|num|letra|tempo
                sscanf(linha + 7, "%d|%c|%d", &rodada, &letra, &tempo);
                
                printf("\n  ╔══════════════════════════════════╗\n");
                printf("  ║        RODADA %d de %d             ║\n", rodada, MAX_RODADAS);
                printf("  ║  Letra: [%c]   Tempo: %d seg     ║\n", letra, tempo);
                printf("  ║  Mínimo: %d caracteres           ║\n", TAMANHO_MIN_PALAVRA);
                printf("  ╚══════════════════════════════════╝\n");
                printf("  Sua palavra: ");
                fflush(stdout);

                char palavra[MAX_PALAVRA];
                // Inicia o cronômetro para o jogador digitar
                if (ler_teclado_com_timeout(palavra, sizeof(palavra), tempo)) {
                    char msg_envio[TAMANHO_BUFFER];
                    snprintf(msg_envio, sizeof(msg_envio), TIPO_PALAVRA"%s", palavra);
                    enviar_mensagem(sock, msg_envio);
                    printf("  Enviado: \"%s\" — aguardando oponente...\n", palavra);
                } else {
                    printf("\n  ⏰ Tempo esgotado!\n");
                    enviar_mensagem(sock, TIPO_TIMEOUT);
                }
            
            } else if (strncmp(linha, TIPO_RESULTADO, 10) == 0) {
                printf("  📋 %s\n", linha + 10);
            
            } else if (strncmp(linha, TIPO_PLACAR, 7) == 0) {
                char n1[MAX_NOME], n2[MAX_NOME];
                int p1, p2;
                sscanf(linha + 7, "%[^|]|%d|%[^|]|%d", n1, &p1, n2, &p2);
                printf("  ┌─────────────────────────────────┐\n");
                printf("  │  PLACAR: %-10s %d x %d %-10s │\n", n1, p1, p2, n2);
                printf("  └─────────────────────────────────┘\n");
            
            } else if (strncmp(linha, TIPO_FIM, 4) == 0) {
                printf("\n%s\n", linha + 4);
                close(sock);
                return 0; // Termina o cliente ao fim do jogo
            }

            // Pega a próxima mensagem colada, se houver
            linha = strtok(NULL, "\n");
        }
    }

    close(sock);
    return 0;
}
