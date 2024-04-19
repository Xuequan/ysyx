#include <am.h>
#include <nemu.h>
#include <string.h>
//#include "mmio.h"

void __am_timer_init() {
	
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
	uint32_t start_time[2] = {0};
	//printf("here start\n");
	start_time[0] = inl(RTC_ADDR);	
	//start_time[0] = mmio_read(RTC_ADDR, 4);
	//printf("start_time[0] = %#x\n", start_time[0]);
	start_time[1] = inl(RTC_ADDR + 4);		
	//start_time[1] = inl(rtc_port_base[1]);		
	//printf("start_time[1] = %#x\n", start_time[1]);
	uint64_t tmp = 0;
	memcpy(&tmp, start_time, 2 * sizeof(start_time[0]) );
	uptime->us = tmp;
	/*
	uint32_t start_time[2] = {0};
	start_time[0] = mmio_read(RTC_ADDR, 4);
	start_time[1] = mmio_read(RTC_ADDR + 4, 4);
	uint64_t tmp = 0;
	memcpy(&tmp, start_time, 2 * sizeof(start_time[0]) );
	uptime->us = tmp;
	*/
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
