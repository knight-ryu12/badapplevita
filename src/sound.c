#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/audioout.h>
#include <vorbis/vorbisfile.h>
#include "sound.h"

int initVorbis(const char *path, OggVorbis_File *vorbisFile, vorbis_info **vi) {
	int ov_res;
	FILE *f = fopen(path, "rb");
	if (f==NULL) return -1;
	ov_res = ov_open(f,vorbisFile, NULL, 0);
	if(ov_res < 0) {
		fclose(f);
		return -2;
	}
	*vi = ov_info(vorbisFile, -1);
	if (*vi == NULL) return -3;
	return 0;
}

size_t fillBuf(int16_t *buf, size_t samples, OggVorbis_File *f) {
	
}

int soundThread(SceSize argc, void* argv) {
	OggVorbis_File *f = NULL;
	vorbis_info *info = NULL;
	int port = (int)argv;
	initVorbis("badapple.ogg",f,&info);
	int16_t *samplebuf; //workram in heap
	void *wvbuf[2]; // SCE RAM
	SceUID uidL = sceKernelAllocMemBlock("Left Channel",0x0D808060,4096*sizeof(int16_t),NULL);
	SceUID uidR = sceKernelAllocMemBlock("Right Channel",0x0D808060,4096*sizeof(int16_t),NULL);
	sceKernelGetMemBlockBase(uidL,&wvbuf[0]);
	sceKernelGetMemBlockBase(uidR,&wvbuf[1]);
	
	


	return 0;
}
