#include <am.h>
#include <npc.h>

void __am_timer_init() {
	outl(RTC_ADDR, 0);
	outl(RTC_ADDR + 4, 0);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
<<<<<<< HEAD
  uptime->us = inl(RTC_ADDR + 4);
	uptime->us <<= 32;
	uptime->us += inl(RTC_ADDR);
=======
  uptime->us = inl(RTC_ADDR + 4) * 2/1000;
	uptime->us <<= 32;
	uptime->us += inl(RTC_ADDR) * 2/1000;
>>>>>>> tracer-ysyx
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
