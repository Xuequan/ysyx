#include <am.h>
#include <nemu.h>
#include <string.h>

void __am_timer_init() {
	
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
	uint32_t start_time[2] = {0};
	start_time[0] = inl(RTC_ADDR);	
	start_time[1] = inl(RTC_ADDR + 4);		
	uint64_t tmp = 0;
	memcpy(&tmp, start_time, 2 * sizeof(start_time[0]) );
	uptime->us = tmp;
	//printf("uptime->us = %#lx\n", uptime->us);
}
/*
IOMap* fetch_mmio_map(paddr_t addr);
word_t map_read(paddr_t addr, int len, IOMap *map);
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
	IOMAP *map = fetch_mmio_map(RTC_ADDR);
	if (map == NULL) {
		panic("__am_timer_uptime(): cannot get map");
	}
	word_t start_time[2] = {0};
	word_t timer[0] = map_read(RTC_ADDR, 4, map);		 
	word_t timer[1] = map_read(RTC_ADDR + 4, 4, map);		 
	uint64_t tmp = 0;
	memcpy(&tmp, start_time, 2 * sizeof(start_time[0]) );
	uptime->us = tmp;
	printf("uptime->us = %#lx\n", uptime->us);
}
*/

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
