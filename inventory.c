#include "raylib.h"
#include <stdio.h>

#define SLOT_TAM      64
#define SLOT_GAP      6
#define SLOT_PASSO    (SLOT_TAM + SLOT_GAP)
#define ICONE_MARGEM  (SLOT_TAM / 16)
#define ICONE_TAM     (SLOT_TAM - 2 * ICONE_MARGEM)
#define INV_PAD       10
#define INV_LARGURA   (INV_COLS * SLOT_PASSO - SLOT_GAP)
#define INV_ALTURA    (INV_ROWS * SLOT_PASSO - SLOT_GAP)
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
    ITEM_ESCUDO,
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
    Texture2D textura;
    Rectangle recorte;
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

Item itens[ITEM_COUNT];

static Item NovoItem(const char *nome, int max, Color cor, const char *caminho) {
    Item it = { 0 };
    snprintf(it.nome, sizeof(it.nome), "%s", nome);
    it.maxQuantidade = max;
    it.cor = cor;

    Image img = LoadImage(caminho);
    if (img.data != NULL) {
        it.recorte = GetImageAlphaBorder(img, 0.0f);
        if (it.recorte.width <= 0 || it.recorte.height <= 0)
            it.recorte = (Rectangle){ 0, 0, (float)img.width, (float)img.height };
        it.textura = LoadTextureFromImage(img);
        SetTextureFilter(it.textura, TEXTURE_FILTER_POINT);
        UnloadImage(img);
    } else {
        TraceLog(LOG_WARNING, "Imagem nao carregou: %s", caminho);
    }
    return it;
}

void Itens_Init(void) {
    itens[ITEM_VAZIO]     = (Item){ 0 };
    itens[ITEM_MADEIRA]   = NovoItem("Madeira",   MAX_STACK, (Color){139, 69, 19, 255},  "pictures/madeira.png");
    itens[ITEM_GRAVETO]   = NovoItem("Graveto",   MAX_STACK, (Color){160, 110, 50, 255}, "pictures/graveto.png");
    itens[ITEM_PEDRA]     = NovoItem("Pedra",     MAX_STACK, GRAY,                       "pictures/pedra.png");
    itens[ITEM_FERRO]     = NovoItem("Ferro",     MAX_STACK, LIGHTGRAY,                  "pictures/ferro.png");
    itens[ITEM_OURO]      = NovoItem("Ouro",      MAX_STACK, GOLD,                       "pictures/ouro.png");
    itens[ITEM_DIAMANTE]  = NovoItem("Diamante",  MAX_STACK, SKYBLUE,                    "pictures/diamante.png");
    itens[ITEM_MAGNETITA] = NovoItem("Magnetita", MAX_STACK, DARKGRAY,                   "pictures/magnetita.png");
    itens[ITEM_ESPADA]    = NovoItem("Espada",    1,         WHITE,                      "pictures/espada.png");
    itens[ITEM_MACHADO]   = NovoItem("Machado",   1,         WHITE,                      "pictures/machado.png");
    itens[ITEM_PICARETA]  = NovoItem("Picareta",  1,         WHITE,                      "pictures/picareta.png");
    itens[ITEM_ESCUDO]    = NovoItem("Escudo",    1,         WHITE,                      "pictures/escudo.png");

    for (int i = 1; i < ITEM_COUNT; i++) {
    if (itens[i].textura.id == 0)
        TraceLog(LOG_WARNING, "Textura nao carregou: %s", itens[i].nome);
    else
        SetTextureFilter(itens[i].textura, TEXTURE_FILTER_POINT);
    }
}

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
        int x = startX + (i % INV_COLS) * SLOT_PASSO;
        int y = startY + (i / INV_COLS) * SLOT_PASSO;
        Slot s = inv->slots[i];

        DrawRectangle(x, y, SLOT_TAM, SLOT_TAM, DARKGRAY);
        DrawRectangleLines(x, y, SLOT_TAM, SLOT_TAM, i == inv->selecionado ? YELLOW : BLACK);

        if (s.tipo != ITEM_VAZIO) {
            Texture2D tex = itens[s.tipo].textura;

            if (tex.id != 0) {
                Rectangle src = itens[s.tipo].recorte;
                float maior = src.width > src.height ? src.width : src.height;
                float escala = ICONE_TAM / maior;
                float w = src.width * escala;
                float h = src.height * escala;
                Rectangle destino = { x + (SLOT_TAM - w) / 2.0f, y + (SLOT_TAM - h) / 2.0f, w, h };
                DrawTexturePro(tex, src, destino, (Vector2){ 0, 0 }, 0.0f, WHITE);
            } else {
                Color cor = itens[s.tipo].maxQuantidade == 1 ? tierCor[s.tier]
                                                             : itens[s.tipo].cor;
                DrawRectangle(x + ICONE_MARGEM, y + ICONE_MARGEM, ICONE_TAM, ICONE_TAM, cor);
            }

            if (s.quant > 1)
                DrawText(TextFormat("%d", s.quant),
                         x + SLOT_TAM - SLOT_TAM / 3, y + SLOT_TAM - SLOT_TAM / 3,
                         SLOT_TAM / 3, WHITE);
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
void Itens_Unload(void) {
    for (int i = 1; i < ITEM_COUNT; i++) UnloadTexture(itens[i].textura);
}