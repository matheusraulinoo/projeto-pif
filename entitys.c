#include "raylib.h"
#include <stdio.h>

typedef struct{
    Vector2 posicao;
    int vida;
    int vida_maxima;
    bool quebrada;
    float tempo_renascimento;
    float cooldown_renascimento;
}Recursos;


void querbrar_arvore(Recursos *arvore, int dano) {
    if (arvore->quebrada) return; // Não dá para quebrar o que já está quebrado

    arvore->vida -= dano;

    if (arvore->vida <= 0) {
        arvore->quebrada = true;
        arvore->tempo_renascimento = arvore->cooldown_renascimento; // Inicia a contagem regressiva
    }
}


void Atualiza_arvore(Recursos *arvores, int quant) {
    float deltaTime = GetFrameTime(); // Tempo do frame atual em segundos

    for (int i = 0; i < quant; i++) {
        if (arvores[i].quebrada) {
            // Conta o tempo para renascer
            arvores[i].tempo_renascimento -= deltaTime;

            if (arvores[i].tempo_renascimento <= 0.0f) {
                // Renasce a árvore!
                arvores[i].quebrada = false;
                arvores[i].vida = arvores[i].vida_maxima;
            }
        }
    }
}

