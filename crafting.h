#ifndef CRAFTING_H
#define CRAFTING_H

#include "inventory.h"

typedef struct {
    TipoItem tipo;
    int qtd;
} Ingrediente;

typedef struct {
    const char *nome;
    TipoItem resultado;
    int qtdResultado;
    TierItem tier;
    Ingrediente ing[4];
    int totalIng;
} Receita;

int PodeCraftar(Inventario *inv, Receita r);
int Craftar(Inventario *inv, Receita r);
int DesenharCraftingUI(Receita *receitas, int total, Inventario *inv,
                       int *selecionada, const char *mensagem, Color corMensagem);

#endif
