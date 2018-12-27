#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/audioout.h>
#include <stdio.h>
#include "monorale.h"
#include "sound.h"
int main(int argc, char *argv[]) {
	//printf("Hello Vita!");
	int ret;
	monorale_hdr *hdr = monorale_init("app0:/monorale.bin");
	
	//Init Audio.
	int port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN,4096,44100,1);//Stereo.
	if(port<0) printf("Open Audio Failed!!!");
       	SceUID snd = sceKernelCreateThread("sound thread", soundThread,0x41,0x10000,0,0,NULL);
	if(snd < 0) printf("soundThread creation failed");
	else {
		sceKernelStartThread(snd,sizeof(int),&port);
	}


	SceUID vid = sceKernelCreateThread("video thread", monoraleThread, 0x40, 0x10000, 0, 0, NULL);
	if(vid < 0) {
		printf("Thread Failed.");
	} else sceKernelStartThread(vid,sizeof(monorale_hdr),&hdr);
	sceKernelWaitThreadEnd(vid,0,0);
	sceKernelWaitThreadEnd(snd,0,0);
	return 0;
}
