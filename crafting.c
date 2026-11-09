#include "raylib.h"
#include <stdio.h>
#include "inventory.c"

typedef struct {
    TipoItem tipo;
    int qtd;
} Ingrediente;

typedef struct {
    TipoItem resultado;
    int qtdResultado;
    TierItem tier;
    Ingrediente ing[4];
    int totalIng;
} Receita;

int Craftar(Inventario *inv, Receita r) {
    for (int i = 0; i < r.totalIng; i++) {
        if (Inv_Contar(inv, r.ing[i].tipo) < r.ing[i].qtd) {
            printf("Falta: %s\n", itens[r.ing[i].tipo].nome);
            return 0;
        }
    }

    //confere se o resultado cabe antes de gastar material
    int ehFerramenta = itens[r.resultado].maxQuantidade == 1;
    if (ehFerramenta) {
        int temVazio = 0;
        for (int i = 0; i < INV_SIZE; i++)
            if (inv->slots[i].tipo == ITEM_VAZIO) { temVazio = 1; break; }
        if (!temVazio) { printf("Inventario cheio\n"); return 0; }
    }
    for (int i = 0; i < r.totalIng; i++)
        Inv_Remover(inv, r.ing[i].tipo, r.ing[i].qtd);

    if (ehFerramenta) Inv_AddFerramenta(inv, r.resultado, r.tier);
    else              Inv_Add(inv, r.resultado, r.qtdResultado);

    return 1;
}