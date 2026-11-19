#include "raylib.h"
#include <stdio.h>
#include "mapa.h"

int ColideComParede(char mapa[LINHAS][COLS], Vector2 centro, float raio)
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

// Retorna 1 se existe algum tile 'tile' a menos de 'alcance' px do jogador
int PertoDeTile(char mapa[LINHAS][COLS], Vector2 pos, char tile, float alcance)
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

// Retorna o id do tile sob a posição (ou -1 se estiver fora do mapa)
int TileEm(char mapa[LINHAS][COLS], Vector2 pos)
{
    int tx = (int)(pos.x / TILE);
    int ty = (int)(pos.y / TILE);
    if (tx < 0 || tx >= COLS || ty < 0 || ty >= LINHAS) return -1;
    return mapa[ty][tx];
}

// Desenha os tiles do mapa como retângulos coloridos (usado sem imagem de fundo ou no modo debug)
void DesenhaTilesMapa(char mapa[LINHAS][COLS], float alpha)
{
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
