#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

// 新增
void __am_screen_size(AM_SCREEN_SIZE_T* size) {
	uint32_t tmp = inl(VGACTL_ADDR); 
	size->width = (tmp & 0xffff0000) >> 16; 
	size->height= tmp & 0xffff ; 
}

void __am_gpu_init() {
	int i;

	AM_SCREEN_SIZE_T size;
	__am_screen_size(&size);
	int w = size.width; 
	int h = size.height; // todo

	uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
	for(i = 0; i < w * h; i++){
		fb[i] = i;
	}
	outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = 0, .height = 0,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}

