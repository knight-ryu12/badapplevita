#include <psp2/display.h> 
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>

#include "monorale.h"

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 368
#define sceClibPrintf printf
#define sceClibMemset memset // emulator aware.
static SceUID fdata_memblock = 0;
static SceUID fb_memblock = 0;

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
	//printf("Doing Frame Job!\n");
	uint32_t fill_val = 0;
	uint16_t *cmds = monorale_frame(hdr, frame);
	size_t cmdcnt = monorale_framecnt(hdr, frame);
	//printf("%d\n",cmdcnt);
	for (size_t i = 0; i < cmdcnt; i++) {
		size_t len_px = cmds[i];
		memset(fb, fill_val, len_px << 1);
		//printf("memset\n");
		fb += len_px;
		fill_val ^= 0x00FFFFFF; 
	}
	//printf("Frame %d render!\n",frame);
}

monorale_hdr *monorale_init(const char *path)
{
	// Allocate needed memory
	monorale_hdr *monorale_base = NULL;
	SceIoStat stat;
	uint32_t memBlockSize = 0;
	if(sceIoGetstat(path,&stat) < 0) {
		return NULL;
	}
	sceClibPrintf("%s=%lldB\n",path,stat.st_size);
	memBlockSize = (((uint32_t)stat.st_size & 0xFFFFF000)+0x1000);
	sceClibPrintf("Need Allocate 0x%lXB\n",memBlockSize);
	if(allocMemory(memBlockSize) < 0) {
		// Alloc Failed!!!
		sceClibPrintf("memory allocation failed???\n");
		return NULL;
	}
	sceKernelGetMemBlockBase(fdata_memblock,(void**)&monorale_base);
	//sceClibPrintf("memblock_base 0x%X",monorale_base);
	SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0777);
	if(fd < 0) {
		sceClibPrintf("File Opening failed!???\n");
		return NULL;
	}
	sceIoRead(fd,monorale_base, stat.st_size);
	sceIoClose(fd);
	return monorale_base;
	
}

int allocMemory(uint32_t monolen) {
	sceClibPrintf("Allocating Memory right now.\n");
	SceKernelAllocMemBlockOpt fbmOpt;
	memset(&fbmOpt,0,sizeof(SceKernelAllocMemBlockOpt)); //Nullify
	fbmOpt.size = sizeof(fbmOpt);
	fbmOpt.attr = 4;
	fbmOpt.alignment = 256*1024;
	fb_memblock = sceKernelAllocMemBlock("FrameBuffer CDRAM",SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,DISPLAY_WIDTH*DISPLAY_HEIGHT*4, &fbmOpt);
	sceClibPrintf("fb_memblock 0x%x\n",fb_memblock);
	fdata_memblock = sceKernelAllocMemBlock("FrameData UMEM",SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,monolen, NULL);
	sceClibPrintf("fdata_memblock 0x%x\n",fdata_memblock);
	if(fb_memblock < 0) {
		return fb_memblock;
	}
	if(fdata_memblock < 0) {
		return fdata_memblock;
	}
	return 0;
}


int monorale_deinit(){
	if(fb_memblock > 0) {
		sceKernelFreeMemBlock(fb_memblock);
	}
	if(fdata_memblock > 0) {
		sceKernelFreeMemBlock(fdata_memblock);
	}
	return 0; // always success.
}



int monoraleThread(SceSize args, void *argp) {
	printf("I'm in.\n");
	void* base;
	sceKernelGetMemBlockBase(fb_memblock,&base);
	SceDisplayFrameBuf buf;
	memset(&buf,0, sizeof(SceDisplayFrameBuf));
       	buf.size = sizeof(SceDisplayFrameBuf);
	buf.base = base;
	buf.pitch = DISPLAY_WIDTH;
	buf.pixelformat = 0;
	buf.width = DISPLAY_WIDTH;
	buf.height = DISPLAY_HEIGHT;
	//buf.height = 640;
	//sceDisplaySetFrameBuf(&buf,SCE_DISPLAY_SETBUF_IMMEDIATE);

	int frame = 0;
	monorale_hdr **tmp = (monorale_hdr**)argp;
	monorale_hdr *hdr = *tmp;
	printf("Render Lv2\n");
	printf("%d frames\n",monorale_frames(hdr));
	printf("baseaddr=%p\n",base);
	while(frame < monorale_frames(hdr)) {
		//printf("f:%d\n",frame);
		monorale_doframe(hdr,frame,(uint32_t*)buf.base);
		if(sceDisplaySetFrameBuf(&buf,0)){
			printf("Error on sceDisplaySetFrameBuf()\n");
			break;
		}
		frame++;
		//sceDisplayWaitSetFrameBuf();
		sceDisplayWaitVblankStart(); //problematic on Vita3K.
	}
	return 0;
}

