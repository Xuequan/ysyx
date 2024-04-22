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

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }else{
		uintptr_t fb = (uintptr_t)FB_ADDR;
		for(uint32_t i = 0; i < ctl->w * ctl->h; i++){
			outl( (fb + i), (uint32_t)((uint32_t *)ctl->pixels + i) );
		}
	}
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}

void __am_vga_sync(AM_VGA_SYNC_T *vga_sync) {
	vga_sync->sync = inl(SYNC_ADDR); 	
}
