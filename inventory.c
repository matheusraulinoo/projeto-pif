#include "raylib.h"
#include <stdio.h>
#include "raymath.h"

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

typedef struct {
    Texture2D textura;
    Rectangle recorte;
} Sprite;

// [tipo da ferramenta][tier]; só as ferramentas usam isso
Sprite spritesFerramenta[ITEM_COUNT][TIER_COUNT];

static const char *tierArquivo[TIER_COUNT] = {
    "madeira", "pedra", "ferro", "ouro", "diamante", "magnetita"
};

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

static Sprite CarregarSprite(const char *caminho) {
    Sprite sp = { 0 };
    Image img = LoadImage(caminho);
    if (img.data != NULL) {
        sp.recorte = GetImageAlphaBorder(img, 0.0f);
        if (sp.recorte.width <= 0 || sp.recorte.height <= 0)
            sp.recorte = (Rectangle){ 0, 0, (float)img.width, (float)img.height };
        sp.textura = LoadTextureFromImage(img);
        SetTextureFilter(sp.textura, TEXTURE_FILTER_POINT);
        UnloadImage(img);
    } else {
        TraceLog(LOG_WARNING, "Imagem nao carregou: %s", caminho);
    }
    return sp;
}

static Item NovoItem(const char *nome, int max, Color cor, const char *caminho) {
    Item it = { 0 };
    snprintf(it.nome, sizeof(it.nome), "%s", nome);
    it.maxQuantidade = max;
    it.cor = cor;
    if (caminho != NULL) {               // ferramentas passam NULL
        Sprite sp = CarregarSprite(caminho);
        it.textura = sp.textura;
        it.recorte = sp.recorte;
    }
    return it;
}

static void CarregarFerramenta(TipoItem tipo, const char *base) {
    for (int t = 0; t < TIER_COUNT; t++) {
        char caminho[96];
        snprintf(caminho, sizeof(caminho), "pictures/%s_%s.png", base, tierArquivo[t]);
        spritesFerramenta[tipo][t] = CarregarSprite(caminho);
    }
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
    itens[ITEM_ESPADA]   = NovoItem("Espada",   1, WHITE, NULL);
    itens[ITEM_MACHADO]  = NovoItem("Machado",  1, WHITE, NULL);
    itens[ITEM_PICARETA] = NovoItem("Picareta", 1, WHITE, NULL);
    itens[ITEM_ESCUDO]   = NovoItem("Escudo",   1, WHITE, NULL);

    CarregarFerramenta(ITEM_ESPADA,   "espada");
    CarregarFerramenta(ITEM_MACHADO,  "machado");
    CarregarFerramenta(ITEM_PICARETA, "picareta");
    CarregarFerramenta(ITEM_ESCUDO,   "escudo");
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

static Sprite ObterSprite(TipoItem tipo, int tier) {
    if (itens[tipo].maxQuantidade == 1)
        return spritesFerramenta[tipo][tier];
    return (Sprite){ itens[tipo].textura, itens[tipo].recorte };
}

// Desenha o ícone do item centralizado e proporcional dentro de 'caixa'
void DesenharIcone(TipoItem tipo, int tier, Rectangle caixa) {
    Sprite sp = ObterSprite(tipo, tier);

    if (sp.textura.id != 0) {
        Rectangle src = sp.recorte;
        float maior = src.width > src.height ? src.width : src.height;
        float escala = caixa.width / maior;
        float w = src.width * escala;
        float h = src.height * escala;
        Rectangle dst = { caixa.x + (caixa.width - w) / 2.0f,
                          caixa.y + (caixa.height - h) / 2.0f, w, h };
        DrawTexturePro(sp.textura, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
    } else {
        Color cor = itens[tipo].maxQuantidade == 1 ? tierCor[tier] : itens[tipo].cor;
        DrawRectangleRec(caixa, cor);
    }
}

void Inv_Draw(Inventario *inv, int startX, int startY, int linhas) {
    for (int i = 0; i < INV_COLS * linhas; i++) {
        int x = startX + (i % INV_COLS) * SLOT_PASSO;
        int y = startY + (i / INV_COLS) * SLOT_PASSO;
        Slot s = inv->slots[i];

        DrawRectangle(x, y, SLOT_TAM, SLOT_TAM, DARKGRAY);
        DrawRectangleLines(x, y, SLOT_TAM, SLOT_TAM, i == inv->selecionado ? YELLOW : BLACK);

        if (s.tipo != ITEM_VAZIO) {

            DesenharIcone(
                s.tipo,
                s.tier,
                (Rectangle){
                    x + ICONE_MARGEM,
                    y + ICONE_MARGEM,
                    ICONE_TAM,
                    ICONE_TAM
                }
            );

            // Barra de durabilidade das ferramentas
            if (s.tipo == ITEM_ESPADA ||
                s.tipo == ITEM_MACHADO ||
                s.tipo == ITEM_PICARETA ||
                s.tipo == ITEM_ESCUDO) {

                if (s.tier >= 0 && s.tier < TIER_COUNT) {

                    int durabilidadeMaxima = tierDurability[s.tier];

                    if (durabilidadeMaxima > 0 &&
                        s.durability < durabilidadeMaxima) {

                        float porcentagem = (float)s.durability /
                                            durabilidadeMaxima;

                        porcentagem = Clamp(porcentagem, 0.0f, 1.0f);

                        Color corBarra = GREEN;

                        if (porcentagem <= 0.25f) {
                            corBarra = RED;
                        } else if (porcentagem <= 0.5f) {
                            corBarra = YELLOW;
                        }

                        int larguraBarra = SLOT_TAM - 10;
                        int alturaBarra = 4;

                        int xBarra = x + 5;
                        int yBarra = y + SLOT_TAM - 7;

                        // Fundo
                        DrawRectangle(
                            xBarra,
                            yBarra,
                            larguraBarra,
                            alturaBarra,
                            BLACK
                        );

                        // Durabilidade restante
                        DrawRectangle(
                            xBarra,
                            yBarra,
                            (int)(larguraBarra * porcentagem),
                            alturaBarra,
                            corBarra
                        );
                    }
                }
            }

            if (s.quant > 1) {
                DrawText(
                    TextFormat("%d", s.quant),
                    x + SLOT_TAM - SLOT_TAM / 3,
                    y + SLOT_TAM - SLOT_TAM / 3,
                    SLOT_TAM / 3,
                    WHITE
                );
            }
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
    for (int i = 1; i < ITEM_COUNT; i++) {
        UnloadTexture(itens[i].textura);
        for (int t = 0; t < TIER_COUNT; t++)
            UnloadTexture(spritesFerramenta[i][t].textura);
    }
}

typedef struct {
    TipoItem tipo;
    TierItem tier;

    int dano;
    int poderCorte;
    int poderMineracao;

    float alcanceSlash;
    float anguloSlash;
    float intervaloAtaque;

    Texture2D textura;
} Ferramenta;

Ferramenta ObterFerramenta(Slot slot) {

    Ferramenta f = {
        .tipo = ITEM_VAZIO,
        .tier = TIER_MADEIRA,
        .dano = 1,
        .poderCorte = 1,
        .poderMineracao = 0,
        .alcanceSlash = 60.0f,
        .anguloSlash = 100.0f,
        .intervaloAtaque = 0.5f,
        .textura = {0}
    };

    if (slot.tipo != ITEM_ESPADA &&
        slot.tipo != ITEM_MACHADO &&
        slot.tipo != ITEM_PICARETA) {
        return f;
    }

    if (slot.tier < 0 || slot.tier >= TIER_COUNT)
        return f;

    f.tipo = slot.tipo;
    f.tier = (TierItem)slot.tier;
    f.dano = tierDamage[slot.tier];
    f.alcanceSlash = 120.0f;
    f.intervaloAtaque = 0.4f;

    switch (slot.tipo) {
        case ITEM_ESPADA:
            f.poderCorte = 1;
            break;

        case ITEM_MACHADO:
            f.poderCorte = tierDamage[slot.tier];
            break;

        case ITEM_PICARETA:
            f.poderCorte = 0;
            f.poderMineracao = tierDamage[slot.tier];
            break;

        default:
            break;
    }

    f.textura = spritesFerramenta[slot.tipo][slot.tier].textura;

    return f;
}