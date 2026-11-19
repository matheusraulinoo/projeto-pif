#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"

#define LARGURA_TELA 800
#define ALTURA_TELA  450

#define TILE        32
#define MAPA_COLS   100
#define MAPA_LINHAS 75

#define MAPA_LARGURA (MAPA_COLS * TILE)   // 3200
#define MAPA_ALTURA  (MAPA_LINHAS * TILE) // 2400

#define COLS 100
#define LINHAS 75

#define MAPA_ILHA    "mapa.txt"
#define MAPA_CASA    "casa.txt"
#define MAPA_CAVERNA "caverna.txt"
#define TILE_PORTA_CASA   7
#define TILE_PORTA_CAVERNA 8

#define CASA_SPAWN_X    (52 * TILE + TILE / 2.0f)
#define CASA_SPAWN_Y    (38 * TILE + TILE / 2.0f)
#define CAVERNA_SPAWN_X (94 * TILE + TILE / 2.0f)
#define CAVERNA_SPAWN_Y (71 * TILE + TILE / 2.0f)

// Sprite do personagem (spritesheet 4x4, frames de 32x32)
#define SPRITE_TAM      32
#define SPRITE_FRAMES   4
#define SPRITE_VEL_ANIM 0.12f   // segundos por frame
#define CORACOES        5
#define VIDA_JOGADOR    (CORACOES * 2.0f)   // 10 pontos = 5 corações

#define DANO_AGUA     1.5f       // pontos por segundo (meio coração/s)


// Ciclo dia/noite
#define HORA_INICIO      6.0f    // o jogo começa às 06:00
#define ESCURIDAO_MAXIMA 0.75f   // opacidade máxima da noite (0 = sem escuro, 1 = preto)

// Telas do jogo
#define DURACAO_TELA_MORTE 10.0f  // segundos na tela de morte (tempo para digitar o nome)
#define NUM_ESTRELAS       90

#define LAVA_CUSTO 5

typedef enum {
    TELA_INICIO = 0,
    TELA_JOGO,
    TELA_MORTE
} TelaJogo;

typedef enum {
    LOCAL_ILHA,
    LOCAL_CASA,
    LOCAL_CAVERNA
} LocalAtual;

#endif
