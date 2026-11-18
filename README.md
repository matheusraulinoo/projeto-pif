<div align="center">
  <img width="472" height="325" alt="Tela inicial do Scary Nights" src="https://github.com/user-attachments/assets/443c8d07-549d-4df8-844c-38419482965d" />
</div>

<div align="center">
<pre>
  _____                        _   _ _       _     _       
 / ____|                      | \ | (_)     | |   | |      
| (___   ___ __ _ _ __ _   _  |  \| |_  __ _| |__ | |_ ___ 
 \___ \ / __/ _` | '__| | | | | . ` | |/ _` | '_ \| __/ __|
 ____) | (_| (_| | |  | |_| | | |\  | | (_| | | | | |_\__ \
|_____/ \___\__,_|_|   \__, | |_| \_|_|\__, |_| |_|\__|___/
                        __/ |           __/ |              
                       |___/           |___/               
</pre>
</div>

# Scary Nights

![Linguagem](https://img.shields.io/badge/linguagem-C-blue)
![Biblioteca](https://img.shields.io/badge/raylib-5.0%2B-black)
![Plataformas](https://img.shields.io/badge/plataformas-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey)

**Scary Nights** é um jogo de sobrevivência 2D em C com [raylib](https://www.raylib.com/), com coleta de recursos, crafting e combate simples.

Você está preso em uma ilha onde monstros aparecem toda noite. Seu objetivo é se fortalecer com armas e escudos cada vez melhores para sobreviver o máximo de noites possível, enquanto os monstros ficam mais fortes a cada dia que passa.

## ✨ Funcionalidades

- **Ciclo de dia e noite:** os monstros aparecem quando a noite chega.
- **Exploração:** a ilha, uma casa e uma caverna para descobrir.
- **Coleta de recursos:** corte árvores e minere na caverna.
- **Crafting:** crie espadas, machados, picaretas e escudos em vários níveis de material.
- **Combate:** ataque na direção do mouse contra diferentes tipos de monstros, incluindo versões elite.
- **Durabilidade:** ferramentas e escudos se desgastam com o uso.
- **Dificuldade progressiva:** os monstros ficam mais fortes a cada dia.
- **Recorde:** o maior número de dias alcançado fica salvo.

## 🎮 Controles

| Ação | Tecla / Mouse |
|------|---------------|
| Mover | `W` `A` `S` `D` ou setas |
| Atacar / cortar / minerar | Botão esquerdo do mouse |
| Zoom | Roda do mouse |
| Inventário | `TAB` |
| Selecionar item da barra | `1` a `9` |
| Crafting (dentro da casa) | `C` |
| Equipar/desequipar escudo | `H` |
| Interagir | `F` |
| Tela cheia | `F11` |
| Sair | `ESC` |

Na tela inicial, aperte `ENTER` para começar.

## 📦 Requisitos

- Compilador C (`gcc` ou `clang`)
- [raylib](https://github.com/raysan5/raylib) **5.0 ou superior**
- Git

## 🚀 Como rodar o projeto

Clone o repositório e entre na pasta:

```bash
git clone https://github.com/matheusraulinoo/projeto-pif.git
cd projeto-pif
```

O código está dividido em módulos: cada `.c` é compilado separadamente e o `main.c` inclui apenas os `.h`. Execute o jogo sempre **a partir da pasta do projeto**, pois ele carrega imagens e mapas por caminho relativo.

### Windows

Instale a raylib pelo [instalador oficial](https://github.com/raysan5/raylib/releases) (neste exemplo, em `C:\raylib`) e rode:

```bash
gcc main.c ranking.c entities.c inventory.c crafting.c -o jogo.exe -I C:\raylib\include -L C:\raylib\lib -lraylib -lopengl32 -lgdi32 -lwinmm
.\jogo.exe
```

### macOS

```bash
brew install raylib
gcc main.c ranking.c entities.c inventory.c crafting.c -o jogo -I/opt/homebrew/include -L/opt/homebrew/lib -lraylib \
    -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
./jogo
```

> Em Macs com processador Intel, troque `/opt/homebrew` por `/usr/local`.

### Linux

Instale as dependências (exemplo para Debian/Ubuntu) e a raylib 5.0+:

```bash
sudo apt update
sudo apt install build-essential git libasound2-dev libx11-dev libxrandr-dev \
    libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev

git clone https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP
sudo make install
cd ../..
```

Depois compile e execute:

```bash
gcc main.c ranking.c entities.c inventory.c crafting.c -o jogo -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./jogo
```


## 🗂️ Estrutura do projeto

```
scary-nights/
├── main.c          # Loop principal, mapas, ciclo dia/noite, combate e telas
├── ranking.c/.h    # Ranking de melhores jogadores
├── entities.c/.h   # Monstros e recursos coletáveis
├── inventory.c/.h  # Itens, inventário, ferramentas e barra de atalhos
├── crafting.c/.h   # Receitas e interface de crafting
├── mapa.txt        # Mapa da ilha
├── casa.txt        # Mapa da casa
├── caverna.txt     # Mapa da caverna
├── ranking.txt     # Ranking salvo com nome e pontuação
├── recorde.txt     # Gerado automaticamente com o melhor dia alcançado
└── pictures/       # Sprites e imagens dos mapas
```

## 🛠️ Problemas comuns

| Problema | Solução |
|----------|---------|
| `raylib.h: No such file or directory` | Verifique o caminho passado em `-I`. |
| `cannot find -lraylib` | Verifique o caminho passado em `-L`. |
| `ToggleBorderlessWindowed` não declarada | Sua raylib é anterior à 5.0. Atualize. |
| `Nao abriu o arquivo: mapa.txt` | Execute o jogo a partir da pasta do projeto. |
| Erro de permissão ao compilar no Windows | O jogo ainda está aberto. Feche-o e compile de novo. |

## 👥 Créditos

Desenvolvido por:

- **Ana Luiza Carvalho Xavier**
- **João Carlos Soares Sampaio**
- **Matheus Raulino de Souza Moreira**

Feito com [raylib](https://www.raylib.com/), de Ramon Santamaria (@raysan5).
