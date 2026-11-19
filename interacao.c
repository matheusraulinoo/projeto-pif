#include <stdio.h>
#include "interacao.h"
#include "audio.h"

int lavaAberta = 0;
char mensagemLava[64] = "";
float tempoMensagemLava = 0.0f;
Color corMensagemLava = WHITE;

void InteragirLava(Inventario *inv)
{
    if (lavaAberta && IsKeyPressed(KEY_ENTER))
    {
        if (Inv_Contar(inv, ITEM_DIAMANTE) < LAVA_CUSTO)
        {
            snprintf(mensagemLava, sizeof(mensagemLava), "Voce precisa de %d diamantes.", LAVA_CUSTO);
            corMensagemLava = RED;
        }
        else
        {
            Inv_Remover(inv, ITEM_DIAMANTE, LAVA_CUSTO);

            if (Inv_Add(inv, ITEM_MAGNETITA, 1) > 0)
            {
                // Sem espaço: devolve os diamantes
                Inv_Add(inv, ITEM_DIAMANTE, LAVA_CUSTO);
                snprintf(mensagemLava, sizeof(mensagemLava), "Inventario cheio!");
                corMensagemLava = RED;
            }
            else
            {
                snprintf(mensagemLava, sizeof(mensagemLava), "A lava aceitou! Voce recebeu 1 magnetita.");
                corMensagemLava = GREEN;
                TocarSom(sndCraft);
            }
        }
        tempoMensagemLava = 2.0f;
    }
}

void InteragirInventario(Inventario *inv, int invAberto, int invX, int invY,
                         int *origemInv, char *aviso, size_t tamAviso,
                         Color *corAviso, float *tempoAviso)
{
    if (invAberto && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int destino = Inv_SlotNoPonto(invX, invY, GetMousePosition());
        if (destino >= 0) {
            if (*origemInv >= 0) {
                if (*origemInv != destino) Inv_TrocarSlots(inv, *origemInv, destino);
                *origemInv = -1;
            } else if (inv->slots[destino].tipo != ITEM_VAZIO) {
                *origemInv = destino;
                TocarSom(sndSelecionar);
            }
        }
    }

    // Clique direito descarta 1 item do slot. Com Shift, descarta o stack.
    if (invAberto && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        int alvo = Inv_SlotNoPonto(invX, invY, GetMousePosition());

        if (alvo >= 0 && inv->slots[alvo].tipo != ITEM_VAZIO) {
            Slot alvoSlot = inv->slots[alvo];
            int qtd = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
                      ? alvoSlot.quant : 1;
            int removidos = Inv_DescartarSlot(inv, alvo, qtd);

            if (removidos > 0) {
                if (*origemInv == alvo) *origemInv = -1;

                if (removidos > 1)
                    snprintf(aviso, tamAviso, "Descartado: %s x%d",
                             itens[alvoSlot.tipo].nome, removidos);
                else
                    snprintf(aviso, tamAviso, "Descartado: %s",
                             itens[alvoSlot.tipo].nome);

                *corAviso = (Color){ 214, 190, 150, 255 };
                *tempoAviso = 1.6f;
            }
        }
    }

    for (int k = 0; k < INV_COLS; k++) {
        if (!IsKeyPressed(KEY_ONE + k)) continue;
        if (*origemInv >= 0) {
            if (*origemInv != k) Inv_TrocarSlots(inv, *origemInv, k);
            *origemInv = -1;
        } else {
            if (inv->selecionado != k) TocarSom(sndSelecionar);
            inv->selecionado = k;
        }
        break;
    }
}
