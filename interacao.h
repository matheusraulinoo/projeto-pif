#ifndef INTERACAO_H
#define INTERACAO_H

#include <stddef.h>
#include "raylib.h"
#include "config.h"
#include "inventory.h"

// Estado da interação com a lava (painel aberto + mensagem de feedback)
extern int lavaAberta;
extern char mensagemLava[64];
extern float tempoMensagemLava;
extern Color corMensagemLava;

// Com o painel da lava aberto, ENTER troca LAVA_CUSTO diamantes por 1 magnetita
void InteragirLava(Inventario *inv);

// Inventário aberto: clique esquerdo move, clique direito descarta (Shift = stack inteiro)
// e as teclas 1-9 selecionam/trocam slots da barra
void InteragirInventario(Inventario *inv, int invAberto, int invX, int invY,
                         int *origemInv, char *aviso, size_t tamAviso,
                         Color *corAviso, float *tempoAviso);

#endif
