#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/audioout.h>
#include <stdio.h>
#include "monorale.h"
#include "sound.h"

#define printf sceClibPrintf
int main(int argc, char *argv[]) {
	//printf("Hello Vita!");
	monorale_hdr *hdr = monorale_init("app0:/monorale.bin");
	
	//Init Audio.
	int port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN,4096,44100,1);//Stereo.
	if(port<0) printf("Open Audio Failed!!!");
	
	SceUID vid = sceKernelCreateThread("video thread", monoraleThread, 0x40, 0x10000, 0, 0, NULL);
       	SceUID snd = sceKernelCreateThread("sound thread", soundThread,0x41,0x10000,0,0,NULL);
	
	if(vid < 0){printf("videoThread creation failed E:%x\n",vid); goto end;}
	else if(snd < 0){ printf("soundThread creation failed E:%x\n",snd); goto end;}
	else {
		sceKernelStartThread(vid,sizeof(monorale_hdr),&hdr);
		sceKernelStartThread(snd,sizeof(int),&port);
	}
	sceKernelWaitThreadEnd(vid,0,0);
	sceKernelWaitThreadEnd(snd,0,0);
end:
	monorale_deinit();
	return 0;
}
