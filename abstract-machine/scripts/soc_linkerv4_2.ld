/* in order to finish "通过 bootloader 将程序加载到 PSRAM中执行”
 * 执行 microbench
 * 栈放在 sram
 * This verison is based on soc_linkerv4.ld
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
	  . = ALIGN(0x8);
    *(entry)
    *(.fsbl*)    
		. = ALIGN(0x8);
  } > flash 

	. = ORIGIN(psram) ;
  .text :
	{
		. = ALIGN(0x8);
		_text_start = .;
    *(.text*)
		. = ALIGN(0x8);
		_text_end = .;
  } > psram AT> flash 

  . = ORIGIN(psram) + SIZEOF(.text);
	.rodata : 
	{
		. = ALIGN(0x8);
		_rodata_start = . ;
    *(.rodata*)
		*(.srodata.mainargs)
		. = ALIGN(0x8);
		_rodata_end   = . ;
  } > psram AT> flash

  . = ORIGIN(psram) + SIZEOF(.text) + SIZEOF(.rodata);
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
	} > psram AT> flash
}
_data_size = SIZEOF(.data);
_data_load_addr = LOADADDR(.data);
_text_load_addr = LOADADDR(.text);
_rodata_load_addr = LOADADDR(.rodata);
_heap_start = ALIGN(0x8) ;
_heap_start = ORIGIN (sram);
_heap_end = ORIGIN (sram) + LENGTH (sram) - 0x100;

_stack_point = ALIGN(0x8) ;
_stack_pointer = ORIGIN (sram) + LENGTH (sram) ;
