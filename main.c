#include "raylib.h"
#include <stdio.h>
#include "entities.c"
#include "crafting.c"
#include "math.h"
#include "time.h"
#include "raymath.h"
#include "ranking.c"
// Ao modificar o codigo, o projeto deve ser recompilado para que as alterações tenham efeito.
// comando pra fazer isso: gcc main.c -o jogo.exe -I C:\raylib\include -L C:\raylib\lib -lraylib -lopengl32 -lgdi32 -lwinmm
// certifique-se de que o jogo previamente aberto foi fechado antes de recompilar.
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

#define CASA_SPAWN_X    (52 * TILE + TILE / 2.0f)
#define CASA_SPAWN_Y    (38 * TILE + TILE / 2.0f)
#define CAVERNA_SPAWN_X (94 * TILE + TILE / 2.0f)
#define CAVERNA_SPAWN_Y (71 * TILE + TILE / 2.0f)

// Sprite do personagem (spritesheet 4x4, frames de 32x32)
#define SPRITE_TAM      32
#define SPRITE_FRAMES   4
#define SPRITE_VEL_ANIM 0.12f   // segundos por frame
#define CORACOES        5
#define VIDA_JOGADOR    (CORACOES * 2.0f)   // 10 pontos = 5 corações

// Arvores
#define MAX_ARVORES        300
#define DENSIDADE_ARVORES  8        // % de chance de cada tile 6 virar árvore
#define DIST_MIN_ARVORES   100.0f   // distância mínima entre árvores (px)
#define ESCALA_ARVORE      2.5f 
#define RAIO_TRONCO        12.0f

#define MAX_PEDRAS        100
#define DENSIDADE_PEDRAS  5        // % de chance de cada tile 9 virar pedra
#define DIST_MIN_MINERIO  80.0f   // distância mínima entre minérios (px)
#define MAX_FERRO          50
#define DENSIDADE_FERRO     4
#define MAX_OURO          25
#define DENSIDADE_OURO     3
#define MAX_DIAMANTE          10
#define DENSIDADE_DIAMANTE     2
#define ESCALA_MINERIO      2.0f
#define RAIO_MINERIO        12.0f

// Recortes no spritesheet (arvore.png, 74x62)
#define RECORTE_TOCO   (Rectangle){ 1, 48, 22, 14 }
#define RECORTE_ARVORE (Rectangle){ 34, 0, 39, 61 }
#define RECORTE_PEDREGULHO (Rectangle){ 1, 50, 10, 7 }
#define RECORTE_MINERIO    (Rectangle){ 38, 32, 35, 25 }

#define DANO_AGUA     1.5f       // pontos por segundo (meio coração/s)
#define MAX_MONSTROS 100

// Ciclo dia/noite
#define HORA_INICIO      6.0f    // o jogo começa às 06:00
#define ESCURIDAO_MAXIMA 0.75f   // opacidade máxima da noite (0 = sem escuro, 1 = preto)

// Telas do jogo
#define DURACAO_TELA_MORTE 4.0f  // segundos na tela de morte antes de voltar ao menu
#define NUM_ESTRELAS       90

#define ARQUIVO_RECORDE "recorde.txt"

#define LAVA_CUSTO 5

int lavaAberta = 0;
char mensagemLava[64] = "";
float tempoMensagemLava = 0.0f;
Color corMensagemLava = WHITE;
    
typedef enum {
    TELA_INICIO = 0,
    TELA_JOGO,
    TELA_MORTE
} TelaJogo;

typedef enum {
    LOCAL_ILHA,
    LOCAL_CASA,
    LOCAL_CAVERNA
} LocalAtual;

Monstro monstros[MAX_MONSTROS] = {0};
int quantidadeMonstros = 0;



float timerSpawn = 0.0f;
float intervaloSpawn = 5.0f; // um monstro a cada 2 segundos
static int CarregarRecorde(void){
    FILE *f = fopen(ARQUIVO_RECORDE, "r");
    if (f == NULL) return 0;

    int record = 0;
    if(fscanf(f, "%d", &record) != 1) record = 0;
    fclose(f);
    return record;
}

static void SalvarRecorde(int record){
    FILE *f = fopen(ARQUIVO_RECORDE, "w");
    if (f == NULL) return;

    fprintf(f, "%d\n", record);
    fclose(f);
}

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
            // Somente os tiles andaveís, fora do mapa também bloqueia.
            if (x >= 0 && x < COLS && y >= 0 && y < LINHAS && (mapa[y][x] == 0 || mapa[y][x] == 6 || mapa[y][x] == 7 || mapa[y][x] == 8 || mapa[y][x] == 9 || mapa[y][x] == 'f' || mapa[y][x] == 'd' || mapa[y][x] == 'e')) continue;

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
        if (recurso->quebrado && !recurso->colide_quebrado) continue;
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
    Recursos *arvores,
    int dia
) {
    int slot = -1;

    // Procura um espaço ocupado por um monstro morto
    for (int i = 0; i < *quantidadeMonstros; i++) {
        if (monstros[i].morto) {
            slot = i;
            break;
        }
    }

    // Se não encontrou um monstro morto, usa um novo espaço
    if (slot == -1) {
        if (*quantidadeMonstros >= MAX_MONSTROS) {
            return;
        }

        slot = *quantidadeMonstros;
    }
    
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
        if (ColideComParede(mapa, pos, 18.0f)) continue;

        // Não permite que monstros nasçam sobre recursos.
        if (SobrepoeRecurso(pos, 18.0f, arvores)) continue;

        // Probabilidades de spawn mudam conforme os dias
        // (slime caranguejo e esqueleto comecam em 60% 25% 15% e vão até 20% 45% 35%)
        int progresso = dia - 1;

        // Limita a progressao ao dia 21
        if (progresso > 20) {
            progresso = 20;
        }

        int chanceSlime = 60 - 2 * progresso;
        int chanceEsqueleto = 25 + progresso;

        // caranguejo recebe a probabilidade restante
        int sorteio = GetRandomValue(1, 100);
        TipoMonstro tipo;

        if (sorteio <= chanceSlime) {
            tipo = MONSTRO_SLIME;
        } else if (sorteio <= chanceSlime + chanceEsqueleto) {
            tipo = MONSTRO_ESQUELETO;
        } else {
            tipo = MONSTRO_CARANGUEJO;
        }

        // Define os atributos conforme o tipo do monstro
        float vida = 10.0f;
        float ataque = 3.0f;
        float defesa = 0.0f;
        float velocidade = 100.0f;
        const char *nome = "Slime";

        switch (tipo) {
            case MONSTRO_ESQUELETO:
                vida = 25.0f;
                ataque = 6.0f;
                defesa = 2.0f;
                velocidade = 70.0f;
                nome = "Esqueleto";
                break;

            case MONSTRO_CARANGUEJO:
                vida = 7.0f;
                ataque = 2.0f;
                defesa = 0.0f;
                velocidade = 160.0f;
                nome = "Caranguejo";
                break;

            case MONSTRO_SLIME:
            default:
                break;
        }

        // Escala atributos conforme o dia atual
        float multiplicadorVida = 1.0f + 0.10f * (dia - 1);
        float multiplicadorAtaque = 1.0f + 0.05f * (dia - 1);

        vida *= multiplicadorVida;
        ataque *= multiplicadorAtaque;

        // Chance de elite aumenta com os dias, ate 30%
        int chanceElite = 5 + 2 * (dia - 1);
        if (chanceElite > 30) chanceElite = 30;

        bool elite = GetRandomValue(1, 100) <= chanceElite;

        // Modificadores dos monstros elite
        if (elite) {
            vida *= 2.0f;
            ataque *= 1.5f;
            velocidade *= 0.8f;
        }

        // Cria o monstro com os atributos definidos
        monstros[slot] = (Monstro){
            .tipo = tipo,
            .posicao = pos,
            .velocidade = velocidade,
            .elite = elite,
            .vida = vida,
            .vida_maxima = vida,
            .ataque = ataque,
            .defesa = defesa,
            .nome = nome,
            .morto = false
        };

        // Incrementa somente se utilizamos um slot novo
        if (slot == *quantidadeMonstros) {
            (*quantidadeMonstros)++;
        }

        return;
    }
}

// Retorna 1 se existe algum tile 'tile' a menos de 'alcance' px do jogador
static int PertoDeTile(char mapa[LINHAS][COLS], Vector2 pos, char tile, float alcance)
{
    int minX = (int)((pos.x - alcance) / TILE);
    int maxX = (int)((pos.x + alcance) / TILE);
    int minY = (int)((pos.y - alcance) / TILE);
    int maxY = (int)((pos.y + alcance) / TILE);

    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            if (x < 0 || x >= COLS || y < 0 || y >= LINHAS) continue;
            if (mapa[y][x] != tile) continue;

            float esq = x * TILE, topo = y * TILE;
            float px = pos.x < esq  ? esq  : (pos.x > esq + TILE  ? esq + TILE  : pos.x);
            float py = pos.y < topo ? topo : (pos.y > topo + TILE ? topo + TILE : pos.y);
            float dx = pos.x - px, dy = pos.y - py;
            if (dx * dx + dy * dy <= alcance * alcance) return 1;
        }
    }
    return 0;
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

static void DesenhaMinerio(Texture2D tex, Recursos *r)
{
    Rectangle src = r->quebrado ? RECORTE_PEDREGULHO : RECORTE_MINERIO;

    float w = src.width  * ESCALA_MINERIO;
    float h = src.height * ESCALA_MINERIO;

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

    float raio = monstro->elite ? 18.0f : 12.0f;

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

bool AtingidoPorSlash(
    Vector2 jogador,
    Vector2 alvo,
    Vector2 direcaoSlash,
    float alcance,
    float angulo,
    float raioAlvo
) {
    Vector2 diferenca = Vector2Subtract(alvo, jogador);
    float distancia = Vector2Length(diferenca);

    // Fora do alcance
    if (distancia > alcance + raioAlvo) return false;

    // Muito próximo: sempre atingido
    if (distancia < 0.001f) return true;

    Vector2 direcaoAlvo = Vector2Normalize(diferenca);

    float produtoEscalar = Vector2DotProduct(
        direcaoSlash,
        direcaoAlvo
    );

    float limiteAngulo = cosf((angulo / 2.0f) * DEG2RAD);

    return produtoEscalar >= limiteAngulo;
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

static void DesenhaTelaInicio(float tempo, Texture2D texturaPersonagem, int recorde)
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

    // Recorde / primeira partida.
    char linhaRecorde[64];
    if (recorde > 0)
        snprintf(linhaRecorde, sizeof(linhaRecorde), "melhor: dia %d", recorde);
    else
        snprintf(linhaRecorde, sizeof(linhaRecorde), "primeira noite");

    DesenhaTextoComSombra(linhaRecorde, 54.0f, 378.0f, 10.0f, 2.0f,
                          (Color){ 132, 119, 96, 255 });

    DesenhaTextoComSombra("WASD mover     F interagir     TAB inventário",
                          54.0f, 400.0f, 10.0f, 2.0f,
                          (Color){ 124, 118, 110, 255 });
    DesenhaTextoComSombra("C crafting     F11 tela cheia     ESC sair",
                          54.0f, 416.0f, 10.0f, 2.0f,
                          (Color){ 124, 118, 110, 255 });
}

static void DesenhaTelaMorte(float tempoMorte, float tempoRestante, int dia, int recorde, int novoRecorde)
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
    if (novoRecorde)
    {
        DesenhaTextoCentralizado("NOVO RECORDE!", 290.0f, 15.0f, 2.0f,
                                 Fade((Color){ 255, 205, 60, 255 }, (0.6f + 0.4f * pulso) * entrada));
    }
    else
    {
        char textoRecorde[64];
        snprintf(textoRecorde, sizeof(textoRecorde), "RECORDE: %d", recorde);
        DesenhaTextoCentralizado(textoRecorde, 290.0f, 15.0f, 2.0f,
                                 Fade((Color){ 200, 170, 90, 255 }, entrada));
    }
}

static Rectangle AjustarTela(void)
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

static void ApresentarTela(RenderTexture2D tela, Rectangle destino)
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
char nomeDigitado[32] = "\0";
int contadorLetras = 0;
int main(void)
{
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Scary Nights");
    SetTargetFPS(60);


    Itens_Init();
    EntradaRanking ranking[MAX_RANKING];
    CarregarRanking(ranking);
    IniciarEstrelas();

    Vector2 jogador = { MAPA_LARGURA / 2.0f, MAPA_ALTURA / 2.0f };
    float velocidade = 300.0f;
    float vida = VIDA_JOGADOR;
    Vector2 inicio = jogador;     // ponto de renascimento

    float defesaJogador = 0.0f;
    float cooldownDano = 0.0f;
    float cooldownEscudo = 0.0f;
    const float intervaloDano = 1.0f;

    Inventario inv = {0};
    int invAberto = 0;
    int origemInv = -1;
    const int invX = (LARGURA_TELA - INV_LARGURA) / 2;
    const int invY = (ALTURA_TELA - INV_ALTURA) / 2;

    // Escudo equipado separadamente da ferramenta principal
    Slot escudoEquipado = {0};

    /*
    // Kit inicial para testes
    Inv_AddFerramenta(&inv, ITEM_ESPADA, TIER_PEDRA);
    Inv_AddFerramenta(&inv, ITEM_ESPADA, TIER_FERRO);
    Inv_AddFerramenta(&inv, ITEM_ESCUDO, TIER_PEDRA);
    Inv_AddFerramenta(&inv, ITEM_ESCUDO, TIER_MAGNETITA);
    Inv_AddFerramenta(&inv, ITEM_PICARETA, TIER_OURO);
    Inv_AddFerramenta(&inv, ITEM_MACHADO, TIER_DIAMANTE);
    Inv_AddFerramenta(&inv, ITEM_ESPADA, TIER_MAGNETITA);
    */

    // Inicializa a ferramenta equipada
    Ferramenta ferramentaAtual = ObterFerramenta(
        inv.slots[inv.selecionado]
    );
    // Ferramenta usada na animacao atual (por que pode ser que a ferramenta quebre ou o jogador mude o slot)
    Ferramenta ferramentaAnimacao = ferramentaAtual;
    
    float cooldownAtaque = 0.0f;
    float timerSlash = 0.0f;
    float duracaoSlash = 0.15f;


    Vector2 direcaoSlash = {1.0f, 0.0f};

    

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
    int totalReceitas = (int)(sizeof(receitasCraft) / sizeof(receitasCraft[0]));
    int receitaSelecionada = 0;
    int craftingAberto = 0;
    char mensagemCraft[64] = "";
    char aviso[96] = "";
    float tempoAviso = 0.0f;
    Color corAviso = WHITE;
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

    Texture2D texPedra = LoadTexture("pictures/pedra_bruta.png");
    SetTextureFilter(texPedra, TEXTURE_FILTER_POINT);

    Texture2D texFerro = LoadTexture("pictures/ferro_bruto.png");
    SetTextureFilter(texFerro, TEXTURE_FILTER_POINT);

    Texture2D texOuro = LoadTexture("pictures/ouro_bruto.png");
    SetTextureFilter(texOuro, TEXTURE_FILTER_POINT);

    Texture2D texDiamante = LoadTexture("pictures/diamante_bruto.png");
    SetTextureFilter(texDiamante, TEXTURE_FILTER_POINT);

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
    // Uma imagem por local (índice = LocalAtual). Se o arquivo não existir, desenha os tiles.
    Texture2D texMapas[3];
    texMapas[LOCAL_ILHA]    = LoadTexture("pictures/ilha.png");
    texMapas[LOCAL_CASA]    = LoadTexture("pictures/casa.png");
    texMapas[LOCAL_CAVERNA] = LoadTexture("pictures/caverna.png");
    for (int i = 0; i < 3; i++)
        if (texMapas[i].id != 0) SetTextureFilter(texMapas[i], TEXTURE_FILTER_POINT);

    bool debug = false;   // F1 mostra a colisão do txt por cima

    Texture2D texturaMonstro = LoadTexture("pictures/slime.png");
    // Texturas temporarias dos diferentes monstros
    Texture2D texturaSlime = texturaMonstro;
    Texture2D texturaEsqueleto = LoadTexture("pictures/esqueleto.png");
    Texture2D texturaCaranguejo = LoadTexture("pictures/caranguejo.png");

    Recursos *arvores = NULL;
    Recursos *pedras = NULL;
    Recursos *ferros = NULL;
    Recursos *ouros = NULL;
    Recursos *diamantes = NULL;
    int numPedras = 0;
    int pedra = 0;
    int numFerro = 0;
    int ferro = 0;
    int numOuro = 0;
    int ouro = 0;
    int numDiamante = 0;
    int diamante = 0;
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
                .colide_quebrado = true,
                .tempo_renascimento = 0.0f,
                .cooldown_renascimento = 10.0f
            });
            numArvores++;
        }
    }

    char mapaCaverna[LINHAS][COLS];
    if (!CarregarMapa(MAPA_CAVERNA, mapaCaverna)) return 1;

    for (int ty = 0; ty < LINHAS && numPedras < MAX_PEDRAS; ty++)
    {
        for (int tx = 0; tx < COLS && numPedras < MAX_PEDRAS; tx++)
        {
            switch (mapaCaverna[ty][tx]) {
                case 9:
                case 'd':
                case 'f':
                    break;          // tile permitido, segue para o sorteio
                default:
                    continue;       // qualquer outro, pula
            }
            if (GetRandomValue(0, 99) >= DENSIDADE_PEDRAS) continue;

            Vector2 pos = { tx * TILE + TILE / 2.0f, ty * TILE + TILE };

            // não nasce em cima do ponto de entrada da caverna
            float dx = pos.x - CAVERNA_SPAWN_X, dy = pos.y - CAVERNA_SPAWN_Y;
            if (dx * dx + dy * dy < 64.0f * 64.0f) continue;

            if (TemRecursoPerto(pos, DIST_MIN_MINERIO, pedras)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, ferros)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, ouros)    ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, diamantes)) continue;

            inserir_recurso(&pedras, (Recursos){
                .posicao = pos,
                .offset_colisao = { 0.0f, -20.0f },
                .raio_colisao = RAIO_MINERIO,
                .vida = 5,
                .vida_maxima = 5,
                .quebrado = false,
                .tempo_renascimento = 0.0f,
                .cooldown_renascimento = 30.0f
            });
            numPedras++;
        }
    }

    for (int ty = 0; ty < LINHAS && numFerro < MAX_FERRO; ty++)
    {
        for (int tx = 0; tx < COLS && numFerro < MAX_FERRO; tx++)
        {
            switch (mapaCaverna[ty][tx]) {
                case 'd':
                case 'f':
                case 'e':
                    break;          // tile permitido, segue para o sorteio
                default:
                    continue;       // qualquer outro, pula
            }
            if (GetRandomValue(0, 99) >= DENSIDADE_FERRO) continue;

            Vector2 pos = { tx * TILE + TILE / 2.0f, ty * TILE + TILE };

            // não nasce em cima do ponto de entrada da caverna
            float dx = pos.x - CAVERNA_SPAWN_X, dy = pos.y - CAVERNA_SPAWN_Y;
            if (dx * dx + dy * dy < 64.0f * 64.0f) continue;

            if (TemRecursoPerto(pos, DIST_MIN_MINERIO, pedras)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, ferros)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, ouros)    ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, diamantes)) continue;

            inserir_recurso(&ferros, (Recursos){
                .posicao = pos,
                .offset_colisao = { 0.0f, -20.0f },
                .raio_colisao = RAIO_MINERIO,
                .vida = 7,
                .vida_maxima = 7,
                .quebrado = false,
                .tempo_renascimento = 0.0f,
                .cooldown_renascimento = 35.0f
            });
            numFerro++;
        }
    }

    for (int ty = 0; ty < LINHAS && numOuro < MAX_OURO; ty++)
    {
        for (int tx = 0; tx < COLS && numOuro < MAX_OURO; tx++)
        {
            switch (mapaCaverna[ty][tx]) {
                case 'f':
                case 'e':
                    break;          // tile permitido, segue para o sorteio
                default:
                    continue;       // qualquer outro, pula
            }
            if (GetRandomValue(0, 99) >= DENSIDADE_OURO) continue;

            Vector2 pos = { tx * TILE + TILE / 2.0f, ty * TILE + TILE };

            // não nasce em cima do ponto de entrada da caverna
            float dx = pos.x - CAVERNA_SPAWN_X, dy = pos.y - CAVERNA_SPAWN_Y;
            if (dx * dx + dy * dy < 64.0f * 64.0f) continue;

            if (TemRecursoPerto(pos, DIST_MIN_MINERIO, pedras)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, ferros)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, ouros)    ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, diamantes)) continue;
                
            inserir_recurso(&ouros, (Recursos){
                .posicao = pos,
                .offset_colisao = { 0.0f, -20.0f },
                .raio_colisao = RAIO_MINERIO,
                .vida = 9,
                .vida_maxima = 9,
                .quebrado = false,
                .tempo_renascimento = 0.0f,
                .cooldown_renascimento = 40.0f
            });
            numOuro++;
        }
    }  
    
    for (int ty = 0; ty < LINHAS && numDiamante < MAX_DIAMANTE; ty++)
    {
        for (int tx = 0; tx < COLS && numDiamante < MAX_DIAMANTE; tx++)
        {
            switch (mapaCaverna[ty][tx]) {
                case 'e':
                    break;          // tile permitido, segue para o sorteio
                default:
                    continue;       // qualquer outro, pula
            }
            if (GetRandomValue(0, 99) >= DENSIDADE_DIAMANTE) continue;

            Vector2 pos = { tx * TILE + TILE / 2.0f, ty * TILE + TILE };

            // não nasce em cima do ponto de entrada da caverna
            float dx = pos.x - CAVERNA_SPAWN_X, dy = pos.y - CAVERNA_SPAWN_Y;
            if (dx * dx + dy * dy < 64.0f * 64.0f) continue;

            if (TemRecursoPerto(pos, DIST_MIN_MINERIO, pedras)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, ferros)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, ouros)    ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, diamantes)) continue;

            inserir_recurso(&diamantes, (Recursos){
                .posicao = pos,
                .offset_colisao = { 0.0f, -20.0f },
                .raio_colisao = RAIO_MINERIO,
                .vida = 11,
                .vida_maxima = 11,
                .quebrado = false,
                .tempo_renascimento = 0.0f,
                .cooldown_renascimento = 45.0f
            });
            numDiamante++;
        }
    }

    //TO DO: garantir que tenha plo menos 1 de cada minerio
    
    int dentro = 0;                          // 0 = fora, 1 = dentro do local
    LocalAtual localAtual = LOCAL_ILHA;
    Vector2 posAntesDeEntrar = jogador;      // para voltar ao lugar certo ao sair
    Recursos *arvoresAtivas = arvores;       // árvores do mapa atual (NULL dentro de casa/caverna)
    Recursos *pedrasAtivas = NULL;         // pedras do mapa atual (NULL dentro de casa/ilha)
    Recursos *ferroAtivos = NULL;         // ferro do mapa atual (NULL dentro de casa/ilha)
    Recursos *ouroAtivos = NULL;         // ouro do mapa atual (NULL dentro de casa/ilha)
    Recursos *diamanteAtivos = NULL;         // diamante do mapa atual (NULL dentro de casa/ilha)

    int dia = 1;                             // contador de dias
    int recorde = CarregarRecorde();
    int novoRecorde = 0;

    TelaJogo telaAtual = TELA_INICIO;
    float tempoTela  = 0.0f;
    float tempoMorte = 0.0f;

    RenderTexture2D tela = LoadRenderTexture(LARGURA_TELA, ALTURA_TELA);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();
        Rectangle destinoTela = AjustarTela();

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

            BeginTextureMode(tela);
                ClearBackground(BLACK);
                DesenhaTelaInicio(tempoTela, personagem, recorde);
                DesenharRankingUI(ranking, 520, 170);
            ApresentarTela(tela, destinoTela);

            continue;
        }

// --- TELA DE MORTE ---
        else if (telaAtual == TELA_MORTE)
        {
            tempoMorte += dt;

            // 1. Captura as letras digitadas pelo jogador
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && key != ';' && (contadorLetras < 31)) {
                    nomeDigitado[contadorLetras] = (char)key;
                    nomeDigitado[contadorLetras + 1] = '\0';
                    contadorLetras++;
                }
                key = GetCharPressed();
            }

            // Apaga com o Backspace
            if (IsKeyPressed(KEY_BACKSPACE)) {
                contadorLetras--;
                if (contadorLetras < 0) contadorLetras = 0;
                nomeDigitado[contadorLetras] = '\0';
            }

            // 2. Condição para salvar no ranking e voltar ao menu (Aperta ENTER com nome ou acaba o tempo)
            if ((tempoMorte >= 1.0f && IsKeyPressed(KEY_ENTER) && contadorLetras > 0) ||
                tempoMorte >= DURACAO_TELA_MORTE)
            {
                // Se o tempo acabou sem digitar nome, salva como "Anônimo" ou o nome atual
                if (contadorLetras == 0) {
                    snprintf(nomeDigitado, sizeof(nomeDigitado), "Anonimo");
                }

                // Salva no ranking!
                AtualizarRanking(ranking, nomeDigitado, dia);

                // Reseta o nome para a próxima partida
                nomeDigitado[0] = '\0';
                contadorLetras = 0;

                // Reseta o mapa e as variáveis da partida...
                if (dentro && CarregarMapa(MAPA_ILHA, mapa))
                {
                    dentro = 0;
                    localAtual = LOCAL_ILHA;
                    arvoresAtivas = arvores;
                    pedrasAtivas = NULL;
                    ferroAtivos = NULL;
                    ouroAtivos = NULL;
                    diamanteAtivos = NULL;
                }

                jogador = inicio;
                vida = VIDA_JOGADOR;

                // Reinicia a run: volta ao dia 1 e perde todos os itens
                dia = 1;
                tempo_jogo = 0.0f;
                inv = (Inventario){0};
                escudoEquipado = (Slot){0};
                invAberto = 0;
                origemInv = -1;
                craftingAberto = 0;
                receitaSelecionada = 0;
                mensagemCraft[0] = '\0';
                tempoMensagemCraft = 0.0f;
                quantidadeMonstros = 0;

                timerSpawn = 0.0f;
                intervaloSpawn = 5.0f;
                defesaJogador = 0.0f;
                cooldownDano = 0.0f;
                cooldownEscudo = 0.0f;
                cooldownAtaque = 0.0f;
                timerSlash = 0.0f;
                tempoAviso = 0.0f;
                aviso[0] = '\0';
                frameAtual = 0;
                direcao = 0;
                timerAnim = 0.0f;
                camera.target = jogador;

                // Volta para a tela de início
                telaAtual = TELA_INICIO;
                tempoTela = 0.0f;
                tempoMorte = 0.0f;
            }

            // 3. Desenho da Tela de Morte e da Caixa de Texto
            BeginTextureMode(tela);
            ClearBackground(BLACK);
                
            // Desenha a tela de morte base
            DesenhaTelaMorte(tempoMorte, DURACAO_TELA_MORTE - tempoMorte, dia, recorde, novoRecorde);

            // Caixa de texto para o jogador digitar o nome
            DrawRectangle(240, 340, 300, 40, (Color){ 20, 10, 15, 255 });
            DrawRectangleLines(240, 340, 300, 40, RED);

            if (contadorLetras > 0) {
                DrawText(nomeDigitado, 250, 350, 20, WHITE);
            }else{
                DrawText("Digite seu nome e ENTER", 250, 352, 16, GRAY);
                }

            ApresentarTela(tela, destinoTela);

            continue;
        }

        if (tempoMensagemCraft > 0.0f) tempoMensagemCraft -= dt;
        if (tempoAviso > 0.0f) tempoAviso -= dt;

        // --- SISTEMA DE HORÁRIO ---
        // tempo_jogo conta os segundos do dia atual; ao completar o ciclo, começa outro dia.
        if (localAtual != LOCAL_CASA) tempo_jogo += dt;   // dentro da casa o relógio para
        if (tempo_jogo >= duracao_dia_noite)
        {
            tempo_jogo -= duracao_dia_noite;
            dia++;

            // Diminui o intervalo de spawn conforme os dias passam
            intervaloSpawn = 5.0f - 0.3f * (dia - 1);

            if (intervaloSpawn < 1.0f) {
                intervaloSpawn = 1.0f;
            }
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

        // Atualiza tempo de invulnerabilidade do jogador
        if (cooldownDano > 0.0f) {
            cooldownDano -= dt;

            if (cooldownDano < 0.0f) {
                cooldownDano = 0.0f;
            }
        }
        // Atualiza o tempo pra o escudo perder durabilidade
        if (cooldownEscudo > 0.0f){
            cooldownEscudo -= dt;
            if (cooldownEscudo < 0.0f) {
                cooldownEscudo = 0.0f;
            }
        }
        // Movimento do jogador (bloqueado enquanto o crafting está aberto)
        // A passagem estreita da caverna precisa de folga para o passo do jogador.
        float raioMovimento = localAtual == LOCAL_CAVERNA ? 12.0f : 14.0f;
        Vector2 proximo = jogador;
        if (!craftingAberto && !invAberto && !lavaAberta)
        {
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) proximo.x += velocidade * dt;
            if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) proximo.x -= velocidade * dt;
            if (!ColideComParede(mapa, proximo, raioMovimento) && !ColideComRecursos(jogador, proximo, raioMovimento, arvoresAtivas) && !ColideComRecursos(jogador, proximo, raioMovimento, pedrasAtivas) && !ColideComRecursos(jogador, proximo, raioMovimento, ferroAtivos) && !ColideComRecursos(jogador, proximo, raioMovimento, ouroAtivos) && !ColideComRecursos(jogador, proximo, raioMovimento, diamanteAtivos)) jogador.x = proximo.x;

            proximo = jogador;
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) proximo.y += velocidade * dt;
            if (IsKeyDown(KEY_UP)   || IsKeyDown(KEY_W)) proximo.y -= velocidade * dt;
            if (!ColideComParede(mapa, proximo, raioMovimento) && !ColideComRecursos(jogador, proximo, raioMovimento, arvoresAtivas) && !ColideComRecursos(jogador, proximo, raioMovimento, pedrasAtivas) && !ColideComRecursos(jogador, proximo, raioMovimento, ferroAtivos) && !ColideComRecursos(jogador, proximo, raioMovimento, ouroAtivos) && !ColideComRecursos(jogador, proximo, raioMovimento, diamanteAtivos)) jogador.y = proximo.y;
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
                    localAtual = LOCAL_CASA;
                }
            }
            else
            {
                if (CarregarMapa(MAPA_ILHA, mapa))
                {
                    dentro = 0;
                    jogador = posAntesDeEntrar;
                    localAtual = LOCAL_ILHA;
                }
            }
        }

        if(TileEm(mapa, jogador) == TILE_PORTA_CAVERNA)
        {
            if (!dentro)
            {
                if (CarregarMapa(MAPA_CAVERNA, mapa))
                {
                    dentro = 1;
                    jogador = (Vector2){ CAVERNA_SPAWN_X, CAVERNA_SPAWN_Y };
                    localAtual = LOCAL_CAVERNA;
                }
            }
            else
            {
                if (CarregarMapa(MAPA_ILHA, mapa))
                {
                    dentro = 0;
                    jogador = posAntesDeEntrar;
                    localAtual = LOCAL_ILHA;
                }
            }
        }

        arvoresAtivas = localAtual == LOCAL_ILHA ? arvores : NULL;
        pedrasAtivas = localAtual == LOCAL_CAVERNA ? pedras : NULL;
        ferroAtivos = localAtual == LOCAL_CAVERNA ? ferros : NULL;
        ouroAtivos = localAtual == LOCAL_CAVERNA ? ouros : NULL;
        diamanteAtivos = localAtual == LOCAL_CAVERNA ? diamantes : NULL;

        // Direção e animação do personagem
        int andando = 0;
        if (!craftingAberto && !invAberto && !lavaAberta)
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
        if(!invAberto && !craftingAberto && !lavaAberta) camera.zoom += GetMouseWheelMove() * 0.1f;
        if (camera.zoom < 0.5f) camera.zoom = 0.5f;
        if (camera.zoom > 3.0f) camera.zoom = 3.0f;

        // Limita a câmera para não mostrar fora do mapa
        float metadeL = (LARGURA_TELA / 2.0f) / camera.zoom;
        float metadeA = (ALTURA_TELA  / 2.0f) / camera.zoom;

        if (camera.target.x < metadeL) camera.target.x = metadeL;
        if (camera.target.y < metadeA) camera.target.y = metadeA;
        if (camera.target.x > MAPA_LARGURA - metadeL) camera.target.x = MAPA_LARGURA - metadeL;
        if (camera.target.y > MAPA_ALTURA  - metadeA) camera.target.y = MAPA_ALTURA  - metadeA;

        // Atualiza timers de ataque
        if (cooldownAtaque > 0.0f) {
            cooldownAtaque -= dt;
            if (cooldownAtaque < 0.0f) cooldownAtaque = 0.0f;
        }

        if (timerSlash > 0.0f) {
            timerSlash -= dt;
            if (timerSlash < 0.0f) timerSlash = 0.0f;
        }

        // Ataque do jogador
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            cooldownAtaque <= 0.0f &&
            !invAberto &&
            !craftingAberto &&
            !IsKeyPressed(KEY_TAB) &&
            timerSlash <= 0.0f) {

            Vector2 mouseMundo = GetScreenToWorld2D(
                GetMousePosition(),
                camera
            );

            Vector2 direcao = Vector2Subtract(mouseMundo, jogador);

            if (Vector2LengthSqr(direcao) > 0.001f) {
                direcaoSlash = Vector2Normalize(direcao);
            }

            cooldownAtaque = ferramentaAtual.intervaloAtaque;
            ferramentaAnimacao = ferramentaAtual;
            
            // Duracao visual depende da ferramenta usada
            duracaoSlash = ferramentaAnimacao.duracaoAnimacao;
            timerSlash = duracaoSlash;

            bool acertouAlvo = false;

            // Verifica todos os monstros
            if (localAtual != LOCAL_CASA) {
                for (int i = 0; i < quantidadeMonstros; i++) {
                    if (monstros[i].morto) continue;

                    if (AtingidoPorSlash(
                        jogador,
                        monstros[i].posicao,
                        direcaoSlash,
                        ferramentaAtual.alcanceSlash,
                        ferramentaAtual.anguloSlash,
                        monstros[i].elite ? 18.0f : 12.0f
                    )) {

                        float dano = ferramentaAtual.dano - monstros[i].defesa;
                        if (dano < 0.0f) dano = 0.0f;

                        monstros[i].vida -= dano;
                        acertouAlvo = true;

                        if (monstros[i].vida <= 0.0f) {
                            monstros[i].vida = 0.0f;
                            monstros[i].morto = true;
                        }
                    }
                }
            }// Corte de arvores com o slash
                if (ferramentaAtual.poderCorte > 0) {
    
                    for (Recursos *a = arvoresAtivas; a != NULL; a = a->proximo) {
    
                        if (a->quebrado) continue;
    
                        // Usa o centro de colisao do tronco
                        Vector2 centroArvore = Vector2Add(
                            a->posicao,
                            a->offset_colisao
                        );
    
                        if (AtingidoPorSlash(
                            jogador,
                            centroArvore,
                            direcaoSlash,
                            ferramentaAtual.alcanceSlash,
                            ferramentaAtual.anguloSlash,
                            a->raio_colisao
                        )) {
    
                            quebrar_recurso(a, ferramentaAtual.poderCorte);
                            acertouAlvo = true;
    
                            // Mantem a recompensa existente
                            if (a->quebrado) {
                                Inv_Add(&inv, ITEM_MADEIRA, 1);
                            }
                        }
                    }
                }
                if (ferramentaAtual.poderMineracao > 0) {
                    if (ferramentaAtual.tipo == ITEM_PICARETA){
                        for (Recursos *a = pedrasAtivas; a != NULL; a = a->proximo) {
        
                            if (a->quebrado) continue;
        
                            // Usa o centro de colisao do tronco
                            Vector2 centroPedra = Vector2Add(
                                a->posicao,
                                a->offset_colisao
                            );
        
                            if (AtingidoPorSlash(
                                jogador,
                                centroPedra,
                                direcaoSlash,
                                ferramentaAtual.alcanceSlash,
                                ferramentaAtual.anguloSlash,
                                a->raio_colisao
                            )) {
        
                                quebrar_recurso(a, ferramentaAtual.poderMineracao);
                                acertouAlvo = true;
        
                                // Mantem a recompensa existente
                                if (a->quebrado) {
                                    Inv_Add(&inv, ITEM_PEDRA, 1);
                                }
                            }
                        }
                    }
                }
                if (ferramentaAtual.poderMineracao > 0) {
                    if (ferramentaAtual.tipo == ITEM_PICARETA){
                        for (Recursos *a = ferroAtivos; a != NULL; a = a->proximo) {
        
                            if (a->quebrado) continue;
        
                            // Usa o centro de colisao do tronco
                            Vector2 centroFerro = Vector2Add(
                                a->posicao,
                                a->offset_colisao
                            );
        
                            if (AtingidoPorSlash(
                                jogador,
                                centroFerro,
                                direcaoSlash,
                                ferramentaAtual.alcanceSlash,
                                ferramentaAtual.anguloSlash,
                                a->raio_colisao
                            )) {
        
                                quebrar_recurso(a, ferramentaAtual.poderMineracao);
                                acertouAlvo = true;
        
                                // Mantem a recompensa existente
                                if (a->quebrado) {
                                    Inv_Add(&inv, ITEM_FERRO, 1);
                                }
                            }
                        }
                    }
                }
                if (ferramentaAtual.poderMineracao > 0) {
                    if (ferramentaAtual.tipo == ITEM_PICARETA){
                        for (Recursos *a = ouroAtivos; a != NULL; a = a->proximo) {
        
                            if (a->quebrado) continue;
        
                            // Usa o centro de colisao do tronco
                            Vector2 centroOuro = Vector2Add(
                                a->posicao,
                                a->offset_colisao
                            );
        
                            if (AtingidoPorSlash(
                                jogador,
                                centroOuro,
                                direcaoSlash,
                                ferramentaAtual.alcanceSlash,
                                ferramentaAtual.anguloSlash,
                                a->raio_colisao
                            )) {
        
                                quebrar_recurso(a, ferramentaAtual.poderMineracao);
                                acertouAlvo = true;
        
                                // Mantem a recompensa existente
                                if (a->quebrado) {
                                    Inv_Add(&inv, ITEM_OURO, 1);
                                }
                            }
                        }
                    }
                }
                if (ferramentaAtual.poderMineracao > 0) {
                    if (ferramentaAtual.tipo == ITEM_PICARETA){
                        for (Recursos *a = diamanteAtivos; a != NULL; a = a->proximo) {
        
                            if (a->quebrado) continue;
        
                            // Usa o centro de colisao do tronco
                            Vector2 centroDiamante = Vector2Add(
                                a->posicao,
                                a->offset_colisao
                            );
        
                            if (AtingidoPorSlash(
                                jogador,
                                centroDiamante,
                                direcaoSlash,
                                ferramentaAtual.alcanceSlash,
                                ferramentaAtual.anguloSlash,
                                a->raio_colisao
                            )) {
        
                                quebrar_recurso(a, ferramentaAtual.poderMineracao);
                                acertouAlvo = true;
        
                                // Mantem a recompensa existente
                                if (a->quebrado) {
                                    Inv_Add(&inv, ITEM_DIAMANTE, 1);
                                }
                            }
                        }
                    }
                }
                // Consome durabilidade apenas quando acertar um alvo
                if (acertouAlvo) {
        
                    Slot *slotEquipado = &inv.slots[inv.selecionado];
        
                    // Apenas ferramentas possuem durabilidade
                    if (slotEquipado->tipo == ITEM_ESPADA ||
                        slotEquipado->tipo == ITEM_MACHADO ||
                        slotEquipado->tipo == ITEM_PICARETA ||
                        slotEquipado->tipo == ITEM_ESCUDO) {
        
                        if (slotEquipado->durability > 0) {
                            slotEquipado->durability--;
        
                            printf("Durabilidade restante: %d\n",
                                slotEquipado->durability);
                        }
        
                        // Ferramenta quebra quando chega a zero
                        if (slotEquipado->durability <= 0) {
                            printf("Ferramenta quebrou!\n");
        
                            *slotEquipado = (Slot){0};
        
                            // Atualiza imediatamente para ataque desarmado
                            ferramentaAtual = ObterFerramenta(*slotEquipado);
                        }
                    }
                }
            
        }
        
        if(localAtual != LOCAL_CASA){
            atualiza_recurso(arvores);
            atualiza_recurso(pedras);
            atualiza_recurso(ferros);
            atualiza_recurso(ouros);
            atualiza_recurso(diamantes);
        }

        // Abre/fecha a interface de crafting (fecha o inventário para não sobrepor)
        if (IsKeyPressed(KEY_C))
        {
            if (localAtual == LOCAL_CASA)
            {
                craftingAberto = !craftingAberto;
                if (craftingAberto) invAberto = 0;
            }
            else
            {
                snprintf(aviso, sizeof(aviso), "Voce precisa estar dentro da casa para abrir o crafting.");
                corAviso = RED;
                tempoAviso = 2.0f;   // fica 2 segundos na tela
            }
        }

        if (IsKeyPressed(KEY_TAB))
        {
            invAberto = !invAberto;
            if (invAberto) {
                craftingAberto = 0;
                lavaAberta = 0;
            }
        }
        if (!invAberto) origemInv = -1;
        if (tempoMensagemLava > 0.0f) tempoMensagemLava -= dt;

        int pertoLava = PertoDeTile(mapa, jogador, 'a', 48.0f);

        if (!pertoLava) lavaAberta = 0;   // se afastar, fecha

        if (pertoLava && IsKeyPressed(KEY_F) && !craftingAberto && !invAberto)
            lavaAberta = !lavaAberta;

        if (lavaAberta && IsKeyPressed(KEY_ENTER))
        {
            if (Inv_Contar(&inv, ITEM_DIAMANTE) < LAVA_CUSTO)
            {
                snprintf(mensagemLava, sizeof(mensagemLava), "Voce precisa de %d diamantes.", LAVA_CUSTO);
                corMensagemLava = RED;
            }
            else
            {
                Inv_Remover(&inv, ITEM_DIAMANTE, LAVA_CUSTO);

                if (Inv_Add(&inv, ITEM_MAGNETITA, 1) > 0)
                {
                    // Sem espaço: devolve os diamantes
                    Inv_Add(&inv, ITEM_DIAMANTE, LAVA_CUSTO);
                    snprintf(mensagemLava, sizeof(mensagemLava), "Inventario cheio!");
                    corMensagemLava = RED;
                }
                else
                {
                    snprintf(mensagemLava, sizeof(mensagemLava), "A lava aceitou! Voce recebeu 1 magnetita.");
                    corMensagemLava = GREEN;
                }
            }
            tempoMensagemLava = 2.0f;
        }

        if (invAberto && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int destino = Inv_SlotNoPonto(invX, invY, GetMousePosition());
            if (destino >= 0) {
                if (origemInv >= 0) {
                    if (origemInv != destino) Inv_TrocarSlots(&inv, origemInv, destino);
                    origemInv = -1;
                } else if (inv.slots[destino].tipo != ITEM_VAZIO) {
                    origemInv = destino;
                }
            }
        }

        // Clique direito descarta 1 item do slot. Com Shift, descarta o stack.
        if (invAberto && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            int alvo = Inv_SlotNoPonto(invX, invY, GetMousePosition());

            if (alvo >= 0 && inv.slots[alvo].tipo != ITEM_VAZIO) {
                Slot alvoSlot = inv.slots[alvo];
                int qtd = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
                          ? alvoSlot.quant : 1;
                int removidos = Inv_DescartarSlot(&inv, alvo, qtd);

                if (removidos > 0) {
                    if (origemInv == alvo) origemInv = -1;

                    if (removidos > 1)
                        snprintf(aviso, sizeof(aviso), "Descartado: %s x%d",
                                 itens[alvoSlot.tipo].nome, removidos);
                    else
                        snprintf(aviso, sizeof(aviso), "Descartado: %s",
                                 itens[alvoSlot.tipo].nome);

                    corAviso = (Color){ 214, 190, 150, 255 };
                    tempoAviso = 1.6f;
                }
            }
        }

        for (int k = 0; k < INV_COLS; k++) {
            if (!IsKeyPressed(KEY_ONE + k)) continue;
            if (origemInv >= 0) {
                if (origemInv != k) Inv_TrocarSlots(&inv, origemInv, k);
                origemInv = -1;
            } else {
                inv.selecionado = k;
            }
            break;
        }

        // Atualiza a ferramenta equipada conforme o slot selecionado
        ferramentaAtual = ObterFerramenta(
            inv.slots[inv.selecionado]
        );
        // Equipar ou desequipar escudo com H
        if (IsKeyPressed(KEY_H) && !craftingAberto && !invAberto && !lavaAberta) {

            Slot *selecionado = &inv.slots[inv.selecionado];

            if (selecionado->tipo == ITEM_ESCUDO) {

                // Troca o escudo selecionado pelo equipado
                Slot temporario = escudoEquipado;
                escudoEquipado = *selecionado;
                *selecionado = temporario;

            } else if (escudoEquipado.tipo == ITEM_ESCUDO &&
                    selecionado->tipo == ITEM_VAZIO) {

                // Devolve o escudo equipado ao slot vazio
                *selecionado = escudoEquipado;
                escudoEquipado = (Slot){0};
            }
        }
        
        defesaJogador = 0.0f;

        if (escudoEquipado.tipo == ITEM_ESCUDO &&
            escudoEquipado.tier >= 0 &&
            escudoEquipado.tier < TIER_COUNT) {

            // Valor inicial de balanceamento
            defesaJogador = 1.0f + escudoEquipado.tier;
        }

        // Dano contínuo na água (testa o "pé" do personagem)
        int naAgua = (TileEm(mapa, (Vector2){ jogador.x, jogador.y + 12.0f }) == 0);
        if (naAgua) vida -= DANO_AGUA * dt;
        if (!naAgua && vida < VIDA_JOGADOR) vida += 0.5f * dt;

        // Morreu: mostra a tela de morte antes de voltar ao menu inicial
        if (vida <= 0.0f)
        {
            vida = 0.0f;
            telaAtual = TELA_MORTE;
            tempoMorte = 0.0f;

            novoRecorde = (dia > recorde);
            if(novoRecorde) 
            {
                recorde = dia;
                SalvarRecorde(recorde);
            }
        }
        //logica de spawn de monstros
        if ((isNoite && localAtual != LOCAL_CASA) || localAtual == LOCAL_CAVERNA){
            timerSpawn += dt;

            if (timerSpawn >= intervaloSpawn) {
                timerSpawn = 0.0f;

                printf("Tentando spawnar monstro...\n");

                SpawnMonstro(
                    monstros,
                    &quantidadeMonstros,
                    jogador,
                    mapa,
                    arvoresAtivas,
                    dia
                );

                printf("Quantidade de monstros: %d\n", quantidadeMonstros);

            }
        }
        else {
            timerSpawn = 0.0f;
        }

        float danoSol = 2.0f;
        
        //atualiza monstros
        if (localAtual != LOCAL_CASA){
            for (int i = 0; i < quantidadeMonstros; i++) {
                if (monstros[i].morto) continue;
                
                MoverMonstro(
                    &monstros[i],
                    jogador,
                    dt,
                    mapa,
                    arvoresAtivas
                );

                // Dano por contato com monstro
                if (cooldownDano <= 0.0f &&
                    CheckCollisionCircles(
                        jogador, 14.0f,
                        monstros[i].posicao,
                        monstros[i].elite ? 18.0f : 12.0f
                    )) {

                    float dano = monstros[i].ataque - defesaJogador;

                    if (dano < 0.0f) {
                        dano = 0.0f;
                    }

                    // Desgasta o escudo quando ele bloqueia dano
                    if (escudoEquipado.tipo == ITEM_ESCUDO &&
                        monstros[i].ataque > 0.0f &&
                        defesaJogador > 0.0f &&
                        cooldownEscudo <= 0.0f){

                        if (escudoEquipado.durability > 0) {
                            escudoEquipado.durability--;
                            cooldownEscudo = intervaloDano;
                        }

                        if (escudoEquipado.durability <= 0) {
                            printf("Escudo quebrou!\n");

                            escudoEquipado = (Slot){0};
                            defesaJogador = 0.0f;
                        }
                    }

                    // Aplica dano e invencibilidade apenas se houver perda de vida
                    if (dano > 0.0f) {
                        vida -= dano;
                        cooldownDano = intervaloDano;
                    }

                    printf("Monstro atacou! Dano: %.1f | Vida: %.1f\n",
                        dano, vida);
                }
                
                //dano de sol
                if (!isNoite && localAtual != LOCAL_CAVERNA) {
                    monstros[i].vida -= danoSol * dt;

                    if (monstros[i].vida <= 0) {
                        monstros[i].vida = 0;
                        monstros[i].morto = true;
                    }
                }
            }
        }

        if (IsKeyPressed(KEY_F1)) debug = !debug;

        BeginTextureMode(tela);
            ClearBackground(BLACK);

            BeginMode2D(camera);

                Texture2D texAtual = texMapas[localAtual];
                bool temImagem = (texAtual.id != 0);

                if (temImagem)
                {
                    DrawTexturePro(texAtual,
                        (Rectangle){ 0, 0, (float)texAtual.width, (float)texAtual.height },
                        (Rectangle){ 0, 0, MAPA_LARGURA, MAPA_ALTURA },
                        (Vector2){ 0, 0 }, 0.0f, WHITE);
                }

                // Sem imagem: desenha os tiles normalmente. Com imagem: só no modo debug, transparente.
                if (debug || !temImagem)
                {
                    float alpha = temImagem ? 0.4f : 1.0f;

                    for (int y = 0; y < LINHAS; y++)
                    {
                        for (int x = 0; x < COLS; x++)
                        {
                            Color cor;
                            switch (mapa[y][x])
                            {
                                case 0: cor = BLUE;      break;
                                case 3: cor = DARKGRAY;  break;
                                case 4: cor = LIGHTGRAY; break;
                                case 6: cor = (Color){ 20, 140, 60, 255 }; break;
                                case 7: cor = BLACK;     break;
                                case 8: cor = (Color){ 100, 50, 0, 255 }; break;
                                case 9: cor = (Color){ 141, 73, 37, 255 }; break;
                                case 'a': cor = (Color){ 255, 255, 0, 255 }; break;
                                case 'd': cor = (Color){ 255, 128, 0, 255 }; break;
                                case 'e': cor = (Color){ 128, 0, 255, 255 }; break;
                                case 'f': cor = (Color){ 0, 255, 128, 255 }; break;
                                default: cor = BLACK;    break;
                            }
                            DrawRectangle(x * TILE, y * TILE, TILE, TILE, Fade(cor, alpha));
                        }
                    }
                }
                
                // 2. Y-SORTING PARTE 1: Desenha as árvores que estão ATRÁS do jogador
                for (Recursos *a = arvoresAtivas; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y <= jogador.y) DesenhaArvore(texArvore, a);
                }
                for (Recursos *p = pedrasAtivas; p != NULL; p = p->proximo)
                {
                    if (p->posicao.y <= jogador.y) DesenhaMinerio(texPedra, p);
                }
                for (Recursos *p = ferroAtivos; p != NULL; p = p->proximo)
                {
                    if (p->posicao.y <= jogador.y) DesenhaMinerio(texFerro, p);
                }
                for (Recursos *p = ouroAtivos; p != NULL; p = p->proximo)
                {
                    if (p->posicao.y <= jogador.y) DesenhaMinerio(texOuro, p);
                }
                for (Recursos *p = diamanteAtivos; p != NULL; p = p->proximo)
                {
                    if (p->posicao.y <= jogador.y) DesenhaMinerio(texDiamante, p);
                }

                // 3. DESENHO DO PERSONAGEM
                Rectangle origem  = { frameAtual * (float)SPRITE_TAM, direcao * (float)SPRITE_TAM, SPRITE_TAM, SPRITE_TAM };
                Rectangle destino = { jogador.x, jogador.y, SPRITE_TAM, SPRITE_TAM };
                Vector2 centro = { SPRITE_TAM / 2.0f, SPRITE_TAM / 2.0f };

                Color corJogador = cooldownDano > 0.0f ? RED : WHITE;
                DrawTexturePro(personagem, origem, destino, centro, 0.0f, corJogador);
                
                // 4. Y-SORTING PARTE 2: Desenha as árvores que estão na FRENTE do jogador
                // Renderiza Monstros
                for (int i = 0; localAtual != LOCAL_CASA && i < quantidadeMonstros; i++) {
                    if (monstros[i].morto) continue;

                    // Seleciona a textura conforme o tipo de monstro
                    Texture2D texturaAtual = texturaSlime;

                    switch (monstros[i].tipo) {
                        case MONSTRO_ESQUELETO:
                            texturaAtual = texturaEsqueleto;
                            break;
                        case MONSTRO_CARANGUEJO:
                            texturaAtual = texturaCaranguejo;
                            break;
                        case MONSTRO_SLIME:
                        default:
                            texturaAtual = texturaSlime;
                            break;
                    }

                    Rectangle origem = {
                        0, 0,
                        (float)texturaAtual.width,
                        (float)texturaAtual.height
                    };

                    // Monstros elite possuem sprites 50% maiores
                    float tamanho = monstros[i].elite ? 48.0f : 32.0f;

                    Rectangle destino = {
                        monstros[i].posicao.x,
                        monstros[i].posicao.y,
                        tamanho,
                        tamanho
                    };

                    Vector2 centro = {
                        tamanho / 2.0f,
                        tamanho / 2.0f
                    };

                    DrawTexturePro(
                        texturaAtual,
                        origem,
                        destino,
                        centro,
                        0.0f,
                        monstros[i].elite ? (Color){255, 190, 130, 255} : WHITE
                    );
                    // Barra de vida do monstro
                    float larguraBarra = 32.0f;
                    float alturaBarra = 5.0f;

                    float porcentagemVida = monstros[i].vida / monstros[i].vida_maxima;
                    porcentagemVida = Clamp(porcentagemVida, 0.0f, 1.0f);

                    float xBarra = monstros[i].posicao.x - larguraBarra / 2.0f;
                    float yBarra = monstros[i].posicao.y - 26.0f;

                    // Fundo da barra
                    DrawRectangle(
                        (int)xBarra,
                        (int)yBarra,
                        (int)larguraBarra,
                        (int)alturaBarra,
                        DARKGRAY
                    );

                    // Parte preenchida
                    DrawRectangle(
                        (int)xBarra,
                        (int)yBarra,
                        (int)(larguraBarra * porcentagemVida),
                        (int)alturaBarra,
                        RED
                    );

                    // Nome do monstro acima da barra de vida
                    const char *nomeMonstro = monstros[i].nome != NULL ? monstros[i].nome : "Monstro";

                    char nomeExibido[64];

                    snprintf(
                        nomeExibido,
                        sizeof(nomeExibido),
                        "%s%s",
                        monstros[i].elite ? "Elite " : "",
                        nomeMonstro
                    );

                    nomeMonstro = nomeExibido;
                    
                    int tamanhoFonte = 12;
                    int larguraNome = MeasureText(nomeMonstro, tamanhoFonte);

                    DrawText(
                        nomeMonstro,
                        (int)(monstros[i].posicao.x - larguraNome / 2),
                        (int)(monstros[i].posicao.y - 38),
                        tamanhoFonte,
                        WHITE
                    );
                }

                // Animacao visual do slash
                if (timerSlash > 0.0f) {

                    // Progresso da animacao: 0 = inicio, 1 = fim
                    float progresso = 1.0f - timerSlash / duracaoSlash;
                    progresso = Clamp(progresso, 0.0f, 1.0f);

                    float anguloCentral = atan2f(
                        direcaoSlash.y,
                        direcaoSlash.x
                    ) * RAD2DEG;

                    // Arco percorre o angulo do ataque
                    float anguloAtual = anguloCentral
                        - ferramentaAnimacao.anguloSlash / 2.0f
                        + ferramentaAnimacao.anguloSlash * progresso;

                    float larguraArco = ferramentaAnimacao.anguloSlash * 0.15f;
                    float raioExterno = ferramentaAnimacao.alcanceSlash; // * 0.65f;
                    float raioInterno = 14.0f; //raioExterno - 8.0f;

                    // Cor do slash acompanha o tier da ferramenta
                    Color corBase;

                    switch (ferramentaAnimacao.tier){
                        case TIER_FERRO:
                            corBase = (Color) {255, 255, 255, 150}; //branco
                            break;

                        case TIER_OURO:
                            corBase = (Color) {255, 234, 0, 200}; //amarelo mais transparente
                            break;

                        case TIER_DIAMANTE:
                            corBase = (Color) {0, 234, 255, 200}; //azul claro
                            break;

                        case TIER_MAGNETITA:
                            corBase = (Color) {150, 100, 255, 230}; //roxo //(Color) {255, 255, 0, 200}; //amarelo
                            break;

                        default:
                            
                            corBase = (Color) {255, 255, 255, 50}; //branco mais transparente
                    }

                    Color corSlash = Fade(
                        corBase,
                        (corBase.a / 255.0f) * (1.0f - progresso)
                    );

                    // Faixa curva do ataque
                    DrawRing(
                        jogador,
                        raioInterno,
                        raioExterno,
                        anguloAtual - larguraArco,
                        anguloAtual + larguraArco,
                        16,
                        corSlash
                    );

                    // Animacao da ferramenta equipada
                    if (ferramentaAnimacao.tipo == ITEM_ESPADA ||
                        ferramentaAnimacao.tipo == ITEM_MACHADO ||
                        ferramentaAnimacao.tipo == ITEM_PICARETA) {

                        Sprite sp = spritesFerramenta
                            [ferramentaAnimacao.tipo]
                            [ferramentaAnimacao.tier];

                        if (sp.textura.id != 0) {

                            // Mesmo angulo usado pelo efeito de slash
                            float anguloFerramenta = anguloAtual;

                            // Posiciona a ferramenta no meio do alcance do slash
                            float distancia = (raioInterno + raioExterno) / 2.0f;

                            float rad = anguloFerramenta * DEG2RAD;

                            Vector2 posFerramenta = {
                                jogador.x + cosf(rad) * distancia,
                                jogador.y + sinf(rad) * distancia
                            };

                            // Dimensoes visuais independentes do alcance do ataque
                            float tamanho = 40.0f;

                            Rectangle destinoFerramenta = {
                                posFerramenta.x,
                                posFerramenta.y,
                                tamanho,
                                tamanho
                            };

                            // Ponto de rotacao no centro inferior da imagem
                            Vector2 origemFerramenta = {
                                tamanho / 2.0f,
                                tamanho / 2.0f
                            };

                            DrawTexturePro(
                                sp.textura,
                                sp.recorte,
                                destinoFerramenta,
                                origemFerramenta,
                                anguloFerramenta + 45.0f,
                                WHITE
                            );
                        }
                    }
                }

                //desehar arvores
                for (Recursos *a = arvoresAtivas; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaArvore(texArvore, a);
                    // Barra de vida da arvore (apenas quando danificada)
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
                for (Recursos *a = pedrasAtivas; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaMinerio(texPedra, a);
                    // Barra de vida da pedra (apenas quando danificada)
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
                for (Recursos *a = ferroAtivos; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaMinerio(texFerro, a);
                    // Barra de vida do ferro (apenas quando danificado)
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
                for (Recursos *a = ouroAtivos; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaMinerio(texOuro, a);
                    // Barra de vida do ouro (apenas quando danificado)
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
                for (Recursos *a = diamanteAtivos; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaMinerio(texDiamante, a);
                    // Barra de vida do diamante (apenas quando danificado)
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
            EndMode2D();
            if (localAtual == LOCAL_ILHA) {
                DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, Fade(BLACK, intensidade_noite));
            }
            // Tela levemente vermelha enquanto toma dano
            if (naAgua) DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, Fade(RED, 0.2f));

            DesenhaVida(vida);

            if (pertoLava && !lavaAberta)
            {
                DesenhaTextoCentralizado("APERTE F PARA INTERAGIR", 90.0f, 20.0f, 2.0f, WHITE);
            }

            if (lavaAberta)
            {
                int w = 360, h = 170;
                int x = (LARGURA_TELA - w) / 2, y = (ALTURA_TELA - h) / 2;

                DrawRectangle(x, y, w, h, Fade(BLACK, 0.85f));
                DrawRectangleLines(x, y, w, h, (Color){ 255, 140, 0, 255 });

                DesenhaTextoCentralizado("LAVA", y + 12, 22.0f, 2.0f, (Color){ 255, 140, 0, 255 });
                DesenhaTextoCentralizado(TextFormat("Jogar %d diamantes na lava?", LAVA_CUSTO),
                                        y + 50, 16.0f, 1.5f, WHITE);

                int tem = Inv_Contar(&inv, ITEM_DIAMANTE);
                DesenhaTextoCentralizado(TextFormat("Diamantes: %d / %d", tem, LAVA_CUSTO),
                                        y + 78, 16.0f, 1.5f, tem >= LAVA_CUSTO ? GREEN : RED);

                DesenhaTextoCentralizado("ENTER - jogar    F - fechar", y + 110, 14.0f, 1.5f, LIGHTGRAY);

                if (tempoMensagemLava > 0.0f)
                    DesenhaTextoCentralizado(mensagemLava, y + 138, 14.0f, 1.5f, corMensagemLava);
            }

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

            if (invAberto) {
                DrawRectangle(invX - INV_PAD, invY - INV_PAD,
                            INV_LARGURA + 2 * INV_PAD, INV_ALTURA + 2 * INV_PAD,
                            Fade(BLACK, 0.6f));
                Inv_Draw(&inv, invX, invY, INV_ROWS, origemInv);

                DrawText("Clique: mover  |  1-9: barra  |  direito: descartar (Shift = tudo)",
                         invX, invY - 26, 14, LIGHTGRAY);

                // Destaque e dica do slot sob o mouse.
                int sobre = Inv_SlotNoPonto(invX, invY, GetMousePosition());
                if (sobre >= 0 && inv.slots[sobre].tipo != ITEM_VAZIO) {
                    Rectangle slotSob = Inv_RetanguloSlot(invX, invY, sobre);
                    DrawRectangleLinesEx(slotSob, 2.0f,
                                         Fade((Color){ 220, 160, 80, 255 }, 0.9f));

                    char dica[96];
                    snprintf(dica, sizeof(dica), "%s - direito descarta 1%s",
                             itens[inv.slots[sobre].tipo].nome,
                             inv.slots[sobre].quant > 1
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
            } else {
                Inv_Draw(&inv, invX, ALTURA_TELA - SLOT_TAM - 10, 1, -1);
            }

            DrawText(
                TextFormat("Escudo: %s | Defesa: %.0f",
                    escudoEquipado.tipo == ITEM_ESCUDO ? "Equipado" : "Nenhum",
                    defesaJogador),
                10, 10, 16, WHITE
            );

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
            DrawText(TextFormat("Dia %d | %s %02d:%02d", dia, isNoite ? "Noite" : "Dia", hora, minuto), 545, 10, 20, isNoite ? BLUE : GOLD);
            if (debug) DrawFPS(10, 40);
            if (tempoAviso > 0.0f)
            {
                float alpha = (tempoAviso < 0.5f) ? tempoAviso / 0.5f : 1.0f;   // some suavemente no final
                DesenhaTextoCentralizado(aviso, 40.0f, 18.0f, 2.0f, Fade(corAviso, alpha));
            }
        ApresentarTela(tela, destinoTela);
    }

    if(dia > recorde) {
        recorde = dia;
        SalvarRecorde(recorde);
    }
    // --- DESCARREGAR RECURSOS ---
        // --- DESCARREGAR RECURSOS ---
    Itens_Unload();
    liberar_recursos(arvores);
    liberar_recursos(pedras);
    liberar_recursos(ferros);
    liberar_recursos(ouros);
    liberar_recursos(diamantes);
    UnloadTexture(personagem);
    UnloadTexture(texArvore);
    UnloadTexture(texturaMonstro);
    UnloadTexture(texturaEsqueleto);
    UnloadTexture(texturaCaranguejo);
    UnloadTexture(texturaSlime);
    UnloadTexture(texPedra);
    UnloadTexture(texFerro);
    UnloadTexture(texOuro);
    UnloadTexture(texDiamante);
    UnloadRenderTexture(tela);
    for (int i = 0; i < 3; i++) UnloadTexture(texMapas[i]);
    CloseWindow();
    return 0;
}
