#ifndef MAPA_H
#define MAPA_H

#include "raylib.h"
#include "config.h"

// Colisão círculo x tiles não andáveis (fora do mapa também bloqueia)
int ColideComParede(char mapa[LINHAS][COLS], Vector2 centro, float raio);

// Retorna 1 se existe algum tile 'tile' a menos de 'alcance' px do jogador
int PertoDeTile(char mapa[LINHAS][COLS], Vector2 pos, char tile, float alcance);

// Carrega um mapa .txt para a matriz
int CarregarMapa(const char *caminho, char mapa[LINHAS][COLS]);

// Retorna o id do tile sob a posição (ou -1 se estiver fora do mapa)
int TileEm(char mapa[LINHAS][COLS], Vector2 pos);

// Desenha os tiles do mapa como retângulos coloridos (usado sem imagem de fundo ou no modo debug)
void DesenhaTilesMapa(char mapa[LINHAS][COLS], float alpha);

#endif
