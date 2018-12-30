#include <psp2/display.h> 
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>

#include "sound.h"
#include "monorale.h"

#define LOG_FILE "ux0:/data/monorale_video.log"

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 368
//#define sceClibPrintf LOG
#define printf LOG
#define sceClibMemset memset // emulator aware.
static SceUID fdata_memblock = 0;
static SceUID fb_memblock = 0;

int video_ready = 1;

void LOG(char* format, ...){
	char str[512] = { 0 };
	va_list va;

	va_start(va, format);
	vsnprintf(str, 512, format, va);
	va_end(va);
	
	SceUID fd;
	fd = sceIoOpen(LOG_FILE, SCE_O_WRONLY | SCE_O_APPEND | SCE_O_CREAT, 0777);
	sceIoWrite(fd, str, strlen(str));
	sceIoClose(fd);
//#undef sceClibPrintf
	sceClibPrintf(str); // callback
}

static inline uint16_t *monorale_frame(monorale_hdr *hdr, size_t frame)
{
	monorale_frameinf *info;
	if (frame >= hdr->framecnt) return NULL;
	info = &hdr->inf[frame];
	return (uint16_t*)((char*)hdr + info->offset);
}

static inline uint32_t monorale_framecnt(monorale_hdr *hdr, size_t frame)
{
	if (frame >= hdr->framecnt) return 0;
	return hdr->inf[frame].cmdcnt;
}

/* TODO: Add hardware filling */
void monorale_doframe(monorale_hdr *hdr, size_t frame, uint32_t *fb)
{
	uint32_t fill_val = 0;
	uint16_t *cmds = monorale_frame(hdr, frame);
	size_t cmdcnt = monorale_framecnt(hdr, frame);
	for (size_t i = 0; i < cmdcnt; i++) {
		size_t len_px = cmds[i];
		memset(fb, fill_val, len_px << 2);
		fb += len_px;
		fill_val ^= 0xFFFFFFFF; 
	}
}

monorale_hdr *monorale_init(const char *path)
{
	// Allocate needed memory
	monorale_hdr *monorale_base = NULL;
	SceIoStat stat;
	uint32_t memBlockSize = 0;
	SceUID fd = 0;

	if(sceIoGetstat(path,&stat) < 0) {
		return NULL;
	}

	sceClibPrintf("%s=%lldB\n",path,stat.st_size);

	memBlockSize = (((uint32_t)stat.st_size & 0xFFFFF000)+0x1000);

	sceClibPrintf("Need Allocate 0x%lXB\n",memBlockSize);

	if(allocMemory(memBlockSize) < 0) {
		sceClibPrintf("memory allocation failed???\n");
		return NULL;
	}

	sceKernelGetMemBlockBase(fdata_memblock,(void**)&monorale_base);

	fd = sceIoOpen(path, SCE_O_RDONLY, 0777);
	if(fd < 0) {
		sceClibPrintf("File Opening failed!???\n");
		sceClibPrintf("Error code : 0x%X\n", fd);
		return NULL;
	}
	sceIoRead(fd,monorale_base, stat.st_size);
	sceIoClose(fd);

	return monorale_base;
	
}

int allocMemory(uint32_t monolen) {
	sceClibPrintf("Allocating Memory right now.\n");

	SceKernelAllocMemBlockOpt fbmOpt;

	memset(&fbmOpt, 0, sizeof(SceKernelAllocMemBlockOpt)); //Nullify

	fbmOpt.size = sizeof(fbmOpt);
	fbmOpt.attr = 4;
	fbmOpt.alignment = 256*1024;

	fb_memblock = sceKernelAllocMemBlock("FrameBuffer CDRAM", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, 0x200000, &fbmOpt);

	sceClibPrintf("fb_memblockL 0x%x\n", fb_memblock);

	if(fb_memblock < 0){
		return fb_memblock;
	}

	fdata_memblock = sceKernelAllocMemBlock("FrameData UMEM", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, monolen, NULL);

	sceClibPrintf("fdata_memblock 0x%x\n", fdata_memblock);

	if(fdata_memblock < 0){
		return fdata_memblock;
	}

	return 0;
}


int monorale_deinit(){
	printf("monorale_deinit()\n");

	if(fb_memblock > 0){
		sceKernelFreeMemBlock(fb_memblock);
	}

	if(fdata_memblock > 0){
		sceKernelFreeMemBlock(fdata_memblock);
	}

	return 0; // always success.
}

int VideoReady(){
	printf("VideoReady()\n");
	while(video_ready){
	}

	printf("VideoReady() end\n");

	return 0;
}

int VideoPlayGo(){
	printf("VideoPlayGo()\n");
	video_ready = 0;

	return 0;
}

int monoraleThread(SceSize args, void *argp) {

	uint32_t frame = 0;
	uint32_t all_frame = 0;

	SceDisplayFrameBuf buf;

	monorale_hdr **tmp = NULL;
	monorale_hdr *hdr = NULL;

	printf("I'm in.\n");

	memset(&buf, 0, sizeof(SceDisplayFrameBuf));

	sceKernelGetMemBlockBase(fb_memblock, &buf.base);

       	buf.size = sizeof(SceDisplayFrameBuf);
	buf.pitch = 640;
	buf.pixelformat = 0;
	buf.width = 640;
	buf.height = 368;

	tmp = (monorale_hdr**)argp;
	hdr = *tmp;

	all_frame = monorale_frames(hdr);

	printf("Render Lv2\n");
	printf("%d frames\n", all_frame);
	printf("baseaddr=%p\n", buf.base);

	VideoPlayGo();
	SoundReady();

	while(frame < all_frame) {
		monorale_doframe(hdr, frame, buf.base);
		sceDisplaySetFrameBuf(&buf, 1);
		frame++;
		sceDisplayWaitVblankStart(); //problematic on Vita3K.
	}
	return 0;
}

