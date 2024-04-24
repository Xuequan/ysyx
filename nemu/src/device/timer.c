/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <device/map.h>
#include <device/alarm.h>
#include <utils.h>

static uint32_t *rtc_port_base = NULL;

static void rtc_io_handler(uint32_t offset, int len, bool is_write) {
  assert(offset == 0 || offset == 4);
	/*
  if (!is_write && offset == 4) {
    uint64_t us = get_time();
    rtc_port_base[0] = (uint32_t)us;
    rtc_port_base[1] = us >> 32;
  }
	*/
  if (!is_write && offset == 4) {
    uint64_t now_time = get_time();
		uint64_t start_time = *(uint64_t *)rtc_port_base;
		/*
		uint32_t init_time[2] = {0};
		init_time[0] = rtc_port_base[0];
		init_time[1] = rtc_port_base[1];
		memset(&start_time, init_time, sizeof(uint64_t));
		*/
		uint64_t passed_time = now_time - start_time;
    rtc_port_base[0] = (uint32_t)passed_time;
    rtc_port_base[1] = passed_time >> 32;
  }
}

#ifndef CONFIG_TARGET_AM
static void timer_intr() {
  if (nemu_state.state == NEMU_RUNNING) {
    extern void dev_raise_intr();
    dev_raise_intr();
  }
}
#endif

void init_timer() {
  rtc_port_base = (uint32_t *)new_space(8);
	// try
	// record system start time
  uint64_t us = get_time();
  rtc_port_base[0] = (uint32_t)us;
  rtc_port_base[1] = us >> 32;

#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("rtc", CONFIG_RTC_PORT, rtc_port_base, 8, rtc_io_handler);
#else
  add_mmio_map("rtc", CONFIG_RTC_MMIO, rtc_port_base, 8, rtc_io_handler);
#endif
  IFNDEF(CONFIG_TARGET_AM, add_alarm_handle(timer_intr));
}
