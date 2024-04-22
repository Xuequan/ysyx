#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

// 新增
void __am_vga_size(AM_VGA_SIZE_T* size) {
	uint32_t tmp = inl(VGACTL_ADDR); 
	size->width = (tmp & 0xffff0000) >> 16; 
	size->height= tmp & 0xffff ; 
}

void __am_gpu_init() {
/*
	int i;
	AM_VGA_SIZE_T size;
	__am_vga_size(&size);
	int w = size.width; 
	int h = size.height;

	uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
	for(i = 0; i < w * h; i++){
		fb[i] = i;
	}
	outl(SYNC_ADDR, 1);
*/
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
	
	AM_VGA_SIZE_T size;
	__am_vga_size(&size);
	int w = size.width; 
	int h = size.height;

  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = w * h
  };
}

/* AM_CPU_FBDRAW, AM 帧缓冲控制器
** AM_DEVREG(11, GPU_FBDRAW,   WR, int x, y; void *pixels; int w, h; bool sync);
** 向屏幕 (x,y) 坐标处绘制 w*h 的矩形图像，图像像素按行优先方式存储在
** pixels 中，每个像素用32bit 整数以 00RRGGBB 的方式描述颜色。若 sync
** 为 true, 则马上将帧缓冲中的内容同步到屏幕上。
*/
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }else{
		AM_GPU_CONFIG_T cfg;
		__am_gpu_config(&cfg);
		int width = cfg.width;   // screen width
		//int height = cfg.height; // screen height
		int x = ctl->x;          // screen x position
		int y = ctl->y;

		uintptr_t block_st = (uintptr_t)FB_ADDR + (uintptr_t)(x + width * y);
		uintptr_t addr;
		uint32_t data;
		
		int block_w = ctl->w;
		int block_h = ctl->h;
		uint32_t *block_d = (uint32_t *)ctl->pixels;
		
		for(int i = 0; i < block_h; i++) {
			for(int j = 0; j < block_w; j++) {
				addr = block_st + (uintptr_t)(i * width + j);
				data = *(block_d + i * block_w + j);
				outl(addr, data);	
			}
		}
	} //end else
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}

void __am_vga_sync(AM_VGA_SYNC_T *vga_sync) {
	vga_sync->sync = inl(SYNC_ADDR); 	
}
