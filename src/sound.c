#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h>
#include <psp2/audioout.h>
#include <psp2/appmgr.h>
#include <vorbis/vorbisfile.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "sound.h"
#include "monorale.h"

#define printf sceClibPrintf

#define MAX_BUF_NUM 32

OggVorbis_File Ogg_VorbisFile;
static int16_t SndBuf[MAX_BUF_NUM][4096];
int read_flag[MAX_BUF_NUM];
int end_flag = 0;
int SndBuf_n = 0;
int sound_ready = 1;

int initVorbis(const char *path, OggVorbis_File *vorbisFile, vorbis_info **vi){
	int ov_res;
	FILE *f = fopen(path, "rb");
	if (f == NULL) return -1;
	ov_res = ov_open(f, vorbisFile, NULL, 0);
	if(ov_res < 0) {
		fclose(f);
		printf("ov_res %d\n", ov_res);
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

int SoundReady(){
	printf("SoundReady()\n");
	while(sound_ready){}

	sceKernelDelayThread(700 * 1000);
	printf("SoundReady() end\n");

	return 0;
}

int SoundPlayGo(){
	printf("SoundPlayGo()\n");
	sound_ready = 0;

	return 0;
}

int readThread(SceSize argc, void* argv){

	printf("ReadThread. I'm in.\n");

	while(1) {
		for(int i=0;i<MAX_BUF_NUM;i++){
			if(read_flag[i] == 0){
				continue;
			}

			read_flag[i] = 0;
			if(fillBuf((int16_t*)SndBuf[i], 4096, &Ogg_VorbisFile) == 0){
				goto end;
			}
		}
	}

end:
	end_flag = 1;

	return 0;
}

int soundThread(SceSize argc, void* argv) {

	vorbis_info *info = NULL;
	int port = *(int*)argv;

	printf("soundThread I'm in.\n");
	printf("port : 0x%X\n", port);

	for(int i=0;i<MAX_BUF_NUM;i++)read_flag[i] = 1;

	if(initVorbis("app0:/badapple.ogg", &Ogg_VorbisFile, &info) < 0) {
		printf("Error on InitVorbis\n");
		goto end;
	}

	sceAppMgrAcquireBgmPort();
	sceAudioOutOutput(port, NULL);

	printf("=SongInfo=\n");
	printf("Version %d\n", info->version);
	printf("Channels %d\n", info->channels);
	printf("Rate %ld\n", info->rate);

	// Extended info
	vorbis_comment *comment = ov_comment(&Ogg_VorbisFile, -1);
	for(int i=0;i<comment->comments;i++) printf("%s\n", comment->user_comments[i]);

	VideoReady();
	SoundPlayGo();

	while(1) {
		if(end_flag == 1) break;

		sceAudioOutOutput(port, SndBuf[SndBuf_n]);

		read_flag[SndBuf_n] = 1;

		SndBuf_n++;

		if(SndBuf_n >= MAX_BUF_NUM)SndBuf_n = 0;

	}

	sceAppMgrReleaseBgmPort();

	sceAudioOutReleasePort(port);

end:
	return 0;
}
