#ifndef ENTITIES_H
#define ENTITIES_H

#include "raylib.h"
#include "config.h"
#include "inventory.h"

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

#define MAX_MONSTROS 100

// Recortes no spritesheet (arvore.png, 74x62)
#define RECORTE_TOCO   (Rectangle){ 1, 48, 22, 14 }
#define RECORTE_ARVORE (Rectangle){ 34, 0, 39, 61 }
#define RECORTE_PEDREGULHO (Rectangle){ 1, 50, 10, 7 }
#define RECORTE_MINERIO    (Rectangle){ 38, 32, 35, 25 }

typedef struct Recursos {
    Vector2 posicao;
    Vector2 offset_colisao;
    float raio_colisao;
    int vida;
    int vida_maxima;
    bool quebrado;
    bool colide_quebrado;
    float tempo_renascimento;
    float cooldown_renascimento;
    struct Recursos *proximo;
} Recursos;

typedef enum {
    MONSTRO_SLIME,
    MONSTRO_ESQUELETO,
    MONSTRO_CARANGUEJO,
    TOTAL_TIPOS_MONSTRO
} TipoMonstro;

typedef struct {
    TipoMonstro tipo;
    Vector2 posicao;
    float velocidade;
    bool elite;
    float vida;
    float vida_maxima;
    float ataque;
    float defesa;
    const char *nome;
    bool morto;
} Monstro;

// Estado global dos monstros (definido em monstros.c)
extern Monstro monstros[MAX_MONSTROS];
extern int quantidadeMonstros;

extern float timerSpawn;
extern float intervaloSpawn; // um monstro a cada 2 segundos

void inserir_recurso(Recursos **cabeca, Recursos novo);
void quebrar_recurso(Recursos *recurso, int dano);
void atualiza_recurso(Recursos *cabeca);
void liberar_recursos(Recursos *cabeca);
int TemRecursoPerto(Vector2 pos, float distMin, Recursos *lista);
bool AtingidoPorSlash( Vector2 jogador, Vector2 alvo, Vector2 direcaoSlash, float alcance, float angulo, float raioAlvo);
void DesenhaMinerio(Texture2D tex, Recursos *r);
void DesenhaArvore(Texture2D tex, Recursos *r);
int SobrepoeRecurso(Vector2 posicao, float raio, Recursos *recursos);
int ColideComRecursos(Vector2 atual, Vector2 proximo, float raio, Recursos *recursos);

// ---- Geração de recursos do mundo ----
// Sorteia as árvores nos tiles 6 da ilha
void GerarArvores(char mapa[LINHAS][COLS], Vector2 jogador, Recursos **arvores);

// Sorteia pedra, ferro, ouro e diamante nos tiles permitidos da caverna
void GerarMinerios(char mapaCaverna[LINHAS][COLS],
                   Recursos **pedras, Recursos **ferros,
                   Recursos **ouros, Recursos **diamantes);

// ---- Monstros: spawn, movimento, dano e desenho ----
void SpawnMonstro(
    Monstro monstros[],
    int *quantidadeMonstros,
    Vector2 jogador,
    char mapa[LINHAS][COLS],
    Recursos *arvores,
    int dia
);

void MoverMonstro(
    Monstro *monstro,
    Vector2 jogador,
    float dt,
    char mapa[LINHAS][COLS],
    Recursos *arvores
);

// Move os monstros, aplica dano de contato no jogador (com desgaste do escudo) e dano do sol
void AtualizarMonstros(
    float dt,
    Vector2 jogador,
    char mapa[LINHAS][COLS],
    Recursos *arvoresAtivas,
    LocalAtual localAtual,
    bool isNoite,
    float intervaloDano,
    float *vida,
    float *cooldownDano,
    float *cooldownEscudo,
    Slot *escudoEquipado,
    float *defesaJogador
);

// Desenha os monstros vivos (sprite, barra de vida e nome)
void DesenhaMonstros(
    Texture2D texturaSlime,
    Texture2D texturaEsqueleto,
    Texture2D texturaCaranguejo,
    LocalAtual localAtual
);

#endif
