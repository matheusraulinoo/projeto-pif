#include "raylib.h"
#include <stdio.h>
#include "math.h"
#include "telas.h"

// ============================================================
//  TELAS DE INÍCIO E MORTE
// ============================================================

typedef struct {
    float x, y;
    float brilho;
    float fase;
    float tamanho;
} Estrela;

static Estrela estrelas[NUM_ESTRELAS];
static unsigned int sementeVisual = 20261110u;

// Ruído determinístico para as estrelas não mudarem de lugar a cada frame.
static float RuidoVisual(void)
{
    sementeVisual = sementeVisual * 1664525u + 1013904223u;
    return (float)((sementeVisual >> 8) & 0xFFFFFF) / 16777215.0f;
}

void IniciarEstrelas(void)
{
    for (int i = 0; i < NUM_ESTRELAS; i++)
    {
        estrelas[i].x       = RuidoVisual() * LARGURA_TELA;
        estrelas[i].y       = RuidoVisual() * (ALTURA_TELA * 0.6f);
        estrelas[i].brilho  = 0.35f + RuidoVisual() * 0.65f;
        estrelas[i].fase    = RuidoVisual() * 2.0f * PI;
        estrelas[i].tamanho = 0.8f + RuidoVisual() * 1.4f;
    }
}

static float Limitar01(float valor)
{
    if (valor < 0.0f) return 0.0f;
    if (valor > 1.0f) return 1.0f;
    return valor;
}

void DesenhaTextoCentralizado(const char *texto, float y, float tamanho,
                              float espacamento, Color cor)
{
    Vector2 medida = MeasureTextEx(GetFontDefault(), texto, tamanho, espacamento);
    DrawTextEx(GetFontDefault(), texto,
               (Vector2){ (LARGURA_TELA - medida.x) / 2.0f, y },
               tamanho, espacamento, cor);
}

// Título com brilho suave em volta (usado no menu e na tela de morte).
static void DesenhaTituloComBrilho(const char *texto, float x, float y,
                                   float tamanho, float espacamento,
                                   Color cor, float brilho, float alpha)
{
    for (int camada = 2; camada >= 1; camada--)
    {
        for (int angulo = 0; angulo < 360; angulo += 45)
        {
            float rad = angulo * DEG2RAD;
            Vector2 pos = { x + cosf(rad) * (3.5f * camada),
                            y + sinf(rad) * (3.5f * camada) };
            DrawTextEx(GetFontDefault(), texto, pos, tamanho, espacamento,
                       Fade(cor, alpha * (0.13f * brilho) / camada));
        }
    }

    DrawTextEx(GetFontDefault(), texto, (Vector2){ x + 3.0f, y + 4.0f },
               tamanho, espacamento, Fade(BLACK, 0.8f * alpha));
    DrawTextEx(GetFontDefault(), texto, (Vector2){ x, y },
               tamanho, espacamento, Fade(cor, alpha));
}

static void DesenhaVinheta(Color cor, float forca)
{
    for (int i = 0; i < 6; i++)
    {
        float margem = i * 20.0f;
        float alpha  = forca * (0.12f - i * 0.016f);
        if (alpha <= 0.0f) continue;

        Color c = Fade(cor, alpha);
        DrawRectangle(0, 0, LARGURA_TELA, (int)margem, c);
        DrawRectangle(0, ALTURA_TELA - (int)margem, LARGURA_TELA, (int)margem, c);
        DrawRectangle(0, 0, (int)margem, ALTURA_TELA, c);
        DrawRectangle(LARGURA_TELA - (int)margem, 0, (int)margem, ALTURA_TELA, c);
    }
}


static void DesenhaTextoComSombra(const char *texto, float x, float y,
                                  float tamanho, float espacamento, Color cor)
{
    DrawTextEx(GetFontDefault(), texto,
               (Vector2){ x + 1.0f, y + 1.0f }, tamanho, espacamento,
               Fade(BLACK, 0.65f));
    DrawTextEx(GetFontDefault(), texto,
               (Vector2){ x, y }, tamanho, espacamento, cor);
}

// Desenha letra por letra, com pequenas imperfeições, para o título não
// parecer texto de fonte aplicado de forma perfeita.
static void DesenhaPalavraTorta(const char *texto, float x, float y,
                                float tamanho, float espacamento,
                                Color cor, Color sombra, float semente)
{
    float cursor = x;
    int i = 0;

    for (const char *c = texto; *c != '\0'; c++, i++)
    {
        if (*c == ' ')
        {
            cursor += tamanho * 0.34f + espacamento;
            continue;
        }

        char letra[2] = { *c, '\0' };
        Vector2 medida = MeasureTextEx(GetFontDefault(), letra, tamanho, 0.0f);
        float tremorY = sinf(i * 1.7f + semente) * (tamanho * 0.035f);
        float tremorX = cosf(i * 2.3f + semente) * (tamanho * 0.012f);

        DrawTextEx(GetFontDefault(), letra,
                   (Vector2){ cursor + tremorX + 2.0f, y + tremorY + 2.5f },
                   tamanho, 0.0f, sombra);
        DrawTextEx(GetFontDefault(), letra,
                   (Vector2){ cursor + tremorX, y + tremorY },
                   tamanho, 0.0f, cor);

        cursor += medida.x + espacamento;
    }
}

static void DesenhaPinheiro(float x, float baseY, float altura, Color cor)
{
    float largura = altura * 0.52f;

    // Tronco
    DrawRectangle((int)(x - altura * 0.05f), (int)(baseY - altura * 0.16f),
                  (int)(altura * 0.10f) + 1, (int)(altura * 0.18f) + 1, cor);

    // Três camadas de folhagem
    for (int i = 0; i < 3; i++)
    {
        float camadaBase = baseY - altura * (0.12f + i * 0.24f);
        float camadaTopo = baseY - altura * (0.46f + i * 0.24f);
        float meiaLargura = largura * (0.5f - i * 0.13f);

        DrawTriangle((Vector2){ x - meiaLargura, camadaBase },
                     (Vector2){ x + meiaLargura, camadaBase },
                     (Vector2){ x, camadaTopo },
                     cor);
    }
}

static void DesenhaCeuEstrelado(float tempo)
{
    DrawRectangleGradientV(0, 0, LARGURA_TELA, 262,
                           (Color){ 6, 8, 16, 255 },
                           (Color){ 37, 24, 32, 255 });

    for (int i = 0; i < NUM_ESTRELAS; i++)
    {
        float cintila = 0.5f + 0.5f * sinf(tempo * 1.3f + estrelas[i].fase);
        DrawCircleV((Vector2){ estrelas[i].x, estrelas[i].y },
                    estrelas[i].tamanho * 0.75f,
                    Fade((Color){ 214, 218, 232, 255 },
                         estrelas[i].brilho * (0.35f + 0.65f * cintila)));
    }
}

static void DesenhaLua(float tempo)
{
    Vector2 lua = { 646.0f, 84.0f };
    float halo = 0.9f + 0.1f * sinf(tempo * 1.1f);

    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 4; i >= 1; i--)
        DrawCircleV(lua, 28.0f + i * 9.0f,
                    Fade((Color){ 150, 165, 200, 255 }, 0.014f * halo));
    EndBlendMode();

    DrawCircleV(lua, 27.0f, (Color){ 224, 217, 196, 255 });
    // Morde a lua para virar crescente.
    DrawCircleV((Vector2){ lua.x + 11.0f, lua.y - 5.0f }, 25.0f,
                (Color){ 12, 13, 21, 255 });
    DrawCircleLines((int)lua.x, (int)lua.y, 27.0f,
                    Fade((Color){ 245, 238, 215, 255 }, 0.18f));
}

static void DesenhaNuvens(float tempo)
{
    Color nuvem = (Color){ 30, 24, 34, 255 };

    for (int i = 0; i < 3; i++)
    {
        float cx = fmodf(tempo * (3.0f + i * 1.5f) + i * 340.0f,
                         LARGURA_TELA + 260.0f) - 130.0f;
        float cy = 132.0f + i * 30.0f;

        DrawEllipse((int)cx, (int)cy, 80.0f, 7.0f, Fade(nuvem, 0.20f));
        DrawEllipse((int)(cx + 38.0f), (int)(cy + 3.0f), 48.0f, 5.0f,
                    Fade(nuvem, 0.14f));
        DrawEllipse((int)(cx - 42.0f), (int)(cy - 2.0f), 44.0f, 5.0f,
                    Fade(nuvem, 0.12f));
    }
}

static void DesenhaHorizonte(void)
{
    Color longe = (Color){ 12, 14, 23, 255 };

    DrawEllipse(130, 256, 240, 16, longe);
    DrawEllipse(480, 258, 320, 14, longe);

    for (int i = 0; i < 26; i++)
    {
        float x = 8.0f + i * 31.0f;
        float altura = 20.0f + (float)((i * 17) % 26);
        DesenhaPinheiro(x, 254.0f, altura, longe);
    }
}

static void DesenhaMar(float tempo)
{
    DrawRectangleGradientV(0, 252, LARGURA_TELA, 86,
                           (Color){ 22, 29, 46, 255 },
                           (Color){ 12, 16, 28, 255 });

    // Reflexo quebrado da lua na água.
    for (int i = 0; i < 15; i++)
    {
        float y = 258.0f + i * 5.2f;
        float larg = 16.0f + (float)((i * 37) % 34) +
                     sinf(tempo * 1.1f + i * 0.8f) * 5.0f;
        float x = 646.0f - larg * 0.5f + sinf(tempo * 0.7f + i * 0.9f) * 5.0f;
        float alpha = (0.12f + 0.05f * sinf(tempo * 1.7f + i)) *
                      (1.0f - i * 0.045f);

        DrawRectangle((int)x, (int)y, (int)larg, 1,
                      Fade((Color){ 196, 204, 208, 255 }, alpha));
    }

    // Ondulações soltas na superfície.
    for (int i = 0; i < 9; i++)
    {
        float y = 266.0f + i * 7.5f;
        float x = fmodf(tempo * (7.0f + i) + i * 97.0f,
                        LARGURA_TELA + 120.0f) - 60.0f;
        DrawRectangle((int)x, (int)y, 28 + i * 3, 1,
                      Fade((Color){ 90, 110, 140, 255 }, 0.10f));
    }
}

static void DesenhaPraia(void)
{
    for (int x = 0; x < LARGURA_TELA; x += 8)
    {
        float borda = 334.0f + sinf(x * 0.010f) * 5.0f +
                      sinf(x * 0.033f + 1.2f) * 2.5f;
        int y = (int)borda;

        DrawRectangle(x, y, 8, ALTURA_TELA - y, (Color){ 28, 24, 25, 255 });
        DrawRectangle(x, y, 8, 1, Fade((Color){ 78, 68, 60, 255 }, 0.7f));

        if (((x / 8) % 3) == 0)
            DrawRectangle(x, y + 1, 8, 1,
                          Fade((Color){ 60, 52, 47, 255 }, 0.5f));
    }

    DrawRectangleGradientV(0, 356, LARGURA_TELA, ALTURA_TELA - 356,
                           BLANK, (Color){ 9, 8, 12, 210 });

    // Grãos de areia, para a faixa inferior não ficar chapada.
    for (int i = 0; i < 70; i++)
    {
        int px = (i * 137 + 29) % LARGURA_TELA;
        int py = 352 + (i * 61) % 92;
        float a = 0.08f + (float)(i % 5) * 0.025f;

        DrawPixel(px, py, Fade((Color){ 92, 78, 66, 255 }, a));
    }
}

static void DesenhaMataDaFrente(void)
{
    Color escuro = (Color){ 6, 7, 12, 255 };

    DesenhaPinheiro(752.0f, 452.0f, 236.0f, escuro);
    DesenhaPinheiro(700.0f, 448.0f, 150.0f, escuro);
    DesenhaPinheiro(790.0f, 460.0f, 130.0f, escuro);

    DrawEllipse(26, 448, 90, 34, (Color){ 7, 8, 13, 255 });
    DrawEllipse(770, 442, 70, 26, (Color){ 7, 8, 13, 255 });

    for (int i = 0; i < 16; i++)
    {
        float x = 12.0f + i * 51.0f;
        float h = 4.0f + (float)((i * 7) % 6);
        DrawLine((int)x, ALTURA_TELA, (int)x + 1, ALTURA_TELA - (int)h,
                 Fade((Color){ 40, 36, 34, 255 }, 0.8f));
    }
}

static void DesenhaCostaDaCabana(void)
{
    Color terra = (Color){ 15, 15, 20, 255 };
    Color barranco = (Color){ 31, 26, 25, 255 };
    Color mato = (Color){ 9, 10, 15, 255 };

    // Promontório que dá chão à cabana e liga a cena à borda esquerda.
    DrawRectangle(0, 249, 300, 11, terra);
    DrawEllipse(150, 255, 210, 17, terra);
    DrawEllipse(60, 257, 150, 14, terra);
    DrawEllipse(260, 256, 92, 14, terra);

    // Barranco irregular, para não parecer uma elipse perfeita.
    for (int i = 0; i < 32; i++)
    {
        float x = 6.0f + i * 10.0f;
        float alt = 3.0f + (float)((i * 11) % 6);
        DrawRectangle((int)x, 260, 8, (int)alt, barranco);
    }

    // Mato quebrando a silhueta do topo.
    for (int i = 0; i < 26; i++)
    {
        float x = 10.0f + i * 13.0f;
        float alt = 3.0f + (float)((i * 7) % 5);
        DrawLine((int)x, 250, (int)x + 1, 250 - (int)alt, mato);
    }

    DesenhaPinheiro(120.0f, 258.0f, 36.0f, mato);
    DesenhaPinheiro(205.0f, 256.0f, 44.0f, mato);
    DesenhaPinheiro(348.0f, 254.0f, 50.0f, mato);
}

static void DesenhaCabana(float tempo)
{
    float base = 256.0f;
    Color parede = (Color){ 15, 13, 17, 255 };
    Color telhado = (Color){ 24, 16, 16, 255 };

    DrawEllipse(279, (int)base + 2, 92, 15, (Color){ 11, 12, 19, 255 });

    // Chaminé e fumaça.
    DrawRectangle(310, 190, 8, 22, parede);
    for (int i = 0; i < 4; i++)
    {
        float t = fmodf(tempo * 0.22f + i * 0.27f, 1.0f);
        DrawEllipse((int)(314.0f + t * 26.0f), (int)(188.0f - t * 46.0f),
                    7.0f + t * 12.0f, 5.0f + t * 7.0f,
                    Fade((Color){ 70, 68, 74, 255 }, (1.0f - t) * 0.16f));
    }

    // Corpo, telhado e porta.
    DrawRectangle(244, 216, 70, 40, parede);
    DrawTriangle((Vector2){ 232, 218 }, (Vector2){ 326, 218 },
                 (Vector2){ 279, 184 }, telhado);
    DrawTriangle((Vector2){ 232, 218 }, (Vector2){ 279, 184 },
                 (Vector2){ 279, 218 }, (Color){ 18, 13, 14, 255 });
    DrawRectangle(271, 236, 14, 20, (Color){ 9, 8, 11, 255 });

    // Janela acesa: o único ponto quente antes da fogueira.
    float luz = 0.75f + 0.25f * sinf(tempo * 2.1f) * sinf(tempo * 0.7f + 1.0f);
    DrawRectangle(252, 226, 11, 9, Fade((Color){ 255, 186, 92, 255 }, luz));
    DrawRectangle(253, 227, 4, 7, Fade((Color){ 255, 224, 160, 255 }, luz));

    BeginBlendMode(BLEND_ADDITIVE);
    DrawCircleV((Vector2){ 257.5f, 230.5f }, 20.0f,
                Fade((Color){ 255, 160, 70, 255 }, 0.035f * luz));
    DrawCircleV((Vector2){ 257.5f, 230.5f }, 40.0f,
                Fade((Color){ 255, 150, 60, 255 }, 0.018f * luz));
    EndBlendMode();
}

static void DesenhaVagaLumes(float tempo)
{
    for (int i = 0; i < 7; i++)
    {
        float fase = i * 1.9f;
        float x = 210.0f + (float)((i * 83) % 420) +
                  sinf(tempo * 0.5f + fase) * 18.0f;
        float y = 250.0f + (float)((i * 47) % 80) +
                  cosf(tempo * 0.4f + fase) * 8.0f;
        float brilho = 0.4f + 0.6f * (0.5f + 0.5f * sinf(tempo * 2.0f + fase));

        DrawCircleV((Vector2){ x, y }, 1.3f,
                    Fade((Color){ 255, 205, 110, 255 }, brilho * 0.8f));
        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleV((Vector2){ x, y }, 4.0f,
                    Fade((Color){ 255, 180, 70, 255 }, brilho * 0.10f));
        EndBlendMode();
    }
}

static void DesenhaPersonagemInicio(Texture2D tex, float tempo)
{
    if (tex.id == 0) return;

    float escala = 2.6f;
    float lado   = SPRITE_TAM * escala;
    float x      = 398.0f;
    float baseY  = 405.0f;
    float respira = sinf(tempo * 1.6f) * 0.9f;

    // Sombra puxada para o lado contrário da fogueira.
    DrawEllipse((int)(x + lado / 2.0f - 20.0f), (int)(baseY - 2.0f),
                lado * 0.46f, 7.0f, Fade(BLACK, 0.55f));

    // Linha 2 = olhando para a direita, na direção da fogueira.
    Rectangle origem = { 0.0f, 2.0f * SPRITE_TAM, SPRITE_TAM, SPRITE_TAM };
    Rectangle destino = { x, baseY - lado + respira, lado, lado };

    DrawTexturePro(tex, origem, destino, (Vector2){ 0, 0 }, 0.0f,
                   (Color){ 232, 220, 205, 255 });

    // Empurra um pouco de calor da fogueira para o sprite.
    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(tex, origem, destino, (Vector2){ 0, 0 }, 0.0f,
                   (Color){ 34, 14, 4, 255 });
    EndBlendMode();
}

static void DesenhaFogueira(float tempo)
{
    float x = 514.0f;
    float y = 396.0f;
    float pisca = 0.82f + 0.18f * (0.5f + 0.5f * sinf(tempo * 11.3f));

    // Luz somada no chão. Depois ela passa por cima do personagem,
    // o que ajuda a integrá-lo à cena.
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 10; i >= 1; i--)
        DrawCircleV((Vector2){ x, y - 2.0f }, 10.0f + i * 13.0f,
                    Fade((Color){ 255, 126, 44, 255 }, 0.024f * pisca));
    EndBlendMode();

    // Poça de luz na areia.
    DrawEllipse((int)x, (int)y + 8, 76, 18,
                Fade((Color){ 118, 58, 24, 255 }, 0.20f * pisca));

    // Pedras em volta da fogueira.
    for (int i = 0; i < 8; i++)
    {
        float ang = i * (2.0f * PI / 8.0f);
        int px = (int)(x + cosf(ang) * 27.0f);
        int py = (int)(y + 6.0f + sinf(ang) * 8.0f);

        DrawCircleV((Vector2){ px, py }, 3.0f + (i % 3),
                    (Color){ 38, 34, 32, 255 });
        DrawCircleV((Vector2){ px - 1.0f, py - 1.0f }, 1.2f,
                    Fade((Color){ 82, 74, 66, 255 }, 0.35f));
    }

    // Toras cruzadas.
    DrawRectanglePro((Rectangle){ x - 22.0f, y - 2.0f, 44.0f, 8.0f },
                     (Vector2){ 22.0f, 4.0f }, 14.0f,
                     (Color){ 43, 28, 21, 255 });
    DrawRectanglePro((Rectangle){ x - 22.0f, y, 44.0f, 8.0f },
                     (Vector2){ 22.0f, 4.0f }, -12.0f,
                     (Color){ 34, 22, 17, 255 });

    float baseChama = y - 6.0f;
    float b1 = sinf(tempo * 9.1f) * 2.5f + sinf(tempo * 15.3f + 1.7f) * 1.2f;
    float b2 = sinf(tempo * 7.7f + 2.0f) * 3.0f;

    // Línguas assimétricas, em vez de um cone perfeito.
    DrawTriangle((Vector2){ x - 12.0f, baseChama },
                 (Vector2){ x + 11.0f, baseChama },
                 (Vector2){ x - 4.0f + b1, baseChama - 29.0f - b2 },
                 (Color){ 176, 56, 18, 255 });
    DrawTriangle((Vector2){ x - 10.0f, baseChama },
                 (Vector2){ x + 12.0f, baseChama },
                 (Vector2){ x + 3.0f + b2, baseChama - 43.0f + b1 },
                 (Color){ 214, 80, 24, 255 });
    DrawTriangle((Vector2){ x - 7.0f, baseChama },
                 (Vector2){ x + 8.0f, baseChama },
                 (Vector2){ x + 7.0f + b1 * 0.5f,
                            baseChama - 34.0f - b2 * 0.6f },
                 (Color){ 232, 116, 32, 255 });
    DrawTriangle((Vector2){ x - 5.0f, baseChama },
                 (Vector2){ x + 5.0f, baseChama },
                 (Vector2){ x + b2 * 0.4f, baseChama - 22.0f - b1 * 0.5f },
                 (Color){ 255, 184, 66, 255 });
    DrawTriangle((Vector2){ x - 2.5f, baseChama },
                 (Vector2){ x + 2.5f, baseChama },
                 (Vector2){ x + b1 * 0.3f, baseChama - 12.0f },
                 (Color){ 255, 238, 170, 255 });

    // Base arredondada, para a chama não terminar em bico seco.
    DrawCircleV((Vector2){ x, baseChama - 1.0f }, 9.0f,
                (Color){ 196, 70, 22, 255 });
    DrawCircleV((Vector2){ x, baseChama - 1.0f }, 5.0f,
                (Color){ 244, 140, 44, 255 });

    // Brasas: cada uma com seu próprio ritmo, subindo torto.
    for (int i = 0; i < 12; i++)
    {
        float ciclo = fmodf(tempo * (16.0f + i * 1.7f) + i * 23.0f, 76.0f);
        float ex = x + sinf(tempo * 1.4f + i * 2.3f) * (3.0f + ciclo * 0.12f);
        float ey = baseChama - 10.0f - ciclo;

        DrawRectangle((int)ex, (int)ey, 1, 2,
                      Fade((Color){ 255, 190, 92, 255 },
                           0.85f * (1.0f - ciclo / 76.0f)));
    }

    for (int i = 0; i < 3; i++)
    {
        float t = fmodf(tempo * 0.16f + i * 0.33f, 1.0f);
        DrawEllipse((int)(x + 6.0f + t * 38.0f),
                    (int)(baseChama - 44.0f - t * 82.0f),
                    8.0f + t * 16.0f, 5.0f + t * 9.0f,
                    Fade((Color){ 74, 72, 80, 255 }, (1.0f - t) * 0.10f));
    }
}

static void DesenhaOlhosNaMata(float tempo)
{
    struct { float x, y, fase; } olhos[] = {
        { 214.0f, 228.0f, 0.0f },
        { 348.0f, 242.0f, 2.1f },
        { 738.0f, 306.0f, 4.0f },
    };

    for (int i = 0; i < 3; i++)
    {
        float ciclo = fmodf(tempo + olhos[i].fase, 5.3f);
        if (ciclo > 5.05f) continue;   // piscada rápida

        float brilho = 0.7f + 0.3f * (0.5f + 0.5f * sinf(tempo * 2.6f + olhos[i].fase));
        Color cor = Fade((Color){ 236, 168, 54, 255 }, brilho);

        DrawCircleV((Vector2){ olhos[i].x, olhos[i].y }, 1.8f, cor);
        DrawCircleV((Vector2){ olhos[i].x + 6.0f, olhos[i].y - 1.0f }, 1.8f, cor);

        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleV((Vector2){ olhos[i].x, olhos[i].y }, 5.0f,
                    Fade((Color){ 255, 140, 40, 255 }, 0.07f));
        DrawCircleV((Vector2){ olhos[i].x + 6.0f, olhos[i].y - 1.0f }, 5.0f,
                    Fade((Color){ 255, 140, 40, 255 }, 0.07f));
        EndBlendMode();
    }
}

static void DesenhaVinhetaSuave(void)
{
    DrawRectangleGradientH(0, 0, 130, ALTURA_TELA,
                           Fade(BLACK, 0.55f), BLANK);
    DrawRectangleGradientH(LARGURA_TELA - 130, 0, 130, ALTURA_TELA,
                           BLANK, Fade(BLACK, 0.55f));
    DrawRectangleGradientV(0, 0, LARGURA_TELA, 90,
                           Fade(BLACK, 0.45f), BLANK);
    DrawRectangleGradientV(0, ALTURA_TELA - 90, LARGURA_TELA, 90,
                           BLANK, Fade(BLACK, 0.5f));
}

void DesenhaTelaInicio(float tempo, Texture2D texturaPersonagem)
{
    DesenhaCeuEstrelado(tempo);
    DesenhaLua(tempo);
    DesenhaNuvens(tempo);
    DesenhaHorizonte();
    DesenhaMar(tempo);
    DesenhaPraia();
    DesenhaCostaDaCabana();
    DesenhaMataDaFrente();
    DesenhaCabana(tempo);
    DesenhaVagaLumes(tempo);

    DesenhaPersonagemInicio(texturaPersonagem, tempo);
    DesenhaFogueira(tempo);
    DesenhaOlhosNaMata(tempo);

    DesenhaVinhetaSuave();

    // Título "impresso" torto no canto, como capa de fanzine.
    DesenhaPalavraTorta("SCARY", 54.0f, 42.0f, 34.0f, 8.0f,
                        (Color){ 205, 196, 176, 255 },
                        Fade(BLACK, 0.75f), 1.3f);
    DesenhaPalavraTorta("NIGHTS", 50.0f, 80.0f, 78.0f, 1.0f,
                        (Color){ 164, 38, 33, 255 },
                        (Color){ 74, 16, 15, 235 }, 4.7f);

    DesenhaTextoComSombra("a ilha não dorme.", 56.0f, 172.0f, 20.0f, 2.0f,
                          (Color){ 142, 132, 124, 255 });

    // Chamada para começar, discreta, junto do título.
    float respira = 0.55f + 0.45f * (0.5f + 0.5f * sinf(tempo * 2.0f));
    Rectangle tecla = { 54, 214, 66, 25 };

    DrawRectangleRounded(tecla, 0.18f, 6, Fade((Color){ 12, 12, 17, 255 }, 0.75f));
    DrawRectangleRoundedLines(tecla, 0.18f, 6,
                              Fade((Color){ 150, 140, 126, 255 },
                                   0.45f + 0.35f * respira));

    Vector2 medidaEnter = MeasureTextEx(GetFontDefault(), "ENTER", 10.0f, 2.0f);
    DrawTextEx(GetFontDefault(), "ENTER",
               (Vector2){ tecla.x + (tecla.width - medidaEnter.x) / 2.0f,
                          tecla.y + 8.0f },
               10.0f, 2.0f,
               Fade((Color){ 224, 216, 200, 255 }, 0.7f + 0.3f * respira));

    DesenhaTextoComSombra("começar", 130.0f, 217.0f, 20.0f, 2.0f,
                          Fade((Color){ 190, 180, 165, 255 }, respira));

    DesenhaTextoComSombra("WASD mover     F interagir     TAB inventário",
                          54.0f, 400.0f, 10.0f, 2.0f,
                          (Color){ 124, 118, 110, 255 });
    DesenhaTextoComSombra("C crafting     F11 tela cheia     ESC sair",
                          54.0f, 416.0f, 10.0f, 2.0f,
                          (Color){ 124, 118, 110, 255 });
}

void DesenhaTelaMorte(float tempoMorte, float tempoRestante, int dia)
{
    float entrada = Limitar01(tempoMorte / 0.8f);
    float pulso   = 0.5f + 0.5f * sinf(tempoMorte * 4.5f);

    DrawRectangleGradientV(0, 0, LARGURA_TELA, ALTURA_TELA,
                           (Color){ 12, 0, 4, 255 },
                           (Color){ 44, 2, 8, 255 });

    DesenhaVinheta((Color){ 120, 0, 0, 255 }, 0.9f + 0.35f * pulso);

    // Sangue escorrendo do topo
    Color sangue = (Color){ 105, 6, 12, 255 };
    for (int i = 0; i < 9; i++)
    {
        float x = 34.0f + i * 90.0f;
        float comprimento = 22.0f + (float)((i * 47) % 58) +
                            6.0f * (0.5f + 0.5f * sinf(tempoMorte + i));
        float largura = 5.0f + (float)((i * 11) % 4);
        float alturaVisivel = comprimento * entrada;

        DrawRectangle((int)x, 0, (int)largura, (int)alturaVisivel, sangue);
        DrawCircleV((Vector2){ x + largura / 2.0f, alturaVisivel },
                    largura * 0.6f, sangue);
    }

    // Título tremendo nos primeiros instantes
    float tremor = (1.0f - Limitar01(tempoMorte / 1.4f)) * 5.0f;
    float ox = sinf(tempoMorte * 67.0f) * tremor;
    float oy = cosf(tempoMorte * 59.0f) * tremor;

    const char *titulo = "VOCÊ MORREU";
    Vector2 medida = MeasureTextEx(GetFontDefault(), titulo, 68.0f, 5.0f);
    float tx = (LARGURA_TELA - medida.x) / 2.0f + ox;
    float ty = 112.0f + oy;

    DesenhaTituloComBrilho(titulo, tx, ty, 68.0f, 5.0f,
                           (Color){ 208, 26, 34, 255 },
                           0.8f + 0.5f * pulso, entrada);

    DesenhaTextoCentralizado("A ILHA NÃO PERDOA.", 200.0f, 17.0f, 3.0f,
                             Fade((Color){ 160, 150, 165, 255 }, entrada));

    char textoDia[64];
    snprintf(textoDia, sizeof(textoDia), "DIA ALCANÇADO: %d", dia);
    DesenhaTextoCentralizado(textoDia, 242.0f, 20.0f, 2.0f,
                             Fade((Color){ 225, 215, 220, 255 }, entrada));

    DesenhaTextoCentralizado("SEUS ITENS FORAM PERDIDOS", 272.0f, 13.0f, 2.0f,
                             Fade((Color){ 155, 110, 118, 255 }, entrada));

    // Barra de renascimento
    float progresso = 1.0f - Limitar01(tempoRestante / DURACAO_TELA_MORTE);
    DrawRectangleRounded((Rectangle){ 250, 314, 300, 12 }, 1.0f, 8,
                         Fade((Color){ 40, 10, 14, 255 }, entrada));
    if (progresso > 0.01f)
    {
        DrawRectangleRounded((Rectangle){ 250, 314, 300 * progresso, 12 }, 1.0f, 8,
                             Fade((Color){ 200, 28, 38, 255 }, entrada));
    }

    int restantes = (int)ceilf(tempoRestante);
    if (restantes < 0) restantes = 0;
    if (restantes > (int)DURACAO_TELA_MORTE) restantes = (int)DURACAO_TELA_MORTE;

    char textoRenascimento[64];
    snprintf(textoRenascimento, sizeof(textoRenascimento),
             "VOLTANDO AO MENU EM %d...", restantes);
    DesenhaTextoCentralizado(textoRenascimento, 342.0f, 16.0f, 2.0f,
                             Fade((Color){ 190, 180, 190, 255 }, entrada));

    if (tempoMorte >= 1.0f)
    {
        float pisca = 0.4f + 0.6f * (0.5f + 0.5f * sinf(tempoMorte * 3.0f));
        DesenhaTextoCentralizado("ENTER PARA VOLTAR AO MENU", 382.0f, 12.0f, 2.0f,
                                 Fade((Color){ 140, 130, 145, 255 }, pisca * entrada));
    }
}

// Captura as letras digitadas pelo jogador na tela de morte
void AtualizarCampoNome(char *nome, int *contador)
{
    // 1. Captura as letras digitadas pelo jogador
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125) && key != ';' && (*contador < 31)) {
            nome[*contador] = (char)key;
            nome[*contador + 1] = '\0';
            (*contador)++;
        }
        key = GetCharPressed();
    }

    // Apaga com o Backspace
    if (IsKeyPressed(KEY_BACKSPACE)) {
        (*contador)--;
        if (*contador < 0) *contador = 0;
        nome[*contador] = '\0';
    }
}

// Caixa de texto para o jogador digitar o nome
void DesenhaCaixaNome(const char *nome, int contador)
{
    DrawRectangle(530, 330, 250, 40, (Color){ 20, 10, 15, 255 });
    DrawRectangleLines(530, 330, 250, 40, RED);

    if (contador > 0) {
        DrawText(nome, 540, 340, 20, WHITE);
    }else{
        DrawText("Digite seu nome e ENTER", 540, 342, 16, GRAY);
    }
}

Rectangle AjustarTela(void)
{
    int largura = GetScreenWidth();
    int altura = GetScreenHeight();
    float escala = fminf((float)largura / LARGURA_TELA, (float)altura / ALTURA_TELA);
    Rectangle destino = {
        floorf((largura - LARGURA_TELA * escala) / 2.0f),
        floorf((altura - ALTURA_TELA * escala) / 2.0f),
        LARGURA_TELA * escala,
        ALTURA_TELA * escala
    };

    // Converte o mouse da janela para as coordenadas usadas pelo jogo.
    SetMouseOffset(-(int)destino.x, -(int)destino.y);
    SetMouseScale(1.0f / escala, 1.0f / escala);
    return destino;
}

void ApresentarTela(RenderTexture2D tela, Rectangle destino)
{
    EndTextureMode();
    BeginDrawing();
        ClearBackground(BLACK);
        // A altura negativa corrige a orientação da textura renderizada.
        DrawTexturePro(tela.texture,
                       (Rectangle){ 0, 0, LARGURA_TELA, -ALTURA_TELA },
                       destino, (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}
