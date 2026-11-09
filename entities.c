#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Recursos {
    Vector2 posicao;
    Vector2 offset_colisao;
    float raio_colisao;
    int vida;
    int vida_maxima;
    bool quebrado;
    float tempo_renascimento;
    float cooldown_renascimento;
    struct Recursos *proximo; // Ponteiro para o próximo recurso na lista
}Recursos;

typedef struct {
    Vector2 posicao;
    float velocidade;
    int nivel; //isso aqui é pra usar na progressão do jogo depois
    //eu gosto muito do jeito que risk of rain faz spawn de monstro, se vcs quiserem me perguntar sobre dps eu explico

    float vida;
    float vida_maxima;
    float ataque;
    float defesa;

    bool morto;
} Monstro;
/*
a ideia é:
<A ataca B> 
B.vida -= max(0, A.ataque - B.defesa);
if B.vida <= 0;
    B.morto = true;
*/

// Cria um nó com os dados de "novo" e o coloca no fim da lista.
void inserir_recurso(Recursos **cabeca, Recursos novo) {
    Recursos *no = (Recursos *)malloc(sizeof(Recursos));
    if (no == NULL) return;

    *no = novo;
    no->proximo = NULL;

    if (*cabeca == NULL) {
        *cabeca = no;
        return;
    }

    Recursos *atual = *cabeca;
    while (atual->proximo != NULL) atual = atual->proximo;
    atual->proximo = no;
}


void quebrar_recurso(Recursos *recurso, int dano) {
    if (recurso->quebrado) return; // Não dá para quebrar o que já está quebrado

    recurso->vida -= dano;

    if (recurso->vida <= 0) {
        recurso->quebrado = true;
        recurso->tempo_renascimento = recurso->cooldown_renascimento; // Inicia a contagem regressiva
    }
}


void atualiza_recurso(Recursos *cabeca) {
    float deltaTime = GetFrameTime(); // Tempo do frame atual em segundos

    for (Recursos *recurso = cabeca; recurso != NULL; recurso = recurso->proximo) {
        if (recurso->quebrado) {
            // Conta o tempo para renascer
            recurso->tempo_renascimento -= deltaTime;

            if (recurso->tempo_renascimento <= 0.0f) {
                // Renasce a árvore!
                recurso->quebrado = false;
                recurso->vida = recurso->vida_maxima;
            }
        }
    }
}


// Libera todos os nós da lista.
void liberar_recursos(Recursos *cabeca) {
    while (cabeca != NULL) {
        Recursos *proximo = cabeca->proximo;
        free(cabeca);
        cabeca = proximo;
    }
}