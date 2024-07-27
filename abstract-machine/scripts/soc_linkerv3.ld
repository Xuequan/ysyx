/* in order to finish "完整测试 PSRAM 的访问”
 * put mem-test all in SRAM;
 * also need to put .rodata in SRAM
 * This verison is based on soc_linkerv2.ld
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
  .text :
	{
		. = ALIGN(0x8);
		_text_start = .;
    *(entry)
    *(.text*)
		KEEP(*(.copy_to_psram))
		KEEP(*(.zero_bss))
		. = ALIGN(0x8);
		_text_end = .;
  } > sram AT> flash 

  . = ORIGIN(sram) + SIZEOF(.text);
	.rodata : 
	{
		. = ALIGN(0x8);
		_rodata_start = . ;
    *(.rodata*)
		*(.srodata.mainargs)
		. = ALIGN(0x8);
		_rodata_end   = . ;
  } > sram AT> flash

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
  } > sram AT> flash

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

_heap_start = ALIGN(0x8) ;
_heap_start = ORIGIN (sram) + SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data);
_heap_end = ORIGIN (sram) + LENGTH (sram) - 0x100 ;

_stack_point = ALIGN(0x8) ;
_stack_pointer = ORIGIN (sram) + LENGTH (sram) ;
