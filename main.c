#include "raylib.h"
#include <stdio.h>
#include "entities.c"

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
#define CORACOES        5
#define VIDA_JOGADOR    (CORACOES * 2.0f)   // 10 pontos = 5 corações

// Arvores
#define MAX_ARVORES        300
#define DENSIDADE_ARVORES  8        // % de chance de cada tile 6 virar árvore
#define DIST_MIN_ARVORES   80.0f   // distância mínima entre árvores (px)
#define ESCALA_ARVORE      2.5f 
#define RAIO_TRONCO        12.0f

// Recortes no spritesheet (arvore.png, 74x62)
#define RECORTE_TOCO   (Rectangle){ 1, 48, 22, 14 }
#define RECORTE_ARVORE (Rectangle){ 34, 0, 39, 61 }

#define DANO_AGUA     1.5f       // pontos por segundo (meio coração/s)

static int ColideComParede(int mapa[LINHAS][COLS], Vector2 centro, float raio)
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
            if (x >= 0 && x < COLS && y >= 0 && y < LINHAS && (mapa[y][x] == 0 || mapa[y][x] == 2 || mapa[y][x] == 1 || mapa[y][x] == 6)) continue;

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

static int ColideComArvore(Vector2 atual, Vector2 proximo, float raio, int numArvores, Recursos arvores[])
{
    for (int i = 0; i < numArvores; i++)
    {

        // centro do tronco: um pouco acima da base da árvore
        Vector2 c = { arvores[i].posicao.x, arvores[i].posicao.y - 20.0f };
        float limite = raio + RAIO_TRONCO;

        float dxN = proximo.x - c.x, dyN = proximo.y - c.y;
        float dxA = atual.x   - c.x, dyA = atual.y   - c.y;
        float distNova  = dxN * dxN + dyN * dyN;
        float distAtual = dxA * dxA + dyA * dyA;

        // bloqueia só se o passo entra no tronco E aproxima do centro.
        // Isso evita o jogador ficar preso se uma árvore renascer em cima dele.
        if (distNova < limite * limite && distNova < distAtual) return 1;
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
static int TileEm(int mapa[LINHAS][COLS], Vector2 pos)
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
        int y = 420;

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

int main(void)
{
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Mapa maior que a tela");
    SetTargetFPS(60);

    Vector2 jogador = { MAPA_LARGURA / 2.0f, MAPA_ALTURA / 2.0f };
    float velocidade = 300.0f;
    float vida = VIDA_JOGADOR;
    Vector2 inicio = jogador;     // ponto de renascimento

    // --- Sprite do personagem (carregar depois do InitWindow) ---
    Texture2D personagem = LoadTexture("pictures/personagem.png");
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
    
    Texture2D texArvore = LoadTexture("pictures/arvore.png");
    SetTextureFilter(texArvore, TEXTURE_FILTER_POINT);

    Recursos arvores[MAX_ARVORES];
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
            for (int i = 0; i < numArvores; i++)
            {
                dx = pos.x - arvores[i].posicao.x;
                dy = pos.y - arvores[i].posicao.y;
                if (dx * dx + dy * dy < DIST_MIN_ARVORES * DIST_MIN_ARVORES) { perto = 1; break; }
            }
            if (perto) continue;

            arvores[numArvores++] = (Recursos){
                .posicao = pos,
                .vida = 3,
                .vida_maxima = 3,
                .quebrado = false,
                .tempo_renascimento = 0.0f,
                .cooldown_renascimento = 10.0f
            };
        }
    }

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // Movimento do jogador
        Vector2 proximo = jogador;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) proximo.x += velocidade * dt;
        if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) proximo.x -= velocidade * dt;
        if (!ColideComParede(mapa, proximo, 14.0f) && !ColideComArvore(jogador, proximo, 14.0f, numArvores, arvores)) jogador.x = proximo.x;

        proximo = jogador;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) proximo.y += velocidade * dt;
        if (IsKeyDown(KEY_UP)   || IsKeyDown(KEY_W)) proximo.y -= velocidade * dt;
        if (!ColideComParede(mapa, proximo, 14.0f) && !ColideComArvore(jogador, proximo, 14.0f, numArvores, arvores)) jogador.y = proximo.y;

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

        atualiza_recurso(arvores, numArvores);

        if (IsKeyPressed(KEY_E))
        {
            for (int i = 0; i < numArvores; i++)
            {
                if (arvores[i].quebrado) continue;

                float dx = jogador.x - arvores[i].posicao.x;
                float dy = jogador.y - arvores[i].posicao.y;
                if (dx * dx + dy * dy < 50.0f * 50.0f)   // alcance de 50 px
                {
                    quebrar_recurso(&arvores[i], 1);
                    if (arvores[i].quebrado) madeira++;   // ganha madeira ao derrubar
                    break;                                // só uma árvore por golpe
                }
            }
        }

        // Dano contínuo na água (testa o "pé" do personagem)
        int naAgua = (TileEm(mapa, (Vector2){ jogador.x, jogador.y + 12.0f }) == 0);
        if (naAgua) vida -= DANO_AGUA * dt;
        if (!naAgua && vida < VIDA_JOGADOR) vida += 0.5f * dt;

        // Morreu: volta ao início com a vida cheia
        if (vida <= 0.0f)
        {
            jogador = inicio;
            vida = VIDA_JOGADOR;
        }

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
                            case 6: cor = (Color){ 20, 140, 60, 255 }; break;
                            case 7: cor = BLACK;     break;
                            default: cor = BLACK;    break;
                        }
                        DrawRectangle(x * TILE, y * TILE, TILE, TILE, cor);
                    }
                }
                for (int i = 0; i < numArvores; i++)
                    if (arvores[i].posicao.y <= jogador.y) DesenhaArvore(texArvore, &arvores[i]);

                // Personagem (substitui a bola vermelha)
                Rectangle origem  = { frameAtual * (float)SPRITE_TAM, direcao * (float)SPRITE_TAM, SPRITE_TAM, SPRITE_TAM };
                Rectangle destino = { jogador.x, jogador.y, SPRITE_TAM, SPRITE_TAM };
                Vector2 centro = { SPRITE_TAM / 2.0f, SPRITE_TAM / 2.0f };
                DrawTexturePro(personagem, origem, destino, centro, 0.0f, WHITE);
                
                for (int i = 0; i < numArvores; i++)
                    if (arvores[i].posicao.y > jogador.y) DesenhaArvore(texArvore, &arvores[i]);

                
            EndMode2D();

            // Tela levemente vermelha enquanto toma dano
            if (naAgua) DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, Fade(RED, 0.2f));

            DesenhaVida(vida);

            DrawText(TextFormat("Madeira: %d", madeira), 10, 70, 20, WHITE);
            DrawText("Setas ou WASD para mover", 10, 10, 20, WHITE);
            DrawFPS(10, 40);
        EndDrawing();
    }

    UnloadTexture(personagem);
    UnloadTexture(texArvore);
    CloseWindow();
    return 0;
}
