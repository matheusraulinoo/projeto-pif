#include "raylib.h"
#include <stdio.h>
#include "inventory.c"

typedef struct {
    TipoItem tipo;
    int qtd;
} Ingrediente;

typedef struct {
    const char *nome;       // nome exibido na interface (ex: "Espada de Madeira")
    TipoItem resultado;
    int qtdResultado;
    TierItem tier;
    Ingrediente ing[4];
    int totalIng;
} Receita;

static int TemSlotVazio(Inventario *inv) {
    for (int i = 0; i < INV_SIZE; i++)
        if (inv->slots[i].tipo == ITEM_VAZIO) return 1;
    return 0;
}

// Confere (sem gastar nada) se a receita pode ser feita agora.
// Usado pela interface para colorir as receitas disponíveis.
int PodeCraftar(Inventario *inv, Receita r) {
    for (int i = 0; i < r.totalIng; i++)
        if (Inv_Contar(inv, r.ing[i].tipo) < r.ing[i].qtd) return 0;

    // ferramentas (stack 1) precisam de um slot livre
    if (itens[r.resultado].maxQuantidade == 1 && !TemSlotVazio(inv)) return 0;

    return 1;
}

int Craftar(Inventario *inv, Receita r) {
    for (int i = 0; i < r.totalIng; i++) {
        if (Inv_Contar(inv, r.ing[i].tipo) < r.ing[i].qtd) {
            printf("Falta: %s\n", itens[r.ing[i].tipo].nome);
            return 0;
        }
    }

    //confere se o resultado cabe antes de gastar material
    int ehFerramenta = itens[r.resultado].maxQuantidade == 1;
    if (ehFerramenta && !TemSlotVazio(inv)) {
        printf("Inventario cheio\n");
        return 0;
    }

    for (int i = 0; i < r.totalIng; i++)
        Inv_Remover(inv, r.ing[i].tipo, r.ing[i].qtd);

    if (ehFerramenta) Inv_AddFerramenta(inv, r.resultado, r.tier);
    else              Inv_Add(inv, r.resultado, r.qtdResultado);

    return 1;
}


// Interface básica de crafting

#define CRAFT_X            200
#define CRAFT_Y            60
#define CRAFT_LARGURA      400
#define CRAFT_ALTURA       310
#define CRAFT_LINHA_ALTURA 62

int DesenharCraftingUI(Receita *receitas, int total, Inventario *inv,
                       int *selecionada, const char *mensagem, Color corMensagem)
{
    if (total <= 0) return 0;

    if (*selecionada < 0) *selecionada = 0;
    if (*selecionada >= total) *selecionada = total - 1;

    // Navegação pelo teclado
    if (IsKeyPressed(KEY_DOWN)) *selecionada = (*selecionada + 1) % total;
    if (IsKeyPressed(KEY_UP))   *selecionada = (*selecionada + total - 1) % total;

    int craftar = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE);

    // Navegação/clique pelo mouse
    int mx = GetMouseX();
    int my = GetMouseY();
    int topoLista = CRAFT_Y + 44;

    for (int i = 0; i < total; i++) {
        Rectangle linha = {
            (float)CRAFT_X + 10,
            (float)(topoLista + i * CRAFT_LINHA_ALTURA),
            (float)CRAFT_LARGURA - 20,
            (float)CRAFT_LINHA_ALTURA - 6
        };

        if (CheckCollisionPointRec((Vector2){ (float)mx, (float)my }, linha)) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                *selecionada = i;
                craftar = 1;
            }
        }
    }

    // Fundo do painel
    DrawRectangle(CRAFT_X, CRAFT_Y, CRAFT_LARGURA, CRAFT_ALTURA, Fade(BLACK, 0.85f));
    DrawRectangleLines(CRAFT_X, CRAFT_Y, CRAFT_LARGURA, CRAFT_ALTURA, LIGHTGRAY);
    DrawText("CRAFTING", CRAFT_X + 16, CRAFT_Y + 10, 22, WHITE);
    DrawText("Enter/clique: craftar", CRAFT_X + CRAFT_LARGURA - 172, CRAFT_Y + 16, 12, GRAY);

    // Lista de receitas
    for (int i = 0; i < total; i++) {
        Receita r = receitas[i];
        int pode = PodeCraftar(inv, r);

        Rectangle linha = {
            (float)CRAFT_X + 10,
            (float)(topoLista + i * CRAFT_LINHA_ALTURA),
            (float)CRAFT_LARGURA - 20,
            (float)CRAFT_LINHA_ALTURA - 6
        };
        int mouseAqui = CheckCollisionPointRec((Vector2){ (float)mx, (float)my }, linha);

        Color fundo = (i == *selecionada) ? (Color){ 70, 70, 70, 255 }
                    : (mouseAqui ? (Color){ 55, 55, 55, 255 }
                                 : (Color){ 35, 35, 35, 255 });
        DrawRectangleRec(linha, fundo);
        if (i == *selecionada) DrawRectangleLinesEx(linha, 1.5f, YELLOW);

        // Ícone do resultado (ferramentas usam a cor do tier)
        Texture2D tex = itens[r.resultado].textura;
        int ehFerr = itens[r.resultado].maxQuantidade == 1;
        Rectangle caixa = { linha.x + 8, linha.y + 8, 32, 32 };

        if (tex.id != 0) {
            Rectangle src = itens[r.resultado].recorte;
            float maior = src.width > src.height ? src.width : src.height;
            float escala = 32.0f / maior;
            float w = src.width * escala, h = src.height * escala;
            Rectangle dst = { caixa.x + (32 - w) / 2, caixa.y + (32 - h) / 2, w, h };
            DrawTexturePro(tex, src, dst, (Vector2){0, 0}, 0.0f,
                        ehFerr ? tierCor[r.tier] : WHITE);
        } else {
            DrawRectangleRec(caixa, ehFerr ? tierCor[r.tier] : itens[r.resultado].cor);
        }
        DrawRectangleLines((int)caixa.x, (int)caixa.y, 32, 32, BLACK);

        // Nome da receita
        const char *nome = r.nome ? r.nome : itens[r.resultado].nome;
        DrawText(nome, (int)linha.x + 50, (int)linha.y + 5, 18, pode ? WHITE : GRAY);

        // Ingredientes
        char textoIng[128] = "";
        int pos = 0;
        for (int j = 0; j < r.totalIng && pos < (int)sizeof(textoIng); j++) {
            pos += snprintf(textoIng + pos, sizeof(textoIng) - pos, "%s%dx %s",
                            j > 0 ? "   " : "",
                            r.ing[j].qtd,
                            itens[r.ing[j].tipo].nome);
        }
        DrawText(textoIng, (int)linha.x + 50, (int)linha.y + 27, 14,
                 pode ? LIGHTGRAY : (Color){ 190, 120, 120, 255 });

        // Status
        const char *status = pode ? "PRONTO" : "FALTAM";
        int larguraStatus = MeasureText(status, 14);
        DrawText(status, (int)(linha.x + linha.width - larguraStatus - 10),
                 (int)linha.y + 19, 14, pode ? GREEN : RED);
    }

    // Rodapé: dica ou mensagem de feedback
    if (mensagem && mensagem[0])
        DrawText(mensagem, CRAFT_X + 16, CRAFT_Y + CRAFT_ALTURA - 24, 14, corMensagem);
    else
        DrawText("Setas: escolher | C: fechar", CRAFT_X + 16,
                 CRAFT_Y + CRAFT_ALTURA - 24, 14, GRAY);

    return craftar;
}
