#include <am.h>
#include <nemu.h>
#include <string.h>

//#include <stdio.h>
void __am_timer_init() {
	//printf("am_timer_init()\n");
	outl(RTC_ADDR, 0);
	outl(RTC_ADDR + 4, 0);	
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
	uint32_t start_time[2] = {0};
	start_time[1] = inl(RTC_ADDR + 4);		
	start_time[0] = inl(RTC_ADDR);	
	uint64_t tmp = 0;
	memcpy(&tmp, start_time, 2 * sizeof(start_time[0]) );
	uptime->us = tmp;
	//printf("uptime->us = %lld\n", uptime->us);
	/*
	// 下面也可以
	uptime->us = inl(RTC_ADDR + 4);
	uptime->us <<= 32;
	uptime->us += inl(RTC_ADDR);
	*/
}
/*
#include <memory/paddr.h>
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
	uint32_t start_time[2] = {0};
	start_time[0] = paddr_read(RTC_ADDR, 4);	
	start_time[1] = paddr_read(RTC_ADDR + 4, 4);		
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
