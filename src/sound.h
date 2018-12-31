#pragma once
#include <stdio.h>
#include <stdlib.h>

#include <psp2/audioout.h>
int soundThread(SceSize argc,void *argv);
int readThread(SceSize argc, void* argv);
int SoundReady();
int SoundPlayGo();
