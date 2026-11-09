#include "raylib.h"
#include <stdio.h>

#define INV_COLS 9
#define INV_ROWS 4
#define INV_SIZE (INV_COLS * INV_ROWS)
#define MAX_STACK 64

// Definição dos tipos de itens
typedef enum {
    ITEM_VAZIO = 0,
    ITEM_MADEIRA,
    ITEM_GRAVETO,
    ITEM_PEDRA,
    ITEM_FERRO,
    ITEM_OURO,
    ITEM_DIAMANTE,
    ITEM_MAGNETITA,
    ITEM_ESPADA,
    ITEM_MACHADO,
    ITEM_PICARETA,
    ITEM_COUNT // Total de tipos de itens
} TipoItem;

typedef enum {
    TIER_MADEIRA,
    TIER_PEDRA,
    TIER_FERRO,
    TIER_OURO,
    TIER_DIAMANTE,
    TIER_MAGNETITA,
    TIER_COUNT // Total de tiers
} TierItem;

// Estrutura de um item do inventário
typedef struct {
    char nome[32];
    int maxQuantidade;
    Color cor;
} Item;

int tierDamage[TIER_COUNT] = {
    [TIER_MADEIRA]   = 4,
    [TIER_PEDRA]     = 5,
    [TIER_FERRO]     = 6,
    [TIER_OURO]      = 4,
    [TIER_DIAMANTE]  = 7,
    [TIER_MAGNETITA] = 8,
};

int tierDurability[TIER_COUNT] = {
    [TIER_MADEIRA]   = 60,
    [TIER_PEDRA]     = 130,
    [TIER_FERRO]     = 250,
    [TIER_OURO]      = 30,
    [TIER_DIAMANTE]  = 1560,
    [TIER_MAGNETITA] = 2000,
};

Color tierCor[TIER_COUNT] = {
    [TIER_MADEIRA]   = (Color){139, 69, 19, 255},
    [TIER_PEDRA]     = (Color){128, 128, 128, 255},
    [TIER_FERRO]     = (Color){192, 192, 192, 255},
    [TIER_OURO]      = (Color){255, 215, 0, 255},
    [TIER_DIAMANTE]  = (Color){0, 191, 255, 255},
    [TIER_MAGNETITA] = (Color){105, 105, 105, 255},
};

typedef struct {
    TipoItem tipo;
    int quant;
    int tier;  
    int durability;
} Slot;

typedef struct {
    Slot slots[INV_SIZE];
    int selecionado; // índice do slot atualmente selecionado
} Inventario;

Item itens[ITEM_COUNT] = {
    { "Vazio", 0, (Color){ 0, 0, 0, 0 } },
    { "Madeira", MAX_STACK, (Color){ 139, 69, 19, 255 } },
    { "Graveto", MAX_STACK, (Color){ 160, 82, 45, 255 } },
    { "Pedra", MAX_STACK, (Color){ 128, 128, 128, 255 } },
    { "Ferro", MAX_STACK, (Color){ 192, 192, 192, 255 } },
    { "Ouro", MAX_STACK, (Color){ 255, 215, 0, 255 } },
    { "Diamante", MAX_STACK, (Color){ 0, 191, 255, 255 } },
    { "Magnetita", MAX_STACK, (Color){ 105, 105, 105, 255 } },
    { "Espada", 1, (Color){ 139, 69, 19, 255 } },
    { "Machado", 1,(Color){128 ,128 ,128 ,255} },
    { "Picareta", 1,(Color){192 ,192 ,192 ,255} },
};

int Inv_Add(Inventario *inv, TipoItem id, int amount) {
    if (id == ITEM_VAZIO || itens[id].maxQuantidade <= 1) return amount;
    int max = itens[id].maxQuantidade;
    // 1) completa stacks existentes
    for (int i = 0; i < INV_SIZE && amount > 0; i++) {
        if (inv->slots[i].tipo == id && inv->slots[i].quant < max) {
            int space = max - inv->slots[i].quant;
            int add = amount < space ? amount : space;
            inv->slots[i].quant += add;
            amount -= add;
        }
    }
    // 2) usa slots vazios
    for (int i = 0; i < INV_SIZE && amount > 0; i++) {
        if (inv->slots[i].tipo == ITEM_VAZIO) {
            int add = amount < max ? amount : max;
            inv->slots[i].tipo = id;
            inv->slots[i].quant = add;
            amount -= add;
        }
    }
    return amount; // sobra (0 = tudo coube)
}

int Inv_AddFerramenta(Inventario *inv, TipoItem tipo, TierItem tier) {
    for (int i = 0; i < INV_SIZE; i++) {
        if (inv->slots[i].tipo == ITEM_VAZIO) {
            inv->slots[i].tipo = tipo;
            inv->slots[i].quant = 1;
            inv->slots[i].tier = tier;
            inv->slots[i].durability = tierDurability[tier];
            return 1; // sucesso
        }
    }
    return 0; // inventário cheio
}

void Inv_Draw(Inventario *inv, int startX, int startY, int linhas) {
    for (int i = 0; i < INV_COLS * linhas; i++) {
        int x = startX + (i % INV_COLS) * 52;
        int y = startY + (i / INV_COLS) * 52;
        Slot s = inv->slots[i];

        DrawRectangle(x, y, 48, 48, DARKGRAY);
        DrawRectangleLines(x, y, 48, 48, i == inv->selecionado ? YELLOW : BLACK);

        if (s.tipo != ITEM_VAZIO) {
            Color cor = itens[s.tipo].maxQuantidade == 1 ? tierCor[s.tier]
                                                         : itens[s.tipo].cor;
            DrawRectangle(x + 8, y + 8, 32, 32, cor);
            if (s.quant > 1)
                DrawText(TextFormat("%d", s.quant), x + 28, y + 32, 14, WHITE);
        }
    }
}

int Inv_Contar(Inventario *inv, TipoItem tipo) {
    int total = 0;
    for (int i = 0; i < INV_SIZE; i++)
        if (inv->slots[i].tipo == tipo) total += inv->slots[i].quant;
    return total;
}

// Remove qtd itens; retorna 1 se conseguiu, 0 se não tinha o suficiente
int Inv_Remover(Inventario *inv, TipoItem tipo, int qtd) {
    if (Inv_Contar(inv, tipo) < qtd) return 0;
    for (int i = 0; i < INV_SIZE && qtd > 0; i++) {
        if (inv->slots[i].tipo != tipo) continue;
        int tira = qtd < inv->slots[i].quant ? qtd : inv->slots[i].quant;
        inv->slots[i].quant -= tira;
        qtd -= tira;
        if (inv->slots[i].quant == 0) inv->slots[i] = (Slot){0};
    }
    return 1;
}