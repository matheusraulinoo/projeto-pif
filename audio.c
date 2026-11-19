#include "audio.h"

Sound sndAmanhecer;
Sound sndAndarCasa;
Sound sndAndarIlha;
Sound sndApanhar;
Sound sndAtingiu;
Sound sndBater;
Sound sndCraft;
Sound sndEntrarCasa;
Sound sndEscudo;
Sound sndMorrer;
Sound sndSelecionar;

void Audio_Init(void)
{
    InitAudioDevice();

    sndAmanhecer   = LoadSound("assets/sounds/amanhecer.wav");
    sndAndarCasa   = LoadSound("assets/sounds/andar_casa.wav");
    sndAndarIlha   = LoadSound("assets/sounds/andar_ilha.wav");
    sndApanhar     = LoadSound("assets/sounds/apanhar.wav");
    sndAtingiu     = LoadSound("assets/sounds/atingiu.wav");
    sndBater       = LoadSound("assets/sounds/bater.wav");
    sndCraft       = LoadSound("assets/sounds/craft.wav");
    sndEntrarCasa  = LoadSound("assets/sounds/entrar_casa.wav");
    sndEscudo      = LoadSound("assets/sounds/escudo.wav");
    sndMorrer      = LoadSound("assets/sounds/morrer.wav");
    sndSelecionar  = LoadSound("assets/sounds/selecionar.wav");

    // Ajustes simples de volume
    SetSoundVolume(sndAndarCasa, 0.35f);
    SetSoundVolume(sndAndarIlha, 0.35f);
    SetSoundVolume(sndAmanhecer, 0.7f);
}

void Audio_Unload(void)
{
    UnloadSound(sndAmanhecer);
    UnloadSound(sndAndarCasa);
    UnloadSound(sndAndarIlha);
    UnloadSound(sndApanhar);
    UnloadSound(sndAtingiu);
    UnloadSound(sndBater);
    UnloadSound(sndCraft);
    UnloadSound(sndEntrarCasa);
    UnloadSound(sndEscudo);
    UnloadSound(sndMorrer);
    UnloadSound(sndSelecionar);

    CloseAudioDevice();
}

void TocarSom(Sound som)
{
    if (IsAudioDeviceReady()) PlaySound(som);
}
