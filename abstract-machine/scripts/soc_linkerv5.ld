/* in order to finish "实现bootloader的二级加载过程”
 * initial start, fsbl(first stage bootloader), ssbl(second stage bootloader) 都位于 flash
 * first execute fsbl, which loading ssbl from flash to sram;
 * then, execute ssbl, which loading program from flash to psram
 */
MEMORY 
	{
	 sram : ORIGIN = 0x0f000000, LENGTH = 8K
	 mrom : ORIGIN = 0x20000000, LENGTH = 4K
	 flash : ORIGIN = 0x30000000, LENGTH = 256M
	 psram : ORIGIN = 0x80000000, LENGTH = 4M
	}

ENTRY(_start)
SECTIONS {
  . = _pmem_start + _entry_offset; 
  .fsbl :
  {
    *(.fsbl*)
  } > flash AT> flash

  .ssbl :
  {
    . = ALIGN(0x8) ;
    . = _sssbl ;
    *(.ssbl*)
    . = ALIGN(0x8) ;
    . = _essbl ;
  } > sram AT> flash

  .text :
	{
		. = ALIGN(0x8);
		_text_start = .;
    *(entry)
    *(.text*)
		KEEP(*(.zero_bss))
		. = ALIGN(0x8);
		_text_end = .;
  } > psram AT> flash 

  . = ORIGIN(sram) + SIZEOF(.text);
	.rodata : 
	{
		. = ALIGN(0x8);
		_rodata_start = . ;
    *(.rodata*)
		*(.srodata.mainargs)
		. = ALIGN(0x8);
		_rodata_end   = . ;
  } > psram AT> flash

  . = ORIGIN(sram) + SIZEOF(.text) + SIZEOF(.rodata);
  .data : 
	{
		. = ALIGN(0x8);
		_data_start = .;
    *(.data)
    *(.data*)
		*(.data.*)
		*(.sdata)
		*(.sdata.*)
		. = ALIGN(0x8);
		_data_end = .;
  } > psram AT> flash

  .bss (NOLOAD) :
	{
		. = ALIGN(0x8);
		_sbss = . ;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
		. = ALIGN(0x8);
		_ebss = . ;
	} > sram AT> flash

}

_data_size = SIZEOF(.data);
_data_load_addr = LOADADDR(.data);

_text_load_addr = LOADADDR(.text);

_rodata_load_addr = LOADADDR(.rodata);

_ssbl_load_addr = LOADADDR(.ssbl);

_heap_start = ALIGN(0x8) ;
_heap_start = ORIGIN (sram);
_heap_end = ORIGIN (sram) + LENGTH (sram) - 0x100;

_stack_point = ALIGN(0x8) ;
_stack_pointer = ORIGIN (sram) + LENGTH (sram) ;
