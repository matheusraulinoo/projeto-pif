#include "raylib.h"
#include <stdio.h>
#include "raymath.h"
#include "hud.h"
#include "telas.h"

static void DesenhaCoracao(int x, int y, int tam, Color cor)
{
    float r = tam / 4.0f;
    DrawCircleV((Vector2){ x + r,     y + r }, r, cor);   // lóbulo esquerdo
    DrawCircleV((Vector2){ x + 3 * r, y + r }, r, cor);   // lóbulo direito
    DrawTriangle((Vector2){ x + tam / 2.0f, y + tam },    // ponta de baixo
                 (Vector2){ x + tam,        y + r },      // canto direito
                 (Vector2){ x,              y + r },      // canto esquerdo
                 cor);
}

void DesenhaVida(float vida)
{
    int hp = (int)vida;
    if (vida > hp) hp++;   // qualquer fração ainda conta como 1 ponto

    for (int i = 0; i < CORACOES; i++)
    {
        int x = 10 + i * 30;
        int y = 350;

        DesenhaCoracao(x, y, 24, (Color){ 60, 20, 20, 255 });   // coração vazio (fundo)

        if (hp >= 2 * i + 2)
        {
            DesenhaCoracao(x, y, 24, RED);                      // inteiro
        }
        else if (hp == 2 * i + 1)
        {
            BeginScissorMode(x, y, 12, 24);                     // recorta só a metade esquerda
            DesenhaCoracao(x, y, 24, RED);
            EndScissorMode();
        }
    }
}

void DesenhaPainelLava(Inventario *inv, const char *mensagem, Color corMensagem, float tempoMensagem)
{
    int w = 360, h = 170;
    int x = (LARGURA_TELA - w) / 2, y = (ALTURA_TELA - h) / 2;

    DrawRectangle(x, y, w, h, Fade(BLACK, 0.85f));
    DrawRectangleLines(x, y, w, h, (Color){ 255, 140, 0, 255 });

    DesenhaTextoCentralizado("LAVA", y + 12, 22.0f, 2.0f, (Color){ 255, 140, 0, 255 });
    DesenhaTextoCentralizado(TextFormat("Jogar %d diamantes na lava?", LAVA_CUSTO),
                            y + 50, 16.0f, 1.5f, WHITE);

    int tem = Inv_Contar(inv, ITEM_DIAMANTE);
    DesenhaTextoCentralizado(TextFormat("Diamantes: %d / %d", tem, LAVA_CUSTO),
                            y + 78, 16.0f, 1.5f, tem >= LAVA_CUSTO ? GREEN : RED);

    DesenhaTextoCentralizado("ENTER - jogar    F - fechar", y + 110, 14.0f, 1.5f, LIGHTGRAY);

    if (tempoMensagem > 0.0f)
        DesenhaTextoCentralizado(mensagem, y + 138, 14.0f, 1.5f, corMensagem);
}

void DesenhaEscudoEquipado(Slot escudoEquipado)
{
    // Slot visual do escudo equipado
    int escudoX = 10;
    int escudoY = ALTURA_TELA - SLOT_TAM - 10;

    // Desenha o escudo equipado
    if (escudoEquipado.tipo == ITEM_ESCUDO) {
        DesenharIcone(
            escudoEquipado.tipo,
            escudoEquipado.tier,
            (Rectangle){
                escudoX + ICONE_MARGEM,
                escudoY + ICONE_MARGEM,
                ICONE_TAM,
                ICONE_TAM
            }
        );
        // Barra de durabilidade do escudo
        int durabilidadeMaxima = tierDurability[escudoEquipado.tier];

        if (durabilidadeMaxima > 0 &&
            escudoEquipado.durability < durabilidadeMaxima) {

            float porcentagem = (float) escudoEquipado.durability / durabilidadeMaxima;

            porcentagem = Clamp(porcentagem, 0.0f, 1.0f);

            Color corBarra = GREEN;

            if (porcentagem <= 0.25f) {
                corBarra = RED;
            } else if (porcentagem <= 0.5f) {
                corBarra = YELLOW;
            }

            int larguraBarra = SLOT_TAM - 10;
            int alturaBarra = 4;

            int xBarra = escudoX + 5;
            int yBarra = escudoY + SLOT_TAM - 7;

            DrawRectangle(xBarra, yBarra,
                        larguraBarra, alturaBarra, BLACK);

            DrawRectangle(xBarra, yBarra,
                        (int)(larguraBarra * porcentagem),
                        alturaBarra, corBarra);
        }
    }
}

void DesenhaInventarioAberto(Inventario *inv, int invX, int invY, int origemInv)
{
    DrawRectangle(invX - INV_PAD, invY - INV_PAD,
                INV_LARGURA + 2 * INV_PAD, INV_ALTURA + 2 * INV_PAD,
                Fade(BLACK, 0.6f));
    Inv_Draw(inv, invX, invY, INV_ROWS, origemInv);

    DrawText("Clique: mover  |  1-9: barra  |  direito: descartar (Shift = tudo)",
             invX, invY - 26, 14, LIGHTGRAY);

    // Destaque e dica do slot sob o mouse.
    int sobre = Inv_SlotNoPonto(invX, invY, GetMousePosition());
    if (sobre >= 0 && inv->slots[sobre].tipo != ITEM_VAZIO) {
        Rectangle slotSob = Inv_RetanguloSlot(invX, invY, sobre);
        DrawRectangleLinesEx(slotSob, 2.0f,
                             Fade((Color){ 220, 160, 80, 255 }, 0.9f));

        char dica[96];
        snprintf(dica, sizeof(dica), "%s - direito descarta 1%s",
                 itens[inv->slots[sobre].tipo].nome,
                 inv->slots[sobre].quant > 1
                     ? " | Shift descarta tudo" : "");

        Vector2 medida = MeasureTextEx(GetFontDefault(), dica, 12.0f, 0.5f);
        float tx = GetMouseX() + 16.0f;
        float ty = GetMouseY() + 16.0f;

        if (tx + medida.x + 10.0f > LARGURA_TELA)
            tx = GetMouseX() - medida.x - 16.0f;
        if (ty + 22.0f > ALTURA_TELA)
            ty = GetMouseY() - 28.0f;

        DrawRectangle((int)tx - 6, (int)ty - 4,
                      (int)medida.x + 12, 21, Fade(BLACK, 0.85f));
        DrawRectangleLines((int)tx - 6, (int)ty - 4,
                           (int)medida.x + 12, 21,
                           Fade((Color){ 120, 110, 95, 255 }, 0.8f));
        DrawTextEx(GetFontDefault(), dica, (Vector2){ tx, ty },
                   12.0f, 0.5f, RAYWHITE);
    }
}

void DesenhaBarraVidaRecurso(Recursos *a)
{
    // Barra de vida do recurso (apenas quando danificado)
    if (!a->quebrado && a->vida < a->vida_maxima) {

        float larguraBarra = 32.0f;
        float alturaBarra = 5.0f;

        float porcentagemVida = (float)a->vida / a->vida_maxima;
        porcentagemVida = Clamp(porcentagemVida, 0.0f, 1.0f);

        float xBarra = a->posicao.x - larguraBarra / 2.0f;
        float yBarra = a->posicao.y - 30.0f;

        // Fundo cinza
        DrawRectangle(
            (int)xBarra,
            (int)yBarra,
            (int)larguraBarra,
            (int)alturaBarra,
            GRAY
        );

        // Preenchimento verde
        DrawRectangle(
            (int)xBarra,
            (int)yBarra,
            (int)(larguraBarra * porcentagemVida),
            (int)alturaBarra,
            GREEN
        );
    }
}
