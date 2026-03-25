#ifndef PROTOCOLO_H
#define PROTOCOLO_H

// Configurações do Jogo
#define PORTA_PADRAO 7070
#define MAX_RODADAS 5
#define TEMPO_RODADA_SEG 10
#define TAMANHO_MIN_PALAVRA 5

// Configurações de Rede
#define TAMANHO_BUFFER 512
#define MAX_NOME 50
#define MAX_PALAVRA 100

// Prefixos do Protocolo (Servidor -> Cliente)
#define TIPO_MSG "MSG|"
#define TIPO_NOME_REQ "NOME|"
#define TIPO_AGUARDE "AGUARDE|"
#define TIPO_RODADA "RODADA|"
#define TIPO_RESULTADO "RESULTADO|"
#define TIPO_PLACAR "PLACAR|"
#define TIPO_FIM "FIM|"

// Prefixos do Protocolo (Cliente -> Servidor)
#define TIPO_NOME_RESP "NOME|"
#define TIPO_PALAVRA "PALAVRA|"
#define TIPO_TIMEOUT "TIMEOUT|"

#endif // PROTOCOLO_H
