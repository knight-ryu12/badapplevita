#include <psp2/display.h> 
#include <psp2/kernel/sysmem.h>
#include "monorale.h"

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 368

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
void monorale_doframe(monorale_hdr *hdr, size_t frame, uint16_t *fb)
{
	printf("Doing Frame Job!\n");
	uint32_t fill_val = 0;
	uint16_t *cmds = monorale_frame(hdr, frame);
	size_t cmdcnt = monorale_framecnt(hdr, frame);
	printf("%d\n",cmdcnt);
	for (size_t i = 0; i < cmdcnt; i++) {
		size_t len_px = cmds[i];

		memset(fb, fill_val, len_px << 1);
		//printf("memset\n");
		fb += len_px;
		fill_val ^= 0xFFFFFFFF;
	}
	printf("Frame %d render!\n",frame);
}

monorale_hdr *monorale_init(const char *path)
{
	printf("Monorale INIT\n");
	size_t monolen;
	monorale_hdr *ret;
	FILE *f = fopen(path, "rb");
	if (f == NULL) return NULL;

	fseek(f, 0L, SEEK_END);
	monolen = ftell(f);
	fseek(f, 0L, SEEK_SET);
	printf("Hello %08x\n",monolen);
	ret = malloc(monolen);
	if (ret != NULL)
		fread(ret, monolen, 1, f);

	fclose(f);
	return ret;
}

int monoraleThread(SceSize args, void *argp) {
	printf("I'm in.\n");
	void* base;
	SceKernelAllocMemBlockOpt opt = {0};
	opt.size = sizeof(opt);
	opt.attr = 0x00000004;
	opt.alignment = DISPLAY_WIDTH*DISPLAY_HEIGHT*2;
	SceUID memb = sceKernelAllocMemBlock("fb",SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,0x200000,&opt);
	sceKernelGetMemBlockBase(memb,&base);
	printf("malloc completed!\n");
	
	SceDisplayFrameBuf buf;
	memset(&buf,0, sizeof(SceDisplayFrameBuf));
       	buf.size = sizeof(SceDisplayFrameBuf);
	buf.base = base;
	buf.pitch = 640;
	buf.pixelformat = 0;
	buf.width = DISPLAY_WIDTH;
	buf.height = DISPLAY_HEIGHT;
	//sceDisplaySetFrameBuf(&buf,SCE_DISPLAY_SETBUF_IMMEDIATE);

	int frame = 0;
	monorale_hdr **tmp = (monorale_hdr**)argp;
	monorale_hdr *hdr = *tmp;
	printf("Render Lv2\n");
	printf("%d frames\n",monorale_frames(hdr));
	printf("baseaddr=%p\n",base);
	while(frame < monorale_frames(hdr)) {
		printf("frame start\n");
		//sceDisplayWaitVblankStart();
		//sceDisplayWaitSetFrameBuf();
		//sceDisplayGetFrameBuf(buf,SCE_DISPLAY_SETBUF_IMMEDIATE);
		monorale_doframe(hdr,frame,(uint16_t*)base);
		printf("frame monorale\n");
		sceDisplaySetFrameBuf(&buf,0);
		frame++;
		//sceDisplayWaitSetFrameBuf();
	}

	return 0;
}

