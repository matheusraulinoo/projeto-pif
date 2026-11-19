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

// Lista de receitas (definida em crafting.c)
extern Receita receitasCraft[];
int TotalReceitas(void);

int PodeCraftar(Inventario *inv, Receita r);
int Craftar(Inventario *inv, Receita r);
int DesenharCraftingUI(Receita *receitas, int total, Inventario *inv,
                       int *selecionada, const char *mensagem, Color corMensagem);

#endif
