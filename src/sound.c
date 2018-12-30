#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/audioout.h>
#include <vorbis/vorbisfile.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "sound.h"

int initVorbis(const char *path, OggVorbis_File *vorbisFile, vorbis_info **vi) {
	int ov_res;
	FILE *f = fopen(path, "rb");
	if (f==NULL) return -1;
	ov_res = ov_open(f,vorbisFile, NULL, 0);
	if(ov_res < 0) {
		fclose(f);
		printf("ov_res %d\n",ov_res);
		return ov_res;
	}
	*vi = ov_info(vorbisFile, -1);
	if (*vi == NULL) return -3;
	return 0;
}

size_t fillBuf(int16_t *buf, size_t samples, OggVorbis_File *f) {
	size_t sampleRead = 0;
	int samplesToRead = samples;
	while(samplesToRead > 0) {
		int current_section;
		int rb = ov_read(f,(char*)buf, samplesToRead > 4096?4096:samplesToRead,0,2,1,&current_section);
		if(rb < 0) return rb;
		else if(rb == 0) break; // EoF
		sampleRead += rb;
		samplesToRead -= rb;
		buf += rb/2; //16bit
	}
	return sampleRead/sizeof(int16_t); // How much file remained.
}

int soundThread(SceSize argc, void* argv) {
	printf("SOUND. I'm in.\n");
	OggVorbis_File *f = NULL;
	vorbis_info *info = NULL;
	int port = (int)argv;
	if(initVorbis("app0:/badapple.ogg",f,&info) < 0) {
		printf("Error on InitVorbis\n");
		goto end;
	}

	printf("=SongInfo=\n");
	printf("Version %d\n",info->version);
	printf("Channels %d\n",info->channels);
	printf("Rate %ld\n",info->rate);
	// Extended info
	vorbis_comment *comment = ov_comment(f,-1);
	for(int i=0;i<comment->comments;i++) printf("%s\n",comment->user_comments[i]);

	goto end;

	//int16_t *samplebuf; //workram in heap
	void *wvbuf; // SCE RAM
	SceUID uid = sceKernelAllocMemBlock("SndBuf",0x0D808060,4096*sizeof(int16_t),NULL);
	sceKernelGetMemBlockBase(uid,&wvbuf);	
	while(1) {
		int i = fillBuf((int16_t*)wvbuf,4096,f);
		if(i == 0) break;
		sceAudioOutOutput(port,wvbuf);
	}

end:
	return 0;
}
