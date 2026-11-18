#ifndef INVENTORY_H
#define INVENTORY_H

#include "raylib.h"

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
    ITEM_COUNT
} TipoItem;

typedef enum {
    TIER_MADEIRA,
    TIER_PEDRA,
    TIER_FERRO,
    TIER_OURO,
    TIER_DIAMANTE,
    TIER_MAGNETITA,
    TIER_COUNT
} TierItem;

typedef struct {
    char nome[32];
    int maxQuantidade;
    Color cor;
    Texture2D textura;
    Rectangle recorte;
} Item;

typedef struct {
    Texture2D textura;
    Rectangle recorte;
} Sprite;

typedef struct {
    TipoItem tipo;
    int quant;
    int tier;
    int durability;
} Slot;

typedef struct {
    Slot slots[INV_SIZE];
    int selecionado;
} Inventario;

typedef struct {
    TipoItem tipo;
    TierItem tier;
    int dano;
    int poderCorte;
    int poderMineracao;
    float alcanceSlash;
    float anguloSlash;
    float intervaloAtaque;
    float duracaoAnimacao;
    Texture2D textura;
} Ferramenta;

extern Item itens[ITEM_COUNT];
extern Sprite spritesFerramenta[ITEM_COUNT][TIER_COUNT];
extern int tierDamage[TIER_COUNT];
extern int tierDurability[TIER_COUNT];
extern Color tierCor[TIER_COUNT];

void Itens_Init(void);
void Itens_Unload(void);
int Inv_Add(Inventario *inv, TipoItem id, int amount);
int Inv_AddFerramenta(Inventario *inv, TipoItem tipo, TierItem tier);
void DesenharIcone(TipoItem tipo, int tier, Rectangle caixa);
Rectangle Inv_RetanguloSlot(int startX, int startY, int slot);
int Inv_SlotNoPonto(int startX, int startY, Vector2 ponto);
void Inv_TrocarSlots(Inventario *inv, int origem, int destino);
void Inv_Draw(Inventario *inv, int startX, int startY, int linhas, int origem);
int Inv_Contar(Inventario *inv, TipoItem tipo);
int Inv_Remover(Inventario *inv, TipoItem tipo, int qtd);
int Inv_DescartarSlot(Inventario *inv, int slot, int qtd);
Ferramenta ObterFerramenta(Slot slot);

#endif
