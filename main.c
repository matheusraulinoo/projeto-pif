#include "raylib.h"
#include <stdio.h>
#include "entitys.c"

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

// Sprite do personagem (spritesheet 4x4, frames de 32x32)
#define SPRITE_TAM      32
#define SPRITE_FRAMES   4
#define SPRITE_VEL_ANIM 0.12f   // segundos por frame

#define ARVORE_TAM      64      
#define ARVORE_FRAMES   2

static int ColideComAzul(int mapa[LINHAS][COLS], Vector2 centro, float raio)
{
    int minX = (int)((centro.x - raio) / TILE);
    int maxX = (int)((centro.x + raio) / TILE);
    int minY = (int)((centro.y - raio) / TILE);
    int maxY = (int)((centro.y + raio) / TILE);

    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            // Somente os tiles verdes (valor 2) permitem a passagem; fora do mapa também bloqueia.
            if (x >= 0 && x < COLS && y >= 0 && y < LINHAS && (mapa[y][x] == 2 || mapa[y][x] == 1)) continue;

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
int CarregarMapa(const char *caminho, int mapa[LINHAS][COLS])
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
            mapa[y][x] = linha[x] - '0';
        }
    }
    fclose(f);
    return 1;
}
int main(void)
{
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Mapa maior que a tela");
    SetTargetFPS(60);

    Vector2 jogador = { MAPA_LARGURA / 2.0f, MAPA_ALTURA / 2.0f };
    float velocidade = 300.0f;

    // --- Sprite do personagem (carregar depois do InitWindow) ---
    Texture2D personagem = LoadTexture("personagem.png");
    SetTextureFilter(personagem, TEXTURE_FILTER_POINT); // mantém a pixel art nítida
    int frameAtual = 0;
    int direcao = 0;          // 0=baixo, 1=esquerda, 2=direita, 3=cima
    float timerAnim = 0.0f;

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ LARGURA_TELA / 2.0f, ALTURA_TELA / 2.0f }; // centro da tela
    camera.target = jogador;
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    int mapa[LINHAS][COLS];

    if (!CarregarMapa("mapa.txt", mapa)) return 1;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // Movimento do jogador
        Vector2 proximo = jogador;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) proximo.x += velocidade * dt;
        if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) proximo.x -= velocidade * dt;
        if (!ColideComAzul(mapa, proximo, 14.0f)) jogador.x = proximo.x;

        proximo = jogador;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) proximo.y += velocidade * dt;
        if (IsKeyDown(KEY_UP)   || IsKeyDown(KEY_W)) proximo.y -= velocidade * dt;
        if (!ColideComAzul(mapa, proximo, 14.0f)) jogador.y = proximo.y;

        // Limita o jogador dentro do mapa
        if (jogador.x < 0) jogador.x = 0;
        if (jogador.y < 0) jogador.y = 0;
        if (jogador.x > MAPA_LARGURA) jogador.x = MAPA_LARGURA;
        if (jogador.y > MAPA_ALTURA)  jogador.y = MAPA_ALTURA;

        // Direção e animação do personagem
        int andando = 0;
        if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) { direcao = 0; andando = 1; }
        if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) { direcao = 3; andando = 1; }
        if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) { direcao = 1; andando = 1; }
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) { direcao = 2; andando = 1; }

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

        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(camera);
                for (int y = 0; y < LINHAS; y++)
                {
                    for (int x = 0; x < COLS; x++)
                    {
                        Color cor;
                        switch (mapa[y][x])
                        {
                            case 0: cor = BLUE;      break;
                            case 1: cor = BEIGE;     break;
                            case 2: cor = GREEN;     break;
                            case 3: cor = DARKGRAY;  break;
                            case 4: cor = LIGHTGRAY; break;
                            case 5: cor = BROWN;     break;
                            case 6: cor = LIME;      break;
                            case 7: cor = BLACK;     break;
                            default: cor = BLACK;    break;
                        }
                        DrawRectangle(x * TILE, y * TILE, TILE, TILE, cor);
                    }
                }

                // Personagem (substitui a bola vermelha)
                Rectangle origem  = { frameAtual * (float)SPRITE_TAM, direcao * (float)SPRITE_TAM, SPRITE_TAM, SPRITE_TAM };
                Rectangle destino = { jogador.x, jogador.y, SPRITE_TAM, SPRITE_TAM };
                Vector2 centro = { SPRITE_TAM / 2.0f, SPRITE_TAM / 2.0f };
                DrawTexturePro(personagem, origem, destino, centro, 0.0f, WHITE);
            EndMode2D();

            DrawText("Setas ou WASD para mover", 10, 10, 20, WHITE);
            DrawFPS(10, 40);
        EndDrawing();
    }

    UnloadTexture(personagem);
    CloseWindow();
    return 0;
}
