#include "raylib.h"
#include <stdio.h>
#include "inventory.h"
#include "crafting.h"

Receita receitasCraft[] = {
    {
        .nome = "Graveto",
        .resultado = ITEM_GRAVETO, .qtdResultado = 1,
        .ing = { { ITEM_MADEIRA, 2 } }, .totalIng = 1
    },
    {
        .nome = "Espada de Madeira",
        .resultado = ITEM_ESPADA, .qtdResultado = 1, .tier = TIER_MADEIRA,
        .ing = { { ITEM_MADEIRA, 2 }, { ITEM_GRAVETO, 1 } }, .totalIng = 2
    },
    {
        .nome = "Espada de Pedra",
        .resultado = ITEM_ESPADA, .qtdResultado = 1, .tier = TIER_PEDRA,
        .ing = { { ITEM_PEDRA, 2 }, { ITEM_GRAVETO, 1 } }, .totalIng = 2
    },
    {
        .nome = "Espada de Ferro",
        .resultado = ITEM_ESPADA, .qtdResultado = 1, .tier = TIER_FERRO,
        .ing = { { ITEM_FERRO, 2 }, { ITEM_GRAVETO, 1 } }, .totalIng = 2
    },
    {
        .nome = "Espada de Ouro",
        .resultado = ITEM_ESPADA, .qtdResultado = 1, .tier = TIER_OURO,
        .ing = { { ITEM_OURO, 2 }, { ITEM_GRAVETO, 1 } }, .totalIng = 2
    },
    {
        .nome = "Espada de Diamante",
        .resultado = ITEM_ESPADA, .qtdResultado = 1, .tier = TIER_DIAMANTE,
        .ing = { { ITEM_DIAMANTE, 2 }, { ITEM_GRAVETO, 1 } }, .totalIng = 2
    },
    {
        .nome = "Espada de Magnetita",
        .resultado = ITEM_ESPADA, .qtdResultado = 1, .tier = TIER_MAGNETITA,
        .ing = { { ITEM_MAGNETITA, 2 }, { ITEM_GRAVETO, 1 } }, .totalIng = 2
    },
    {
        .nome = "Picareta de Madeira",
        .resultado = ITEM_PICARETA, .qtdResultado = 1, .tier = TIER_MADEIRA,
        .ing = { { ITEM_MADEIRA, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Picareta de Pedra",
        .resultado = ITEM_PICARETA, .qtdResultado = 1, .tier = TIER_PEDRA,
        .ing = { { ITEM_PEDRA, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Picareta de Ferro",
        .resultado = ITEM_PICARETA, .qtdResultado = 1, .tier = TIER_FERRO,
        .ing = { { ITEM_FERRO, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Picareta de Ouro",
        .resultado = ITEM_PICARETA, .qtdResultado = 1, .tier = TIER_OURO,
        .ing = { { ITEM_OURO, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Picareta de Diamante",
        .resultado = ITEM_PICARETA, .qtdResultado = 1, .tier = TIER_DIAMANTE,
        .ing = { { ITEM_DIAMANTE, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Picareta de Magnetita",
        .resultado = ITEM_PICARETA, .qtdResultado = 1, .tier = TIER_MAGNETITA,
        .ing = { { ITEM_MAGNETITA, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Machado de Madeira",
        .resultado = ITEM_MACHADO, .qtdResultado = 1, .tier = TIER_MADEIRA,
        .ing = { { ITEM_MADEIRA, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Machado de Pedra",
        .resultado = ITEM_MACHADO, .qtdResultado = 1, .tier = TIER_PEDRA,
        .ing = { { ITEM_PEDRA, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Machado de Ferro",
        .resultado = ITEM_MACHADO, .qtdResultado = 1, .tier = TIER_FERRO,
        .ing = { { ITEM_FERRO, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Machado de Ouro",
        .resultado = ITEM_MACHADO, .qtdResultado = 1, .tier = TIER_OURO,
        .ing = { { ITEM_OURO, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Machado de Diamante",
        .resultado = ITEM_MACHADO, .qtdResultado = 1, .tier = TIER_DIAMANTE,
        .ing = { { ITEM_DIAMANTE, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Machado de Magnetita",
        .resultado = ITEM_MACHADO, .qtdResultado = 1, .tier = TIER_MAGNETITA,
        .ing = { { ITEM_MAGNETITA, 3 }, { ITEM_GRAVETO, 2 } }, .totalIng = 2
    },
    {
        .nome = "Escudo de Madeira",
        .resultado = ITEM_ESCUDO, .qtdResultado = 1, .tier = TIER_MADEIRA,
        .ing = { { ITEM_MADEIRA, 4 } }, .totalIng = 1
    },
    {
        .nome = "Escudo de Pedra",
        .resultado = ITEM_ESCUDO, .qtdResultado = 1, .tier = TIER_PEDRA,
        .ing = { { ITEM_PEDRA, 4 } }, .totalIng = 1
    },
    {
        .nome = "Escudo de Ferro",
        .resultado = ITEM_ESCUDO, .qtdResultado = 1, .tier = TIER_FERRO,
        .ing = { { ITEM_FERRO, 4 } }, .totalIng = 1
    },
    {
        .nome = "Escudo de Ouro",
        .resultado = ITEM_ESCUDO, .qtdResultado = 1, .tier = TIER_OURO,
        .ing = { { ITEM_OURO, 4 } }, .totalIng = 1
    },
    {
        .nome = "Escudo de Diamante",
        .resultado = ITEM_ESCUDO, .qtdResultado = 1, .tier = TIER_DIAMANTE,
        .ing = { { ITEM_DIAMANTE, 4 } }, .totalIng = 1
    },
    {
        .nome = "Escudo de Magnetita",
        .resultado = ITEM_ESCUDO, .qtdResultado = 1, .tier = TIER_MAGNETITA,
        .ing = { { ITEM_MAGNETITA, 4 } }, .totalIng = 1
    },
};

// Quantidade de receitas da lista acima
int TotalReceitas(void) {
    return (int)(sizeof(receitasCraft) / sizeof(receitasCraft[0]));
}

static int TemSlotVazio(Inventario *inv) {
    for (int i = 0; i < INV_SIZE; i++)
        if (inv->slots[i].tipo == ITEM_VAZIO) return 1;
    return 0;
}
//verifica se pode craftar, se tem slots vazios para realizar o craft
int PodeCraftar(Inventario *inv, Receita r) {
    for (int i = 0; i < r.totalIng; i++)
        if (Inv_Contar(inv, r.ing[i].tipo) < r.ing[i].qtd) return 0;

    if (itens[r.resultado].maxQuantidade == 1 && !TemSlotVazio(inv)) return 0;

    return 1;
}
//Realiza o crafting das ferramentas, além de verificar se o inventário tá cheio ou não, caso nn esteja adiciona a ferramenta no inventário.
int Craftar(Inventario *inv, Receita r) {
    for (int i = 0; i < r.totalIng; i++) {
        if (Inv_Contar(inv, r.ing[i].tipo) < r.ing[i].qtd) {
            printf("Falta: %s\n", itens[r.ing[i].tipo].nome);
            return 0;
        }
    }

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
#define CRAFT_LINHA_ALTURA 50
#define CRAFT_VISIVEIS     4
#define CRAFT_TOPO_LISTA   (CRAFT_Y + 44)

int DesenharCraftingUI(Receita *receitas, int total, Inventario *inv,
                       int *selecionada, const char *mensagem, Color corMensagem)
{
    if (total <= 0) return 0;

    if (*selecionada < 0) *selecionada = 0;
    if (*selecionada >= total) *selecionada = total - 1;

    // Primeira receita visível na lista (rolagem)
    static int rolagem = 0;

    int mx = GetMouseX();
    int my = GetMouseY();

    // Navegação pelo teclado
    if (IsKeyPressed(KEY_DOWN)) *selecionada = (*selecionada + 1) % total;
    if (IsKeyPressed(KEY_UP))   *selecionada = (*selecionada + total - 1) % total;

    // Roda do mouse percorre a lista de receitas
    Rectangle painel = { CRAFT_X, CRAFT_Y, CRAFT_LARGURA, CRAFT_ALTURA };
    float roda = GetMouseWheelMove();
    if (roda != 0.0f && CheckCollisionPointRec((Vector2){ (float)mx, (float)my }, painel)) {
        int passos = (int)roda;
        *selecionada = (*selecionada - passos) % total;
        if (*selecionada < 0) *selecionada += total;
    }

    // Mantém a receita selecionada dentro da área visível
    if (*selecionada < rolagem) rolagem = *selecionada;
    if (*selecionada >= rolagem + CRAFT_VISIVEIS)
        rolagem = *selecionada - CRAFT_VISIVEIS + 1;

    int maxRolagem = total - CRAFT_VISIVEIS;
    if (maxRolagem < 0) maxRolagem = 0;
    if (rolagem > maxRolagem) rolagem = maxRolagem;
    if (rolagem < 0) rolagem = 0;

    int fim = rolagem + CRAFT_VISIVEIS;
    if (fim > total) fim = total;

    int craftar = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE);

    // Navegação/clique pelo mouse
    for (int i = rolagem; i < fim; i++) {
        Rectangle linha = {
            (float)CRAFT_X + 10,
            (float)(CRAFT_TOPO_LISTA + (i - rolagem) * CRAFT_LINHA_ALTURA),
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

    // Lista de receitas (apenas as visíveis)
    for (int i = rolagem; i < fim; i++) {
        Receita r = receitas[i];
        int pode = PodeCraftar(inv, r);

        Rectangle linha = {
            (float)CRAFT_X + 10,
            (float)(CRAFT_TOPO_LISTA + (i - rolagem) * CRAFT_LINHA_ALTURA),
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
        Rectangle caixa = { linha.x + 8, linha.y + 8, 32, 32 };
        DesenharIcone(r.resultado, r.tier, caixa);
        DrawRectangleLines((int)caixa.x, (int)caixa.y, 32, 32, BLACK);

        // Nome da receita
        const char *nome = r.nome ? r.nome : itens[r.resultado].nome;
        DrawText(nome, (int)linha.x + 50, (int)linha.y + 4, 18, pode ? WHITE : GRAY);

        // Ingredientes
        char textoIng[128] = "";
        int pos = 0;
        for (int j = 0; j < r.totalIng && pos < (int)sizeof(textoIng); j++) {
            pos += snprintf(textoIng + pos, sizeof(textoIng) - pos, "%s%dx %s",
                            j > 0 ? "   " : "",
                            r.ing[j].qtd,
                            itens[r.ing[j].tipo].nome);
        }
        DrawText(textoIng, (int)linha.x + 50, (int)linha.y + 26, 14,
                 pode ? LIGHTGRAY : (Color){ 190, 120, 120, 255 });

        // Status
        const char *status = pode ? "PRONTO" : "FALTAM";
        int larguraStatus = MeasureText(status, 14);
        DrawText(status, (int)(linha.x + linha.width - larguraStatus - 10),
                 (int)linha.y + 17, 14, pode ? GREEN : RED);
    }

    // Indicadores de rolagem
    if (rolagem > 0)
        DrawRectangle(CRAFT_X + CRAFT_LARGURA - 8, CRAFT_TOPO_LISTA, 3, 10, GRAY);
    if (fim < total)
        DrawRectangle(CRAFT_X + CRAFT_LARGURA - 8,
                      CRAFT_TOPO_LISTA + CRAFT_VISIVEIS * CRAFT_LINHA_ALTURA - 16,
                      3, 10, GRAY);

    // Rodapé: dica ou mensagem de feedback
    if (mensagem && mensagem[0])
        DrawText(mensagem, CRAFT_X + 16, CRAFT_Y + CRAFT_ALTURA - 24, 14, corMensagem);
    else
        DrawText("Setas/roda: escolher | C: fechar", CRAFT_X + 16,
                 CRAFT_Y + CRAFT_ALTURA - 24, 14, GRAY);

    // Posição da receita selecionada na lista
    DrawText(TextFormat("%d/%d", *selecionada + 1, total),
             CRAFT_X + CRAFT_LARGURA - 58, CRAFT_Y + CRAFT_ALTURA - 24, 14, GRAY);

    return craftar;
}
