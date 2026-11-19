#include "raylib.h"
#include <stdio.h>
#include "raymath.h"
#include "combate.h"

// Cada minério exige no minimo a picareta do tier indicado
int PicaretaPodeQuebrar(Ferramenta ferramenta, TierItem tierMinimo) {
    return ferramenta.tipo == ITEM_PICARETA && ferramenta.tier >= tierMinimo;
}

bool AtacarMonstros(Vector2 jogador, Vector2 direcaoSlash, Ferramenta ferramentaAtual)
{
    bool acertouAlvo = false;

    // Verifica todos os monstros
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

    return acertouAlvo;
}

bool AtacarRecursos(Recursos *lista, Vector2 jogador, Vector2 direcaoSlash,
                    Ferramenta ferramentaAtual, int poder, TipoItem recompensa,
                    Inventario *inv)
{
    bool acertouAlvo = false;

    for (Recursos *a = lista; a != NULL; a = a->proximo) {

        if (a->quebrado) continue;

        // Usa o centro de colisao do tronco
        Vector2 centro = Vector2Add(
            a->posicao,
            a->offset_colisao
        );

        if (AtingidoPorSlash(
            jogador,
            centro,
            direcaoSlash,
            ferramentaAtual.alcanceSlash,
            ferramentaAtual.anguloSlash,
            a->raio_colisao
        )) {

            quebrar_recurso(a, poder);
            acertouAlvo = true;

            // Mantem a recompensa existente
            if (a->quebrado) {
                Inv_Add(inv, recompensa, 1);
            }
        }
    }

    return acertouAlvo;
}

void ConsumirDurabilidade(Inventario *inv, Ferramenta *ferramentaAtual)
{
    // Consome durabilidade apenas quando acertar um alvo
    Slot *slotEquipado = &inv->slots[inv->selecionado];

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
            *ferramentaAtual = ObterFerramenta(*slotEquipado);
        }
    }
}

void DesenhaSlash(Vector2 jogador, Vector2 direcaoSlash,
                  Ferramenta ferramentaAnimacao, float timerSlash, float duracaoSlash)
{
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
}
