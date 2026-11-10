#include "raylib.h"
#include <stdio.h>
#include "entities.c"
#include "crafting.c"
#include "math.h"
#include "time.h"
#include "raymath.h"
// Ao modificar o codigo, o projeto deve ser recompilado para que as alterações tenham efeito.
// comando pra fazer isso: gcc main.c -o jogo.exe -I C:\raylib\include -L C:\raylib\lib -lraylib -lopengl32 -lgdi32 -lwinmm
// certifiqque-se de que o jogo previamente aberto foi fechado antes de recompilar.
// para executar o jogo, abra o terminal na pasta do projeto e digite: ./jogo.exe

#define LARGURA_TELA 800
#define ALTURA_TELA  450

#define TILE        32
#define MAPA_COLS   100
#define MAPA_LINHAS 75

#define MAPA_LARGURA (MAPA_COLS * TILE)   // 3200
#define MAPA_ALTURA  (MAPA_LINHAS * TILE) // 2400

#define COLS 100
#define LINHAS 75

#define MAPA_ILHA    "mapa.txt"
#define MAPA_CASA    "casa.txt"
#define MAPA_CAVERNA "caverna.txt"
#define TILE_PORTA_CASA   7
#define TILE_PORTA_CAVERNA 8

#define CASA_SPAWN_X (55 * TILE + TILE / 2.0f)
#define CASA_SPAWN_Y (38 * TILE + TILE / 2.0f)
#define CAVERNA_SPAWN_X (96 * TILE + TILE / 2.0f)
#define CAVERNA_SPAWN_Y (19 * TILE + TILE / 2.0f)

// Sprite do personagem (spritesheet 4x4, frames de 32x32)
#define SPRITE_TAM      32
#define SPRITE_FRAMES   4
#define SPRITE_VEL_ANIM 0.12f   // segundos por frame
#define CORACOES        5
#define VIDA_JOGADOR    (CORACOES * 2.0f)   // 10 pontos = 5 corações

// Arvores
#define MAX_ARVORES        300
#define DENSIDADE_ARVORES  8        // % de chance de cada tile 6 virar árvore
#define DIST_MIN_ARVORES   80.0f   // distância mínima entre árvores (px)
#define ESCALA_ARVORE      2.5f 
#define RAIO_TRONCO        12.0f

#define MAX_PEDRAS        200
#define DENSIDADE_PEDRAS  5        // % de chance de cada tile 3 virar pedra
#define DIST_MIN_PEDRAS   80.0f   // distância mínima entre pedras (px)
#define ESCALA_PEDRA      2.0f
#define RAIO_PEDRA        12.0f

// Recortes no spritesheet (arvore.png, 74x62)
#define RECORTE_TOCO   (Rectangle){ 1, 48, 22, 14 }
#define RECORTE_ARVORE (Rectangle){ 34, 0, 39, 61 }

#define DANO_AGUA     1.5f       // pontos por segundo (meio coração/s)
#define MAX_MONSTROS 100

// Ciclo dia/noite
#define HORA_INICIO      6.0f    // o jogo começa às 06:00
#define ESCURIDAO_MAXIMA 0.75f   // opacidade máxima da noite (0 = sem escuro, 1 = preto)

// Telas do jogo
#define DURACAO_TELA_MORTE 4.0f  // segundos na tela de morte antes de renascer
#define NUM_ESTRELAS       90

typedef enum {
    TELA_INICIO = 0,
    TELA_JOGO,
    TELA_MORTE
} TelaJogo;

Monstro monstros[MAX_MONSTROS] = {0};
int quantidadeMonstros = 0;



float timerSpawn = 0.0f;
float intervaloSpawn = 2.0f; // um monstro a cada 2 segundos

static int ColideComParede(char mapa[LINHAS][COLS], Vector2 centro, float raio)
{
    int minX = (int)((centro.x - raio) / TILE);
    int maxX = (int)((centro.x + raio) / TILE);
    int minY = (int)((centro.y - raio) / TILE);
    int maxY = (int)((centro.y + raio) / TILE);

    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            // Somente os tiles andaveís (valor 1, 2 e 6) permitem a passagem; fora do mapa também bloqueia.
            if (x >= 0 && x < COLS && y >= 0 && y < LINHAS && (mapa[y][x] == 0 || mapa[y][x] == 2 || mapa[y][x] == 3 || mapa[y][x] == 1 || mapa[y][x] == 6 || mapa[y][x] == 7 || mapa[y][x] == 8 || mapa[y][x] == 9)) continue;

            float esquerda = x * TILE;
            float topo = y * TILE;
            float pertoX = centro.x < esquerda ? esquerda : (centro.x > esquerda + TILE ? esquerda + TILE : centro.x);
            float pertoY = centro.y < topo ? topo : (centro.y > topo + TILE ? topo + TILE : centro.y);
            float dx = centro.x - pertoX;
            float dy = centro.y - pertoY;
            if (dx * dx + dy * dy < raio * raio) return 1;
        }
    }

    return 0;
}

static int ColideComRecursos(Vector2 atual, Vector2 proximo, float raio, Recursos *recursos)
{
    for (Recursos *recurso = recursos; recurso != NULL; recurso = recurso->proximo)
    {
        if (recurso->raio_colisao <= 0.0f) continue;

        Vector2 centro = Vector2Add(recurso->posicao, recurso->offset_colisao);
        float limite = raio + recurso->raio_colisao;

        float dxN = proximo.x - centro.x, dyN = proximo.y - centro.y;
        float dxA = atual.x - centro.x, dyA = atual.y - centro.y;
        float distNova  = dxN * dxN + dyN * dyN;
        float distAtual = dxA * dxA + dyA * dyA;

        // Evita prender o personagem caso um recurso reapareça sobre ele.
        if (distNova < limite * limite && distNova < distAtual) return 1;
    }
    return 0;
}

static int SobrepoeRecurso(Vector2 posicao, float raio, Recursos *recursos)
{
    for (Recursos *recurso = recursos; recurso != NULL; recurso = recurso->proximo)
    {
        if (recurso->raio_colisao <= 0.0f) continue;

        Vector2 centro = Vector2Add(recurso->posicao, recurso->offset_colisao);
        float limite = raio + recurso->raio_colisao;
        float dx = posicao.x - centro.x;
        float dy = posicao.y - centro.y;

        if (dx * dx + dy * dy < limite * limite) return 1;
    }
    return 0;
}

static int TileEm(char mapa[LINHAS][COLS], Vector2 pos);

void SpawnMonstro(
    Monstro monstros[],
    int *quantidadeMonstros,
    Vector2 jogador,
    char mapa[LINHAS][COLS],
    Recursos *arvores
) {
    if (*quantidadeMonstros >= MAX_MONSTROS) return;

    for (int tentativa = 0; tentativa < 100; tentativa++) {

        // Escolhe uma posição aleatória entre -20 e +20 tiles
        int offsetX = GetRandomValue(-20, 20);
        int offsetY = GetRandomValue(-20, 20);

        Vector2 pos = {
            jogador.x + offsetX * TILE,
            jogador.y + offsetY * TILE
        };

        // Verifica distância real até o jogador
        float dx = pos.x - jogador.x;
        float dy = pos.y - jogador.y;
        float distancia2 = dx * dx + dy * dy;

        float minDist = 5.0f * TILE;
        float maxDist = 20.0f * TILE;

        if (distancia2 < minDist * minDist ||
            distancia2 > maxDist * maxDist) {
            continue;
        }

        // Monstros não nascem na água
        if (TileEm(mapa, pos) == 0) continue;

        // Verifica colisão com paredes
        if (ColideComParede(mapa, pos, 12.0f)) continue;

        // Não permite que monstros nasçam sobre recursos.
        if (SobrepoeRecurso(pos, 12.0f, arvores)) continue;

        // Posição válida: cria o monstro
        monstros[*quantidadeMonstros] = (Monstro){
            .posicao = pos,
            .velocidade = 100.0f,
            .nivel = 1,
            .vida = 10.0f,
            .vida_maxima = 10.0f,
            .ataque = 3.0f,
            .defesa = 0.0f,
            .morto = false
        };

        (*quantidadeMonstros)++;
        return;
    }
}

int CarregarMapa(const char *caminho, char mapa[LINHAS][COLS])
{
    FILE *f = fopen(caminho, "r"); //Abre o arquivo para modificar o que deve ser feito (Carregar o mapa).
    if (f == NULL)
    {
        TraceLog(LOG_ERROR, "Nao abriu o arquivo: %s", caminho); //Trata o erro de caso não tenha aberto o arquivo
        return 0;
    }

    char linha[COLS + 4]; //Cria uma string (+4 -> Espaço extra por preocausão)
    for (int y = 0; y < LINHAS; y++)
    {
        if (fgets(linha, sizeof(linha), f) == NULL) break; // Passa por todos até achar uma quebra de linha.
        for (int x = 0; x < COLS; x++)
        {
            char c = linha[x];
            mapa[y][x] = (c >= 'a') ? c : c - '0';
        }
    }
    fclose(f);
    return 1;
}

static void DesenhaArvore(Texture2D tex, Recursos *r)
{
    Rectangle src = r->quebrado ? RECORTE_TOCO : RECORTE_ARVORE;

    float w = src.width  * ESCALA_ARVORE;
    float h = src.height * ESCALA_ARVORE;

    Rectangle dst = { r->posicao.x, r->posicao.y, w, h };
    Vector2 origem = { w / 2.0f, h };   // base, no centro (já escalado)

    DrawTexturePro(tex, src, dst, origem, 0.0f, WHITE);
}

// Retorna o id do tile sob a posição (ou -1 se estiver fora do mapa)
static int TileEm(char mapa[LINHAS][COLS], Vector2 pos)
{
    int tx = (int)(pos.x / TILE);
    int ty = (int)(pos.y / TILE);
    if (tx < 0 || tx >= COLS || ty < 0 || ty >= LINHAS) return -1;
    return mapa[ty][tx];
}

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

static void DesenhaVida(float vida)
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

void MoverMonstro(
    Monstro *monstro,
    Vector2 jogador,
    float dt,
    char mapa[LINHAS][COLS],
    Recursos *arvores
) {
    if (monstro->morto) return;

    Vector2 direcao = Vector2Subtract(jogador, monstro->posicao);
    float distancia = Vector2Length(direcao);

    if (distancia < 1.0f) return;

    direcao = Vector2Scale(direcao, 1.0f / distancia);

    float passo = monstro->velocidade * dt;
    if (passo > distancia) passo = distancia;

    float raio = 12.0f;

    // Movimento horizontal
    Vector2 proximo = monstro->posicao;
    proximo.x += direcao.x * passo;

    if (!ColideComParede(mapa, proximo, raio) &&
        !ColideComRecursos(monstro->posicao, proximo, raio, arvores)) {
        monstro->posicao.x = proximo.x;
    }

    // Movimento vertical
    proximo = monstro->posicao;
    proximo.y += direcao.y * passo;

    if (!ColideComParede(mapa, proximo, raio) &&
        !ColideComRecursos(monstro->posicao, proximo, raio, arvores)) {
        monstro->posicao.y = proximo.y;
    }
}

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

static void IniciarEstrelas(void)
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

static void DesenhaTextoCentralizado(const char *texto, float y, float tamanho,
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

static void DesenhaCeuNoturno(float tempo)
{
    DrawRectangleGradientV(0, 0, LARGURA_TELA, ALTURA_TELA,
                           (Color){ 6, 8, 20, 255 },
                           (Color){ 30, 8, 18, 255 });

    // Estrelas cintilando
    for (int i = 0; i < NUM_ESTRELAS; i++)
    {
        float cintila = 0.55f + 0.45f * sinf(tempo * 1.7f + estrelas[i].fase);
        float alpha   = estrelas[i].brilho * cintila;
        DrawCircleV((Vector2){ estrelas[i].x, estrelas[i].y },
                    estrelas[i].tamanho,
                    Fade((Color){ 220, 225, 255, 255 }, alpha));
    }

    // Lua com halo e crateras
    Vector2 lua = { LARGURA_TELA - 62.0f, 62.0f };
    for (int i = 5; i >= 1; i--)
        DrawCircleV(lua, 30.0f + i * 7.0f,
                    Fade((Color){ 180, 195, 255, 255 }, 0.022f));

    DrawCircleV(lua, 30.0f, (Color){ 232, 230, 214, 255 });
    DrawCircleV((Vector2){ lua.x - 9.0f, lua.y - 7.0f }, 6.0f,
                Fade((Color){ 190, 188, 170, 255 }, 0.8f));
    DrawCircleV((Vector2){ lua.x + 8.0f, lua.y + 6.0f }, 4.5f,
                Fade((Color){ 190, 188, 170, 255 }, 0.7f));
    DrawCircleV((Vector2){ lua.x + 2.0f, lua.y - 14.0f }, 3.0f,
                Fade((Color){ 190, 188, 170, 255 }, 0.6f));

    // Névoa passando devagar
    for (int i = 0; i < 4; i++)
    {
        float velocidade = 12.0f + i * 5.0f;
        float x = fmodf(tempo * velocidade + i * 270.0f, LARGURA_TELA + 420.0f) - 210.0f;
        float y = ALTURA_TELA - 90.0f + i * 13.0f;
        DrawEllipse((int)x, (int)y, 190.0f, 24.0f,
                    Fade((Color){ 150, 160, 190, 255 }, 0.06f));
    }

    // Chão escuro
    DrawRectangle(0, ALTURA_TELA - 48, LARGURA_TELA, 48, (Color){ 4, 5, 10, 255 });
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

static void DesenhaFlorestaSilhueta(float tempo)
{
    Color cor = (Color){ 3, 4, 8, 255 };

    for (int i = 0; i < 14; i++)
    {
        float x = 18.0f + i * 62.0f + sinf(tempo * 0.8f + i * 0.7f) * 1.5f;
        float altura = 70.0f + (float)((i * 37) % 55);
        float base = ALTURA_TELA - 34.0f + (float)((i * 13) % 8);

        DesenhaPinheiro(x, base, altura, cor);
    }
}

static void DesenhaTelaInicio(float tempo)
{
    DesenhaCeuNoturno(tempo);
    DesenhaFlorestaSilhueta(tempo);

    DesenhaTextoCentralizado("UM JOGO DE SOBREVIVÊNCIA", 48.0f, 14.0f, 4.0f,
                             (Color){ 130, 125, 150, 255 });

    const char *titulo = "SCARY NIGHTS";
    Vector2 medida = MeasureTextEx(GetFontDefault(), titulo, 76.0f, 6.0f);
    float tx = (LARGURA_TELA - medida.x) / 2.0f;
    float pulso = 0.72f + 0.28f * (0.5f + 0.5f * sinf(tempo * 2.4f));

    DesenhaTituloComBrilho(titulo, tx, 82.0f, 76.0f, 6.0f,
                           (Color){ 214, 28, 40, 255 }, pulso, 1.0f);

    DesenhaTextoCentralizado("SOBREVIVA À NOITE. TEMA O QUE ESPREITA.",
                             176.0f, 17.0f, 2.0f,
                             (Color){ 175, 170, 190, 255 });

    // Linha decorativa
    int linhaY = 212;
    DrawRectangle(LARGURA_TELA / 2 - 150, linhaY, 110, 2,
                  Fade((Color){ 190, 30, 40, 255 }, 0.8f));
    DrawRectangle(LARGURA_TELA / 2 + 40, linhaY, 110, 2,
                  Fade((Color){ 190, 30, 40, 255 }, 0.8f));
    DrawCircleV((Vector2){ LARGURA_TELA / 2.0f, linhaY + 1.0f }, 4.0f,
                (Color){ 214, 28, 40, 255 });

    // Chamada piscando
    float pisca = 0.45f + 0.55f * (0.5f + 0.5f * sinf(tempo * 3.2f));
    DesenhaTextoCentralizado("PRESSIONE ENTER PARA COMEÇAR", 246.0f, 24.0f, 3.0f,
                             Fade((Color){ 240, 235, 225, 255 }, pisca));

    // Caixa de controles
    DrawRectangleRounded((Rectangle){ 168, 296, 464, 88 }, 0.18f, 8,
                         Fade((Color){ 8, 8, 14, 255 }, 0.72f));
    DesenhaTextoCentralizado("WASD / SETAS - MOVER      E - COLETAR",
                             306.0f, 15.0f, 1.5f,
                             (Color){ 190, 188, 205, 255 });
    DesenhaTextoCentralizado("TAB - INVENTÁRIO      C - CRAFTING",
                             330.0f, 15.0f, 1.5f,
                             (Color){ 190, 188, 205, 255 });
    DesenhaTextoCentralizado("ESC - SAIR", 357.0f, 12.0f, 2.0f,
                             (Color){ 120, 118, 138, 255 });

    DesenhaVinheta(BLACK, 1.0f);
}

static void DesenhaTelaMorte(float tempoMorte, float tempoRestante, int dia)
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
             "RENASCENDO EM %d...", restantes);
    DesenhaTextoCentralizado(textoRenascimento, 342.0f, 16.0f, 2.0f,
                             Fade((Color){ 190, 180, 190, 255 }, entrada));

    if (tempoMorte >= 1.0f)
    {
        float pisca = 0.4f + 0.6f * (0.5f + 0.5f * sinf(tempoMorte * 3.0f));
        DesenhaTextoCentralizado("ENTER PARA RENASCER AGORA", 382.0f, 12.0f, 2.0f,
                                 Fade((Color){ 140, 130, 145, 255 }, pisca * entrada));
    }
}

int main(void)
{
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Scary Nights");
    SetTargetFPS(60);

    Itens_Init();

    IniciarEstrelas();

    Vector2 jogador = { MAPA_LARGURA / 2.0f, MAPA_ALTURA / 2.0f };
    float velocidade = 300.0f;
    float vida = VIDA_JOGADOR;
    Vector2 inicio = jogador;     // ponto de renascimento

    Inventario inv = {0};
    int invAberto = 0;
    // inventory[1] vai representar a Madeira. 
    

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
    };
    int totalReceitas = (int)(sizeof(receitasCraft) / sizeof(receitasCraft[0]));
    int receitaSelecionada = 0;
    int craftingAberto = 0;
    char mensagemCraft[64] = "";
    float tempoMensagemCraft = 0.0f;
    Color corMensagemCraft = WHITE;

    // --- CARREGAMENTO DE TEXTURAS (Uma única vez aqui) ---
    float tempo_jogo = 0.0f;          // Conta os segundos passados no jogo
    float duracao_dia_noite = 60.0f;  // Duração completa de um ciclo 
    // --- Sprite do personagem (carregar depois do InitWindow) ---
    Texture2D personagem = LoadTexture("pictures/personagem.png");
    SetTextureFilter(personagem, TEXTURE_FILTER_POINT); // mantém a pixel art nítida
    
    Texture2D texArvore = LoadTexture("pictures/arvore.png");
    SetTextureFilter(texArvore, TEXTURE_FILTER_POINT);

    int frameAtual = 0;
    int direcao = 0;          // 0=baixo, 1=esquerda, 2=direita, 3=cima
    float timerAnim = 0.0f;

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ LARGURA_TELA / 2.0f, ALTURA_TELA / 2.0f }; // centro da tela
    camera.target = jogador;
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    char mapa[LINHAS][COLS];

    if (!CarregarMapa(MAPA_ILHA, mapa)) return 1;

    printf("Arquivo existe: %d\n", FileExists("pictures/carangueijo.png"));

    Texture2D texturaMonstro = LoadTexture("pictures/carangueijo.png");

    if (!IsTextureValid(texturaMonstro)) {
        printf("ERRO: Nao foi possivel carregar a textura do monstro!\n");
    } else {
        printf("Textura carregada com sucesso! Tamanho: %d x %d\n",
            texturaMonstro.width, texturaMonstro.height);
    }

    Recursos *arvores = NULL;
    Recursos *pedras = NULL;
    int numPedras = 0;
    int pedra = 0;
    int numArvores = 0;
    int madeira = 0;

    for (int ty = 0; ty < LINHAS && numArvores < MAX_ARVORES; ty++)
    {
        for (int tx = 0; tx < COLS && numArvores < MAX_ARVORES; tx++)
        {
            if (mapa[ty][tx] != 6) continue;                      // só nos tiles 6
            if (GetRandomValue(0, 99) >= DENSIDADE_ARVORES) continue;

            Vector2 pos = { tx * TILE + TILE / 2.0f, ty * TILE + TILE };

            // não nasce em cima do jogador
            float dx = pos.x - jogador.x, dy = pos.y - jogador.y;
            if (dx * dx + dy * dy < 64.0f * 64.0f) continue;

                        // nem colada em outra árvore
            int perto = 0;
            for (Recursos *a = arvores; a != NULL; a = a->proximo)
            {
                dx = pos.x - a->posicao.x;
                dy = pos.y - a->posicao.y;
                if (dx * dx + dy * dy < DIST_MIN_ARVORES * DIST_MIN_ARVORES) { perto = 1; break; }
            }
            if (perto) continue;

            inserir_recurso(&arvores, (Recursos){
                .posicao = pos,
                .offset_colisao = { 0.0f, -20.0f },
                .raio_colisao = RAIO_TRONCO,
                .vida = 3,
                .vida_maxima = 3,
                .quebrado = false,
                .tempo_renascimento = 0.0f,
                .cooldown_renascimento = 10.0f
            });
            numArvores++;
        }
    }

    for (int ty = 0; ty < LINHAS && numPedras < MAX_PEDRAS; ty++)
    {
        for (int tx = 0; tx < COLS && numPedras < MAX_PEDRAS; tx++)
        {
            if (mapa[ty][tx] != 3) continue;                      // só nos tiles 3
            if (GetRandomValue(0, 99) >= DENSIDADE_PEDRAS) continue;

            Vector2 pos = { tx * TILE + TILE / 2.0f, ty * TILE + TILE };

            // não nasce em cima do jogador
            float dx = pos.x - jogador.x, dy = pos.y - jogador.y;
            if (dx * dx + dy * dy < 64.0f * 64.0f) continue;

                        // nem colada em outra pedra
            int perto = 0;
            for (Recursos *a = pedras; a != NULL; a = a->proximo)
            {
                dx = pos.x - a->posicao.x;
                dy = pos.y - a->posicao.y;
                if (dx * dx + dy * dy < DIST_MIN_PEDRAS * DIST_MIN_PEDRAS) { perto = 1; break; }
            }
            if (perto) continue;

            inserir_recurso(&pedras, (Recursos){
                .posicao = pos,
                .offset_colisao = { 0.0f, -20.0f },
                .raio_colisao = RAIO_PEDRA,
                .vida = 5,
                .vida_maxima = 5,
                .quebrado = false,
                .tempo_renascimento = 0.0f,
                .cooldown_renascimento = 30.0f
            });
            numPedras++;
        }
    }
    
    int dentro = 0;                          // 0 = fora, 1 = dentro do local
    Vector2 posAntesDeEntrar = jogador;      // para voltar ao lugar certo ao sair
    Recursos *arvoresAtivas = arvores;       // árvores do mapa atual (NULL dentro de casa/caverna)
    int dia = 1;                             // contador de dias

    TelaJogo telaAtual = TELA_INICIO;
    float tempoTela  = 0.0f;
    float tempoMorte = 0.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        tempoTela += dt;

        // --- TELA DE INÍCIO ---
        if (telaAtual == TELA_INICIO)
        {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE))
            {
                telaAtual = TELA_JOGO;
                tempoTela = 0.0f;
            }

            BeginDrawing();
                ClearBackground(BLACK);
                DesenhaTelaInicio(tempoTela);
            EndDrawing();

            continue;
        }

        // --- TELA DE MORTE ---
        if (telaAtual == TELA_MORTE)
        {
            tempoMorte += dt;

            if ((tempoMorte >= 1.0f && IsKeyPressed(KEY_ENTER)) ||
                tempoMorte >= DURACAO_TELA_MORTE)
            {
                if (dentro && CarregarMapa(MAPA_ILHA, mapa))
                {
                    dentro = 0;
                    arvoresAtivas = arvores;
                }

                jogador = inicio;
                vida = VIDA_JOGADOR;

                // Reinicia a run: volta ao dia 1 e perde todos os itens
                dia = 1;
                tempo_jogo = 0.0f;
                inv = (Inventario){0};
                invAberto = 0;
                craftingAberto = 0;
                receitaSelecionada = 0;
                mensagemCraft[0] = '\0';
                tempoMensagemCraft = 0.0f;

                telaAtual = TELA_JOGO;
                tempoTela = 0.0f;
            }

            BeginDrawing();
                ClearBackground(BLACK);
                DesenhaTelaMorte(tempoMorte, DURACAO_TELA_MORTE - tempoMorte, dia);
            EndDrawing();

            continue;
        }

        if (tempoMensagemCraft > 0.0f) tempoMensagemCraft -= dt;

        // --- SISTEMA DE HORÁRIO ---
        // tempo_jogo conta os segundos do dia atual; ao completar o ciclo, começa outro dia.
        tempo_jogo += dt;
        if (tempo_jogo >= duracao_dia_noite)
        {
            tempo_jogo -= duracao_dia_noite;
            dia++;
        }

        // O ciclo começa às 06:00 e avança 24h em duracao_dia_noite segundos.
        float progresso_dia = tempo_jogo / duracao_dia_noite;
        float hora_frac = fmodf(progresso_dia * 24.0f + HORA_INICIO, 24.0f);
        int hora   = (int)hora_frac;
        int minuto = (int)((hora_frac - hora) * 60.0f);

        // 00:00 = escuridão máxima, 12:00 = claridade máxima.
        float angulo_dia = (hora_frac / 24.0f) * 2.0f * PI;
        float intensidade_noite = (cosf(angulo_dia) + 1.0f) * 0.5f * ESCURIDAO_MAXIMA;
        bool isNoite = intensidade_noite > 0.4f;

        // Movimento do jogador (bloqueado enquanto o crafting está aberto)
        Vector2 proximo = jogador;
        if (!craftingAberto)
        {
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) proximo.x += velocidade * dt;
            if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) proximo.x -= velocidade * dt;
            if (!ColideComParede(mapa, proximo, 14.0f) && !ColideComRecursos(jogador, proximo, 14.0f, arvoresAtivas)) jogador.x = proximo.x;

            proximo = jogador;
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) proximo.y += velocidade * dt;
            if (IsKeyDown(KEY_UP)   || IsKeyDown(KEY_W)) proximo.y -= velocidade * dt;
            if (!ColideComParede(mapa, proximo, 14.0f) && !ColideComRecursos(jogador, proximo, 14.0f, arvoresAtivas)) jogador.y = proximo.y;
        }

        // Limita o jogador dentro do mapa
        if (jogador.x < 0) jogador.x = 0;
        if (jogador.y < 0) jogador.y = 0;
        if (jogador.x > MAPA_LARGURA) jogador.x = MAPA_LARGURA;
        if (jogador.y > MAPA_ALTURA)  jogador.y = MAPA_ALTURA;

        // Guarda o último ponto que NÃO era porta (é para onde voltamos ao sair)
        if (!dentro && TileEm(mapa, jogador) != (TILE_PORTA_CASA) && TileEm(mapa, jogador) != (TILE_PORTA_CAVERNA)) posAntesDeEntrar = jogador;

        // Pisou na porta: troca de mapa
        if (TileEm(mapa, jogador) == TILE_PORTA_CASA)
        {
            if (!dentro)
            {
                if (CarregarMapa(MAPA_CASA, mapa))
                {
                    dentro = 1;
                    jogador = (Vector2){ CASA_SPAWN_X, CASA_SPAWN_Y };
                }
            }
            else
            {
                if (CarregarMapa(MAPA_ILHA, mapa))
                {
                    dentro = 0;
                    jogador = posAntesDeEntrar;
                }
            }
            arvoresAtivas = dentro ? NULL : arvores;
        }

        if(TileEm(mapa, jogador) == TILE_PORTA_CAVERNA)
        {
            if (!dentro)
            {
                if (CarregarMapa(MAPA_CAVERNA, mapa))
                {
                    dentro = 1;
                    jogador = (Vector2){ CAVERNA_SPAWN_X, CAVERNA_SPAWN_Y };
                }
            }
            else
            {
                if (CarregarMapa(MAPA_ILHA, mapa))
                {
                    dentro = 0;
                    jogador = posAntesDeEntrar;
                }
            }
            arvoresAtivas = dentro ? NULL : arvores;
        }

        // Direção e animação do personagem
        int andando = 0;
        if (!craftingAberto)
        {
            if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) { direcao = 0; andando = 1; }
            if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) { direcao = 3; andando = 1; }
            if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) { direcao = 1; andando = 1; }
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) { direcao = 2; andando = 1; }
        }

        if (andando)
        {
            timerAnim += dt;
            if (timerAnim >= SPRITE_VEL_ANIM)
            {
                timerAnim = 0.0f;
                frameAtual = (frameAtual + 1) % SPRITE_FRAMES;
            }
        }
        else
        {
            frameAtual = 0;   // parado = primeiro frame
            timerAnim = 0.0f;
        }

        // Câmera segue o jogador
        camera.target = jogador;
        camera.zoom += GetMouseWheelMove() * 0.1f;
        if (camera.zoom < 0.5f) camera.zoom = 0.5f;
        if (camera.zoom > 3.0f) camera.zoom = 3.0f;

        // Limita a câmera para não mostrar fora do mapa
        float metadeL = (LARGURA_TELA / 2.0f) / camera.zoom;
        float metadeA = (ALTURA_TELA  / 2.0f) / camera.zoom;

        if (camera.target.x < metadeL) camera.target.x = metadeL;
        if (camera.target.y < metadeA) camera.target.y = metadeA;
        if (camera.target.x > MAPA_LARGURA - metadeL) camera.target.x = MAPA_LARGURA - metadeL;
        if (camera.target.y > MAPA_ALTURA  - metadeA) camera.target.y = MAPA_ALTURA  - metadeA;

                atualiza_recurso(arvores);

        if (!craftingAberto && IsKeyPressed(KEY_E))
        {
            for (Recursos *a = arvoresAtivas; a != NULL; a = a->proximo)
            {
                if (a->quebrado) continue;

                float dx = jogador.x - a->posicao.x;
                float dy = jogador.y - a->posicao.y;
                if (dx * dx + dy * dy < 50.0f * 50.0f)   // alcance de 50 px
                {
                    quebrar_recurso(a, 1);
                    if (a->quebrado) Inv_Add(&inv, ITEM_MADEIRA, 1);   // ganha madeira ao derrubar
                    break;                                // só uma árvore por golpe
                }
            }
        }

        // Abre/fecha a interface de crafting (fecha o inventário para não sobrepor)
        if (IsKeyPressed(KEY_C))
        {
            craftingAberto = !craftingAberto;
            if (craftingAberto) invAberto = 0;
        }

        if (IsKeyPressed(KEY_TAB))
        {
            invAberto = !invAberto;
            if (invAberto) craftingAberto = 0;
        }
        for (int k = 0; k < INV_COLS; k++)
            if (IsKeyPressed(KEY_ONE + k)) inv.selecionado = k;

        // Dano contínuo na água (testa o "pé" do personagem)
        int naAgua = (TileEm(mapa, (Vector2){ jogador.x, jogador.y + 12.0f }) == 0);
        if (naAgua) vida -= DANO_AGUA * dt;
        if (!naAgua && vida < VIDA_JOGADOR) vida += 0.5f * dt;

        // Morreu: mostra a tela de morte antes de renascer
        if (vida <= 0.0f)
        {
            vida = 0.0f;
            telaAtual = TELA_MORTE;
            tempoMorte = 0.0f;
        }
        //logica de spawn de monstros
        if (isNoite){
            timerSpawn += dt;

            if (timerSpawn >= intervaloSpawn) {
                timerSpawn = 0.0f;

                printf("Tentando spawnar monstro...\n");

                SpawnMonstro(
                    monstros,
                    &quantidadeMonstros,
                    jogador,
                    mapa,
                    arvoresAtivas
                );

                printf("Quantidade de monstros: %d\n", quantidadeMonstros);

            }
        }
        else {
            timerSpawn = 0.0f;
        }

        float danoSol = 2.0f;
        
        //atualiza monstros
        for (int i = 0; i < quantidadeMonstros; i++) {
            if (monstros[i].morto) continue;
            
            MoverMonstro(
                &monstros[i],
                jogador,
                dt,
                mapa,
                arvoresAtivas
            );
            
            //dano de sol
            if (!isNoite) {
                monstros[i].vida -= danoSol * dt;

                if (monstros[i].vida <= 0) {
                    monstros[i].vida = 0;
                    monstros[i].morto = true;
                }
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(camera);
                
                // 1. DESENHO DO CHÃO DO MAPA
                for (int y = 0; y < LINHAS; y++)
                {
                    for (int x = 0; x < COLS; x++)
                    {
                        Color cor;
                        switch (mapa[y][x])
                        {
                            case 0: cor = BLUE;      break;
                            case 2: cor = GREEN;     break;
                            case 1: cor = BEIGE;     break;
                            case 3: cor = DARKGRAY;  break;
                            case 4: cor = LIGHTGRAY; break;
                            case 5: cor = BROWN;     break;
                            case 6: cor = (Color){ 20, 140, 60, 255 }; break;
                            case 7: case 8: cor = BLACK;     break;
                            case 9: cor = (Color){ 141, 73, 37, 255 }; break;
                            case 'a': cor = (Color){ 255, 255, 0, 255 }; break;
                            case 'b': cor = (Color){ 255, 0, 255, 255 }; break;
                            case 'c': cor = (Color){ 0, 255, 255, 255 }; break;
                            case 'd': cor = (Color){ 255, 128, 0, 255 }; break;
                            case 'e': cor = (Color){ 128, 0, 255, 255 }; break;
                            case 'f': cor = (Color){ 0, 255, 128, 255 }; break;
                            case 'g': cor = (Color){ 255, 0, 0, 255 };   break;
                            case 'h': cor = (Color){ 69, 46, 81, 255 };  break;
                            case 'i': cor = (Color){ 0, 85, 255, 255 };  break;
                            case 'j': cor = (Color){ 255, 0, 170, 255 }; break;
                            case 'k': cor = (Color){ 0, 250, 255, 255 }; break;
                            case 'l': cor = (Color){ 242, 0, 255, 255 }; break;
                            case 'm': cor = (Color){ 0, 255, 0, 255 };   break;
                            default: cor = BLACK;    break; 
                        }
                        DrawRectangle(x * TILE, y * TILE, TILE, TILE, cor);
                    }
                }

                // 2. Y-SORTING PARTE 1: Desenha as árvores que estão ATRÁS do jogador
                for (Recursos *a = arvoresAtivas; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y <= jogador.y) DesenhaArvore(texArvore, a);
                }

                // 3. DESENHO DO PERSONAGEM
                Rectangle origem  = { frameAtual * (float)SPRITE_TAM, direcao * (float)SPRITE_TAM, SPRITE_TAM, SPRITE_TAM };
                Rectangle destino = { jogador.x, jogador.y, SPRITE_TAM, SPRITE_TAM };
                Vector2 centro = { SPRITE_TAM / 2.0f, SPRITE_TAM / 2.0f };
                DrawTexturePro(personagem, origem, destino, centro, 0.0f, WHITE);
                
                // 4. Y-SORTING PARTE 2: Desenha as árvores que estão na FRENTE do jogador
                // Monstros
                for (int i = 0; i < quantidadeMonstros; i++) {
                    if (monstros[i].morto) continue;

                    Rectangle origem = {
                        0, 0,
                        (float)texturaMonstro.width,
                        (float)texturaMonstro.height
                    };

                    Rectangle destino = {
                        monstros[i].posicao.x,
                        monstros[i].posicao.y,
                        32.0f,
                        32.0f
                    };

                    Vector2 centro = {16.0f, 16.0f};

                    DrawTexturePro(
                        texturaMonstro,
                        origem,
                        destino,
                        centro,
                        0.0f,
                        WHITE
                    );
                }

                for (Recursos *a = arvoresAtivas; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaArvore(texArvore, a);
                }

            EndMode2D();
            DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, Fade(BLACK, intensidade_noite));
            // Tela levemente vermelha enquanto toma dano
            if (naAgua) DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, Fade(RED, 0.2f));

            DesenhaVida(vida);

            int invX = (LARGURA_TELA - INV_LARGURA) / 2;

            if (invAberto) {
                int invY = (ALTURA_TELA - INV_ALTURA) / 2;
                DrawRectangle(invX - INV_PAD, invY - INV_PAD,
                            INV_LARGURA + 2 * INV_PAD, INV_ALTURA + 2 * INV_PAD,
                            Fade(BLACK, 0.6f));
                Inv_Draw(&inv, invX, invY, INV_ROWS);
            } else {
                Inv_Draw(&inv, invX, ALTURA_TELA - SLOT_TAM - 10, 1);
            }

            if (craftingAberto) {
                int craftou = DesenharCraftingUI(
                    receitasCraft, totalReceitas, &inv, &receitaSelecionada,
                    (tempoMensagemCraft > 0.0f) ? mensagemCraft : NULL,
                    corMensagemCraft
                );

                if (craftou) {
                    if (Craftar(&inv, receitasCraft[receitaSelecionada])) {
                        snprintf(mensagemCraft, sizeof(mensagemCraft),
                                 "Craftado: %s", receitasCraft[receitaSelecionada].nome);
                        corMensagemCraft = GREEN;
                    } else {
                        snprintf(mensagemCraft, sizeof(mensagemCraft),
                                 "Faltam ingredientes ou espaco!");
                        corMensagemCraft = RED;
                    }
                    tempoMensagemCraft = 2.0f;
                }
            }

            
            DrawText("Setas ou WASD para mover | E: coletar", 10, 10, 18, BLACK);

            DrawText("TAB: inventario | C: crafting", 10, 32, 16, BLACK);
            DrawText(TextFormat("Dia %d | %s %02d:%02d", dia, isNoite ? "Noite" : "Dia", hora, minuto), 545, 10, 20, isNoite ? BLUE : GOLD);
            DrawFPS(10, 40);
        EndDrawing();
    }

    // --- DESCARREGAR RECURSOS ---
        // --- DESCARREGAR RECURSOS ---
    Itens_Unload();
    liberar_recursos(arvores);
    UnloadTexture(personagem);
    UnloadTexture(texArvore);
    UnloadTexture(texturaMonstro);
    CloseWindow();
    return 0;
}