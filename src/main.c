#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include "monorale.h"
int main(int argc, char *argv[]) {
	//printf("Hello Vita!");
	monorale_hdr *hdr = monorale_init("app0:/monorale.bin");

	SceUID vid = sceKernelCreateThread("video thread", monoraleThread, 0x40, 0x10000, 0, 0, NULL);
	if(vid < 0) {
		printf("Thread Failed.");
	} else sceKernelStartThread(vid,sizeof(monorale_hdr),&hdr);
	sceKernelWaitThreadEnd(vid,0,0);

	return 0;
}
