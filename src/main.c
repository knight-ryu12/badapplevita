#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/audioout.h>
#include <stdio.h>
#include "monorale.h"
#include "sound.h"

#define printf sceClibPrintf

int main(int argc, char *argv[]){

	int Audio_port = 0;
	SceUID snd_thread_uid = 0;
	SceUID vid_thread_uid = 0;
	SceUID read_thread_uid = 0;

	monorale_hdr *hdr = NULL;

	printf("Start BadAppleVita\n");

	hdr = monorale_init("app0:/monorale.bin");

	if(hdr == NULL){
		printf("monorale_init Failed!!!\n");
		goto end;
	}

	//Init Audio.
	Audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, 1024, 44100, SCE_AUDIO_OUT_MODE_STEREO);
	if(Audio_port < 0){
		printf("Open Audio Failed!!!\n");
		printf("error code : 0x%X\n", Audio_port);
		goto end;
	}

	printf("sceAudioOutOpenPort : 0x%X\n", Audio_port);

	read_thread_uid = sceKernelCreateThread("read thread", readThread, 0x40, 0x10000, 0, 0, NULL);
	vid_thread_uid = sceKernelCreateThread("video thread", monoraleThread, 0x40, 0x10000, 0, 0, NULL);
	snd_thread_uid = sceKernelCreateThread("sound thread", soundThread, 0x40, 0x10000, 0, 0, NULL);

	printf("0x%08X : sceKernelCreateThread(\"read thread\", soundThread, 0x40, 0x10000, 0, 0, NULL)\n", read_thread_uid);
	printf("0x%08X : sceKernelCreateThread(\"video thread\", monoraleThread, 0x40, 0x10000, 0, 0, NULL)\n", vid_thread_uid);
	printf("0x%08X : sceKernelCreateThread(\"sound thread\", soundThread, 0x40, 0x10000, 0, 0, NULL)\n", snd_thread_uid);

	if(vid_thread_uid < 0 || snd_thread_uid < 0 || read_thread_uid < 0){

		printf("Error  !!!\n");
		printf("Failed !!!\n");
		goto end;

	}else{
		sceKernelStartThread(read_thread_uid, 0, NULL);
		sceKernelStartThread(vid_thread_uid, sizeof(monorale_hdr), &hdr);
		sceKernelStartThread(snd_thread_uid, sizeof(int), &Audio_port);

		sceKernelWaitThreadEnd(read_thread_uid, 0, 0);
		sceKernelWaitThreadEnd(vid_thread_uid, 0, 0);
		sceKernelWaitThreadEnd(snd_thread_uid, 0, 0);
	}

end:
	monorale_deinit();
	sceKernelDelayThread(3 * 1000 * 1000);

	return 0;
}
