#include "raylib.h"
#include <stdio.h>
#include "entities.h"
#include "inventory.h"
#include "crafting.h"
#include "math.h"
#include "time.h"
#include "raymath.h"
#include "ranking.h"
#include "config.h"
#include "mapa.h"
#include "combate.h"
#include "hud.h"
#include "telas.h"
#include "interacao.h"
<<<<<<< HEAD
#include "audio.h"
=======

>>>>>>> 6c87f3b833ecb3324931520bad8ccb0bf72a7af8
// Ao modificar o codigo, o projeto deve ser recompilado para que as alterações tenham efeito.
// comando pra fazer isso: gcc *.c -o jogo.exe -I C:\raylib\include -L C:\raylib\lib -lraylib -lopengl32 -lgdi32 -lwinmm
// certifique-se de que o jogo previamente aberto foi fechado antes de recompilar.
// para executar o jogo, abra o terminal na pasta do projeto e digite: ./jogo.exe

char nomeDigitado[32] = "\0";
int contadorLetras = 0;
int main(void)
{
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Scary Nights");
    SetTargetFPS(60);
    Audio_Init();


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
    
    int totalReceitas = TotalReceitas();
    int receitaSelecionada = 0;
    int craftingAberto = 0;
    char mensagemCraft[64] = "";
    char aviso[96] = "";
    float tempoAviso = 0.0f;
    Color corAviso = WHITE;
    float tempoMensagemCraft = 0.0f;
    Color corMensagemCraft = WHITE;
    float timerPassos = 0.0f;

    // --- CARREGAMENTO DE TEXTURAS (Uma única vez aqui) ---
    float tempo_jogo = 0.0f;          // Conta os segundos passados no jogo
    float duracao_dia_noite = 60.0f;  // Duração completa de um ciclo 
    // --- Sprite do personagem (carregar depois do InitWindow) ---
    Texture2D personagem = LoadTexture("assets/textures/personagem.png");
    SetTextureFilter(personagem, TEXTURE_FILTER_POINT); // mantém a pixel art nítida
    
    Texture2D texArvore = LoadTexture("assets/textures/arvore.png");
    SetTextureFilter(texArvore, TEXTURE_FILTER_POINT);

    Texture2D texPedra = LoadTexture("assets/textures/pedra_bruta.png");
    SetTextureFilter(texPedra, TEXTURE_FILTER_POINT);

    Texture2D texFerro = LoadTexture("assets/textures/ferro_bruto.png");
    SetTextureFilter(texFerro, TEXTURE_FILTER_POINT);

    Texture2D texOuro = LoadTexture("assets/textures/ouro_bruto.png");
    SetTextureFilter(texOuro, TEXTURE_FILTER_POINT);

    Texture2D texDiamante = LoadTexture("assets/textures/diamante_bruto.png");
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
    texMapas[LOCAL_ILHA]    = LoadTexture("assets/textures/ilha.png");
    texMapas[LOCAL_CASA]    = LoadTexture("assets/textures/casa.png");
    texMapas[LOCAL_CAVERNA] = LoadTexture("assets/textures/caverna.png");
    for (int i = 0; i < 3; i++)
        if (texMapas[i].id != 0) SetTextureFilter(texMapas[i], TEXTURE_FILTER_POINT);

    bool debug = false;   // F1 mostra a colisão do txt por cima

    Texture2D texturaMonstro = LoadTexture("assets/textures/slime.png");
    // Texturas temporarias dos diferentes monstros
    Texture2D texturaSlime = texturaMonstro;
    Texture2D texturaEsqueleto = LoadTexture("assets/textures/esqueleto.png");
    Texture2D texturaCaranguejo = LoadTexture("assets/textures/caranguejo.png");

    Recursos *arvores = NULL;
    Recursos *pedras = NULL;
    Recursos *ferros = NULL;
    Recursos *ouros = NULL;
    Recursos *diamantes = NULL;

    GerarArvores(mapa, jogador, &arvores);

    char mapaCaverna[LINHAS][COLS];
    if (!CarregarMapa(MAPA_CAVERNA, mapaCaverna)) return 1;

    GerarMinerios(mapaCaverna, &pedras, &ferros, &ouros, &diamantes);
    
    int dentro = 0;                          // 0 = fora, 1 = dentro do local
    LocalAtual localAtual = LOCAL_ILHA;
    Vector2 posAntesDeEntrar = jogador;      // para voltar ao lugar certo ao sair
    Recursos *arvoresAtivas = arvores;       // árvores do mapa atual (NULL dentro de casa/caverna)
    Recursos *pedrasAtivas = NULL;         // pedras do mapa atual (NULL dentro de casa/ilha)
    Recursos *ferroAtivos = NULL;         // ferro do mapa atual (NULL dentro de casa/ilha)
    Recursos *ouroAtivos = NULL;         // ouro do mapa atual (NULL dentro de casa/ilha)
    Recursos *diamanteAtivos = NULL;         // diamante do mapa atual (NULL dentro de casa/ilha)

    int dia = 1;                             // contador de dias

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
                DesenhaTelaInicio(tempoTela, personagem);
                DesenharRankingUI(ranking, 520, 170);
            ApresentarTela(tela, destinoTela);

            continue;
        }

// --- TELA DE MORTE ---
        else if (telaAtual == TELA_MORTE)
        {
            tempoMorte += dt;

            // 1. Captura as letras digitadas pelo jogador (e apaga com o Backspace)
            AtualizarCampoNome(nomeDigitado, &contadorLetras);

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
            DesenhaTelaMorte(tempoMorte, DURACAO_TELA_MORTE - tempoMorte, dia);

            // Caixa de texto para o jogador digitar o nome
            DesenhaCaixaNome(nomeDigitado, contadorLetras);

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
            TocarSom(sndAmanhecer);

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
                    TocarSom(sndEntrarCasa);
                }
            }
            else
            {
                if (CarregarMapa(MAPA_ILHA, mapa))
                {
                    dentro = 0;
                    jogador = posAntesDeEntrar;
                    localAtual = LOCAL_ILHA;
                    TocarSom(sndEntrarCasa);
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
                    TocarSom(sndEntrarCasa);
                }
            }
            else
            {
                if (CarregarMapa(MAPA_ILHA, mapa))
                {
                    dentro = 0;
                    jogador = posAntesDeEntrar;
                    localAtual = LOCAL_ILHA;
                    TocarSom(sndEntrarCasa);
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

            timerPassos -= dt;
            if (timerPassos <= 0.0f)
            {
                if (localAtual == LOCAL_CASA) TocarSom(sndAndarCasa);
                else TocarSom(sndAndarIlha);
                timerPassos = 0.38f;
            }
        }
        else
        {
            frameAtual = 0;   // parado = primeiro frame
            timerAnim = 0.0f;
            timerPassos = 0.0f;   // toca ao dar o proximo passo
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
            TocarSom(sndBater);

            bool acertouAlvo = false;

            // Verifica todos os monstros
            if (localAtual != LOCAL_CASA) {
                if (AtacarMonstros(jogador, direcaoSlash, ferramentaAtual)) acertouAlvo = true;
            }

            // Corte de arvores com o slash
            if (ferramentaAtual.poderCorte > 0) {
                if (AtacarRecursos(arvoresAtivas, jogador, direcaoSlash, ferramentaAtual,
                                   ferramentaAtual.poderCorte, ITEM_MADEIRA, &inv)) acertouAlvo = true;
            }

            if (ferramentaAtual.poderMineracao > 0) {
                if (ferramentaAtual.tipo == ITEM_PICARETA){
                    if (AtacarRecursos(pedrasAtivas, jogador, direcaoSlash, ferramentaAtual,
                                       ferramentaAtual.poderMineracao, ITEM_PEDRA, &inv)) acertouAlvo = true;
                }
            }
            if (ferramentaAtual.poderMineracao > 0) {
                if (PicaretaPodeQuebrar(ferramentaAtual, TIER_PEDRA)) {
                    if (AtacarRecursos(ferroAtivos, jogador, direcaoSlash, ferramentaAtual,
                                       ferramentaAtual.poderMineracao, ITEM_FERRO, &inv)) acertouAlvo = true;
                }
            }
            if (ferramentaAtual.poderMineracao > 0) {
                if (PicaretaPodeQuebrar(ferramentaAtual, TIER_FERRO)) {
                    if (AtacarRecursos(ouroAtivos, jogador, direcaoSlash, ferramentaAtual,
                                       ferramentaAtual.poderMineracao, ITEM_OURO, &inv)) acertouAlvo = true;
                }
            }
            if (ferramentaAtual.poderMineracao > 0) {
                if (PicaretaPodeQuebrar(ferramentaAtual, TIER_OURO)) {
                    if (AtacarRecursos(diamanteAtivos, jogador, direcaoSlash, ferramentaAtual,
                                       ferramentaAtual.poderMineracao, ITEM_DIAMANTE, &inv)) acertouAlvo = true;
                }
            }

            // Consome durabilidade apenas quando acertar um alvo
            if (acertouAlvo) {
                TocarSom(sndAtingiu);
                ConsumirDurabilidade(&inv, &ferramentaAtual);
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

        InteragirLava(&inv);

        InteragirInventario(&inv, invAberto, invX, invY, &origemInv,
                            aviso, sizeof(aviso), &corAviso, &tempoAviso);

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
            TocarSom(sndMorrer);
            telaAtual = TELA_MORTE;
            tempoMorte = 0.0f;
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

        //atualiza monstros
        AtualizarMonstros(
            dt,
            jogador,
            mapa,
            arvoresAtivas,
            localAtual,
            isNoite,
            intervaloDano,
            &vida,
            &cooldownDano,
            &cooldownEscudo,
            &escudoEquipado,
            &defesaJogador
        );

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

                    DesenhaTilesMapa(mapa, alpha);
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
                DesenhaMonstros(texturaSlime, texturaEsqueleto, texturaCaranguejo, localAtual);

                // Animacao visual do slash
                DesenhaSlash(jogador, direcaoSlash, ferramentaAnimacao, timerSlash, duracaoSlash);

                //desehar arvores
                for (Recursos *a = arvoresAtivas; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaArvore(texArvore, a);
                    // Barra de vida da arvore (apenas quando danificada)
                    DesenhaBarraVidaRecurso(a);
                }
                for (Recursos *a = pedrasAtivas; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaMinerio(texPedra, a);
                    // Barra de vida da pedra (apenas quando danificada)
                    DesenhaBarraVidaRecurso(a);
                }
                for (Recursos *a = ferroAtivos; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaMinerio(texFerro, a);
                    // Barra de vida do ferro (apenas quando danificado)
                    DesenhaBarraVidaRecurso(a);
                }
                for (Recursos *a = ouroAtivos; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaMinerio(texOuro, a);
                    // Barra de vida do ouro (apenas quando danificado)
                    DesenhaBarraVidaRecurso(a);
                }
                for (Recursos *a = diamanteAtivos; a != NULL; a = a->proximo)
                {
                    if (a->posicao.y > jogador.y) DesenhaMinerio(texDiamante, a);
                    // Barra de vida do diamante (apenas quando danificado)
                    DesenhaBarraVidaRecurso(a);
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
                DesenhaPainelLava(&inv, mensagemLava, corMensagemLava, tempoMensagemLava);
            }

            // Slot visual do escudo equipado
            DesenhaEscudoEquipado(escudoEquipado);

            if (invAberto) {
                DesenhaInventarioAberto(&inv, invX, invY, origemInv);
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
                        TocarSom(sndCraft);
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
    Audio_Unload();
    CloseWindow();
    return 0;
}
