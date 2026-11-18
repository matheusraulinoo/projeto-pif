#ifndef ENTITIES_H
#define ENTITIES_H

#include "raylib.h"

typedef struct Recursos {
    Vector2 posicao;
    Vector2 offset_colisao;
    float raio_colisao;
    int vida;
    int vida_maxima;
    bool quebrado;
    bool colide_quebrado;
    float tempo_renascimento;
    float cooldown_renascimento;
    struct Recursos *proximo;
} Recursos;

typedef enum {
    MONSTRO_SLIME,
    MONSTRO_ESQUELETO,
    MONSTRO_CARANGUEJO,
    TOTAL_TIPOS_MONSTRO
} TipoMonstro;

typedef struct {
    TipoMonstro tipo;
    Vector2 posicao;
    float velocidade;
    bool elite;
    float vida;
    float vida_maxima;
    float ataque;
    float defesa;
    const char *nome;
    bool morto;
} Monstro;

void inserir_recurso(Recursos **cabeca, Recursos novo);
void quebrar_recurso(Recursos *recurso, int dano);
void atualiza_recurso(Recursos *cabeca);
void liberar_recursos(Recursos *cabeca);
int TemRecursoPerto(Vector2 pos, float distMin, Recursos *lista);

#endif
