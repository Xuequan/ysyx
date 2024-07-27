/* all .text, .rodata, .data will be in flash
 * stack, heap in sram 
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
  .text : AT (ORIGIN(flash)) 
	{
		. = ALIGN(0x8);
		_text_start = .;
    *(entry)
    *(.text*)
		*(.copy_to_sram)
		. = ALIGN(0x8);
		_text_end = .;
  }  

	.rodata : AT (ORIGIN(flash) + SIZEOF(.text))  
	{
		. = ALIGN(0x8);
		_rodata_start = . ;
    *(.rodata*)
		*(.srodata.mainargs)
		. = ALIGN(0x8);
		_rodata_end   = . ;
  } 

  .data : AT (ORIGIN(flash) + SIZEOF(.text) + SIZEOF(.rodata))
	{
		. = ORIGIN(psram);
		. = ALIGN(0x8);
		_data_start = .;
    *(.data)
    *(.data*)
		*(.data.*)
		*(.sdata)
		*(.sdata.*)
		. = ALIGN(0x8);
		_data_end = .;
  }

  .bss (NOLOAD) :  
	{
		. = ALIGN(0x8);
    *(.bss*)
    *(.sbss*)
    *(.scommon)
		. = ALIGN(0x8);
  } 
}

_data_size = SIZEOF(.data);
_data_load_addr = LOADADDR(.data);

_text_load_addr = LOADADDR(.text);

_rodata_load_addr = LOADADDR(.rodata);

_bss_start = ADDR(.bss);
_bss_end = _bss_start + SIZEOF(.bss);

_heap_start = ALIGN(0x8) ;
_heap_start = ORIGIN (sram);
_heap_end = ORIGIN (sram) + LENGTH (sram) - 0x8000 ;

_stack_point = ALIGN(0x8) ;
_stack_pointer = ORIGIN (sram) + LENGTH (sram) ;
