#ifndef COMBATE_H
#define COMBATE_H

#include "raylib.h"
#include "entities.h"
#include "inventory.h"

// Cada minério exige no minimo a picareta do tier indicado
int PicaretaPodeQuebrar(Ferramenta ferramenta, TierItem tierMinimo);

// Aplica o slash nos monstros. Retorna true se acertou algum
bool AtacarMonstros(Vector2 jogador, Vector2 direcaoSlash, Ferramenta ferramenta);

// Aplica o slash numa lista de recursos (arvores/minerios) e entrega o item ao quebrar.
// Retorna true se acertou algum
bool AtacarRecursos(Recursos *lista, Vector2 jogador, Vector2 direcaoSlash,
                    Ferramenta ferramenta, int poder, TipoItem recompensa,
                    Inventario *inv);

// Consome durabilidade da ferramenta equipada (quebra ao chegar em zero)
void ConsumirDurabilidade(Inventario *inv, Ferramenta *ferramentaAtual);

// Animacao visual do slash + ferramenta equipada
void DesenhaSlash(Vector2 jogador, Vector2 direcaoSlash,
                  Ferramenta ferramentaAnimacao, float timerSlash, float duracaoSlash);

#endif
