#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "entities.h"
#include "mapa.h"
#include "raymath.h"

// Cria um nó com os dados de "novo" e o coloca no fim da lista.
void inserir_recurso(Recursos **cabeca, Recursos novo) {
    Recursos *no = (Recursos *)malloc(sizeof(Recursos));
    if (no == NULL) return;

    *no = novo;
    no->proximo = NULL;

    if (*cabeca == NULL) {
        *cabeca = no;
        return;
    }

    Recursos *atual = *cabeca;
    while (atual->proximo != NULL) atual = atual->proximo;
    atual->proximo = no;
}


void quebrar_recurso(Recursos *recurso, int dano) {
    if (recurso->quebrado) return; // Não dá para quebrar o que já tá quebrado

    recurso->vida -= dano;

    if (recurso->vida <= 0) {
        recurso->quebrado = true;
        recurso->tempo_renascimento = recurso->cooldown_renascimento; // Inicia a contagem regressiva
    }
}


void atualiza_recurso(Recursos *cabeca) {
    float deltaTime = GetFrameTime(); // Tempo do frame atual em segundos

    for (Recursos *recurso = cabeca; recurso != NULL; recurso = recurso->proximo) {
        if (recurso->quebrado) {
            // Conta o tempo para renascer
            recurso->tempo_renascimento -= deltaTime;

            if (recurso->tempo_renascimento <= 0.0f) {
                recurso->quebrado = false;
                recurso->vida = recurso->vida_maxima;
            }
        }
    }
}


// Libera todos os nós da lista.
void liberar_recursos(Recursos *cabeca) {
    while (cabeca != NULL) {
        Recursos *proximo = cabeca->proximo;
        free(cabeca);
        cabeca = proximo;
    }
}

int TemRecursoPerto(Vector2 pos, float distMin, Recursos *lista)
{
    for (Recursos *r = lista; r != NULL; r = r->proximo)
    {
        float dx = pos.x - r->posicao.x;
        float dy = pos.y - r->posicao.y;
        if (dx * dx + dy * dy < distMin * distMin) return 1;
    }
    return 0;
}


int ColideComRecursos(Vector2 atual, Vector2 proximo, float raio, Recursos *recursos)
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

int SobrepoeRecurso(Vector2 posicao, float raio, Recursos *recursos)
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

void DesenhaArvore(Texture2D tex, Recursos *r)
{
    Rectangle src = r->quebrado ? RECORTE_TOCO : RECORTE_ARVORE;

    float w = src.width  * ESCALA_ARVORE;
    float h = src.height * ESCALA_ARVORE;

    Rectangle dst = { r->posicao.x, r->posicao.y, w, h };
    Vector2 origem = { w / 2.0f, h };   // base, no centro (já escalado)

    DrawTexturePro(tex, src, dst, origem, 0.0f, WHITE);
}

void DesenhaMinerio(Texture2D tex, Recursos *r)
{
    Rectangle src = r->quebrado ? RECORTE_PEDREGULHO : RECORTE_MINERIO;

    float w = src.width  * ESCALA_MINERIO;
    float h = src.height * ESCALA_MINERIO;

    Rectangle dst = { r->posicao.x, r->posicao.y, w, h };
    Vector2 origem = { w / 2.0f, h };   // base, no centro (já escalado)

    DrawTexturePro(tex, src, dst, origem, 0.0f, WHITE);
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
//  RECURSOS DO MUNDO (geração de árvores e minérios)
// ============================================================

void GerarArvores(char mapa[LINHAS][COLS], Vector2 jogador, Recursos **arvores)
{
    int numArvores = 0;

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
            for (Recursos *a = *arvores; a != NULL; a = a->proximo)
            {
                dx = pos.x - a->posicao.x;
                dy = pos.y - a->posicao.y;
                if (dx * dx + dy * dy < DIST_MIN_ARVORES * DIST_MIN_ARVORES) { perto = 1; break; }
            }
            if (perto) continue;

            inserir_recurso(arvores, (Recursos){
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
}

void GerarMinerios(char mapaCaverna[LINHAS][COLS],
                   Recursos **pedras, Recursos **ferros,
                   Recursos **ouros, Recursos **diamantes)
{
    int numPedras = 0;
    int numFerro = 0;
    int numOuro = 0;
    int numDiamante = 0;

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

            if (TemRecursoPerto(pos, DIST_MIN_MINERIO, *pedras)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *ferros)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *ouros)    ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *diamantes)) continue;

            inserir_recurso(pedras, (Recursos){
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

            if (TemRecursoPerto(pos, DIST_MIN_MINERIO, *pedras)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *ferros)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *ouros)    ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *diamantes)) continue;

            inserir_recurso(ferros, (Recursos){
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

            if (TemRecursoPerto(pos, DIST_MIN_MINERIO, *pedras)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *ferros)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *ouros)    ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *diamantes)) continue;
                
            inserir_recurso(ouros, (Recursos){
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

            if (TemRecursoPerto(pos, DIST_MIN_MINERIO, *pedras)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *ferros)   ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *ouros)    ||
                TemRecursoPerto(pos, DIST_MIN_MINERIO, *diamantes)) continue;

            inserir_recurso(diamantes, (Recursos){
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
}


// ============================================================
//  MONSTROS
// ============================================================

Monstro monstros[MAX_MONSTROS] = {0};
int quantidadeMonstros = 0;

float timerSpawn = 0.0f;
float intervaloSpawn = 5.0f; // um monstro a cada 2 segundos

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
) {
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
            if (*cooldownDano <= 0.0f &&
                CheckCollisionCircles(
                    jogador, 14.0f,
                    monstros[i].posicao,
                    monstros[i].elite ? 18.0f : 12.0f
                )) {

                float dano = monstros[i].ataque - *defesaJogador;

                if (dano < 0.0f) {
                    dano = 0.0f;
                }

                // Desgasta o escudo quando ele bloqueia dano
                if (escudoEquipado->tipo == ITEM_ESCUDO &&
                    monstros[i].ataque > 0.0f &&
                    *defesaJogador > 0.0f &&
                    *cooldownEscudo <= 0.0f){

                    if (escudoEquipado->durability > 0) {
                        escudoEquipado->durability--;
                        *cooldownEscudo = intervaloDano;
                    }

                    if (escudoEquipado->durability <= 0) {
                        printf("Escudo quebrou!\n");

                        *escudoEquipado = (Slot){0};
                        *defesaJogador = 0.0f;
                    }
                }

                // Aplica dano e invencibilidade apenas se houver perda de vida
                if (dano > 0.0f) {
                    *vida -= dano;
                    *cooldownDano = intervaloDano;
                }

                printf("Monstro atacou! Dano: %.1f | Vida: %.1f\n",
                    dano, *vida);
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
}

void DesenhaMonstros(
    Texture2D texturaSlime,
    Texture2D texturaEsqueleto,
    Texture2D texturaCaranguejo,
    LocalAtual localAtual
) {
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
}
