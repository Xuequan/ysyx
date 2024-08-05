/* 
 * initial start, fsbl(first stage bootloader), ssbl(second stage bootloader) 都位于 flash
 * first execute fsbl, which loading ssbl from flash to sram;
 * then, execute ssbl, which loading program from flash to psram
 * 将程序加载到 sdram 中运行；注意，此时只有一个SDRAM 颗粒，因此其容量是 32M
 */
MEMORY 
	{
	 sram : ORIGIN = 0x0f000000, LENGTH = 8K
	 mrom : ORIGIN = 0x20000000, LENGTH = 4K
	 flash : ORIGIN = 0x30000000, LENGTH = 256M
	 psram : ORIGIN = 0x80000000, LENGTH = 4M
   sdram : ORIGIN = 0Xa0000000, LENGTH = 32M
	}

ENTRY(_start)
SECTIONS {
  . = _pmem_start + _entry_offset; 
  .fsbl :
  {
		. = ALIGN(0x8);
    *(entry)
    *(.fsbl*)
		. = ALIGN(0x8);
  } > flash AT> flash

  .ssbl :
  {
    . = ALIGN(0x8) ;
    _sssbl = .  ;
    *(.ssbl*)
    . = ALIGN(0x8) ;
    _essbl = . ;
  } > sram AT> flash

  . = ORIGIN(sdram);
  .text :
	{
		. = ALIGN(0x8);
		_text_start = .;
    *(.text*)
		. = ALIGN(0x8);
		_text_end = .;
  } > sdram AT> flash 

  . = ORIGIN(sdram) + SIZEOF(.text);
	.rodata : 
	{
		. = ALIGN(0x8);
		_rodata_start = . ;
    *(.rodata*)
		*(.srodata.mainargs)
		*(.srodata*)
		. = ALIGN(0x8);
		_rodata_end   = . ;
  } > sdram AT> flash

  /* . = ORIGIN(sdram) + SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data.extra); */
  . = ORIGIN(sdram) + SIZEOF(.text) + SIZEOF(.rodata);
  .data : 
	{
		. = ALIGN(0x8);
		_data_start = .;
		*(.data.extra*)
    *(.data*)
		*(.data.*)
		*(.sdata)
		*(.sdata.*)
		. = ALIGN(0x8);
		_data_end = .;
  } > sdram AT> flash

  .bss (NOLOAD) :
	{
		. = ALIGN(0x8);
		_sbss = . ;
    *(.bss*)
		*(.bss.extra*)
    *(.sbss*)
    *(.scommon)
		. = ALIGN(0x8);
		_ebss = . ;
	} > sdram AT> flash
}

/* some extern address for date moving */
_data_size = SIZEOF(.data);
_data_load_addr = LOADADDR(.data);
_text_load_addr = LOADADDR(.text);
_rodata_load_addr = LOADADDR(.rodata);
_ssbl_load_addr = LOADADDR(.ssbl);

/* heap memory space */
_heap_start = ORIGIN(sdram) + SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data) + SIZEOF(.data.extra) ;
_heap_start = ALIGN(0x1000) ;
_heap_end = ORIGIN (sdram) + LENGTH (sdram) ;
/*
_heap_start = ALIGN(0x1000) ;
_heap_start = ORIGIN(psram);
_heap_end = ORIGIN (psram) + LENGTH (psram) ;
*/

/* stack space */
_stack_point = ALIGN(0x1000) ;
_stack_pointer = ORIGIN (sram) + LENGTH (sram) ;
