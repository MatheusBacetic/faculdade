#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "jogo.h"

// Estruturas para organizar os dados da partida
typedef struct {
    int fd; // Descritor do socket do jogador
    char nome[MAX_NOME];
    int pontos;
} Jogador;

typedef struct {
    int id_partida;
    Jogador j1;
    Jogador j2;
} Partida;

// Função que roda em uma thread separada para cada par de jogadores
void *gerenciar_partida(void *arg) {
    Partida *p = (Partida *)arg;
    char buffer[TAMANHO_BUFFER];
    char msg[TAMANHO_BUFFER];

    printf("[Partida #%d] Iniciando configuração dos jogadores...\n", p->id_partida);

    // 1. Solicita e recebe o nome do Jogador 1
    enviar_mensagem(p->j1.fd, TIPO_NOME_REQ);
    receber_com_timeout(p->j1.fd, buffer, sizeof(buffer), 30); // 30s para responder
    if (strncmp(buffer, TIPO_NOME_RESP, 5) == 0) {
        strncpy(p->j1.nome, buffer + 5, MAX_NOME - 1);
    } else {
        strcpy(p->j1.nome, "Jogador 1");
    }
    enviar_mensagem(p->j1.fd, TIPO_AGUARDE"Aguardando oponente...");

    // 2. Solicita e recebe o nome do Jogador 2
    enviar_mensagem(p->j2.fd, TIPO_NOME_REQ);
    receber_com_timeout(p->j2.fd, buffer, sizeof(buffer), 30);
    if (strncmp(buffer, TIPO_NOME_RESP, 5) == 0) {
        strncpy(p->j2.nome, buffer + 5, MAX_NOME - 1);
    } else {
        strcpy(p->j2.nome, "Jogador 2");
    }

    // 3. Anuncia o início da partida
    snprintf(msg, sizeof(msg), TIPO_MSG"\n🎮 Batalha: %s vs %s! Boa sorte!", p->j1.nome, p->j2.nome);
    enviar_mensagem(p->j1.fd, msg);
    enviar_mensagem(p->j2.fd, msg);

    // --- LOOP DO JOGO (AS 5 RODADAS) ---
    for (int rodada = 1; rodada <= MAX_RODADAS; rodada++) {
        // Sorteia uma letra maiúscula aleatória (A-Z)
        char letra = 'A' + (rand() % 26);
        
        // Envia a rodada para ambos
        snprintf(msg, sizeof(msg), TIPO_RODADA"%d|%c|%d", rodada, letra, TEMPO_RODADA_SEG);
        enviar_mensagem(p->j1.fd, msg);
        enviar_mensagem(p->j2.fd, msg);

        char palavra_j1[MAX_PALAVRA] = "";
        char palavra_j2[MAX_PALAVRA] = "";

        // Recebe a resposta com timeout (+2 segundos de tolerância para o delay da rede)
        int rec_j1 = receber_com_timeout(p->j1.fd, buffer, sizeof(buffer), TEMPO_RODADA_SEG + 2);
        if (rec_j1 > 0 && strncmp(buffer, TIPO_PALAVRA, 8) == 0) strcpy(palavra_j1, buffer + 8);

        int rec_j2 = receber_com_timeout(p->j2.fd, buffer, sizeof(buffer), TEMPO_RODADA_SEG + 2);
        if (rec_j2 > 0 && strncmp(buffer, TIPO_PALAVRA, 8) == 0) strcpy(palavra_j2, buffer + 8);

        // Validação
        int j1_ok = validar_palavra(palavra_j1, letra);
        int j2_ok = validar_palavra(palavra_j2, letra);

        // Verifica empate na palavra (ninguém pontua se enviarem a mesma)
        if (j1_ok && j2_ok && strcasecmp(palavra_j1, palavra_j2) == 0) {
            j1_ok = 0; j2_ok = 0;
            enviar_mensagem(p->j1.fd, TIPO_RESULTADO"Palavras iguais! 0 pontos.");
            enviar_mensagem(p->j2.fd, TIPO_RESULTADO"Palavras iguais! 0 pontos.");
        } else {
            // Pontua e avisa J1
            if (j1_ok) { p->j1.pontos++; enviar_mensagem(p->j1.fd, TIPO_RESULTADO"+1 ponto! Palavra válida."); }
            else { enviar_mensagem(p->j1.fd, TIPO_RESULTADO"0 pontos. Palavra inválida ou tempo esgotado."); }

            // Pontua e avisa J2
            if (j2_ok) { p->j2.pontos++; enviar_mensagem(p->j2.fd, TIPO_RESULTADO"+1 ponto! Palavra válida."); }
            else { enviar_mensagem(p->j2.fd, TIPO_RESULTADO"0 pontos. Palavra inválida ou tempo esgotado."); }
        }

        // Envia o placar da rodada
        snprintf(msg, sizeof(msg), TIPO_PLACAR"%s|%d|%s|%d", p->j1.nome, p->j1.pontos, p->j2.nome, p->j2.pontos);
        enviar_mensagem(p->j1.fd, msg);
        enviar_mensagem(p->j2.fd, msg);
    }

    // --- FIM DE JOGO ---
    if (p->j1.pontos > p->j2.pontos) snprintf(msg, sizeof(msg), TIPO_FIM"🏆 %s venceu o jogo!", p->j1.nome);
    else if (p->j2.pontos > p->j1.pontos) snprintf(msg, sizeof(msg), TIPO_FIM"🏆 %s venceu o jogo!", p->j2.nome);
    else snprintf(msg, sizeof(msg), TIPO_FIM"🤝 O jogo terminou em empate!");

    enviar_mensagem(p->j1.fd, msg);
    enviar_mensagem(p->j2.fd, msg);

    // Limpeza
    close(p->j1.fd);
    close(p->j2.fd);
    printf("[Partida #%d] Finalizada.\n", p->id_partida);
    free(p); // Libera a memória alocada para a struct Partida
    return NULL;
}

int main(int argc, char *argv[]) {
    int porta = PORTA_PADRAO;
    if (argc > 1) porta = atoi(argv[1]);

    srand(time(NULL)); // Inicializa o gerador de números aleatórios para as letras

    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Cria o socket TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Permite reuso rápido da porta caso o servidor reinicie
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(porta);

    // Vincula o socket à porta
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Erro no bind");
        exit(EXIT_FAILURE);
    }

    // Coloca o socket em modo de escuta
    if (listen(server_fd, 10) == -1) {
        perror("Erro no listen");
        exit(EXIT_FAILURE);
    }

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║      BATALHA DE PALAVRAS — Servidor          ║\n");
    printf("║  Porta: %d                                 ║\n", porta);
    printf("║  Aguardando jogadores (pares de 2)...        ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    int id_partida_atual = 1;

    // Loop principal de aceitação de conexões
    while (1) {
        // Aguarda e aceita o Jogador 1
        int fd1 = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        printf("[+] Jogador conectou. Aguardando par...\n");

        // Aguarda e aceita o Jogador 2
        int fd2 = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        printf("[+] Segundo jogador conectou. Iniciando partida!\n");

        // Prepara os dados da partida dinamicamente para passar para a thread
        Partida *nova_partida = malloc(sizeof(Partida));
        nova_partida->id_partida = id_partida_atual++;
        nova_partida->j1.fd = fd1;
        nova_partida->j1.pontos = 0;
        nova_partida->j2.fd = fd2;
        nova_partida->j2.pontos = 0;

        // Cria a thread
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, gerenciar_partida, nova_partida) != 0) {
            perror("Erro ao criar thread");
            free(nova_partida);
        } else {
            // Desvincula a thread para que seus recursos sejam liberados automaticamente ao terminar
            pthread_detach(thread_id); 
        }
    }

    close(server_fd);
    return 0;
}
