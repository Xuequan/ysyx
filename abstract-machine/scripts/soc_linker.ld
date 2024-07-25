MEMORY 
	{
	 sram : ORIGIN = 0x0f000000, LENGTH = 8K
	 mrom : ORIGIN = 0x20000000, LENGTH = 4K
	 flash : ORIGIN = 0x30000000, LENGTH = 256M
	 psram : ORIGIN = 0x80000000, LENGTH = 4M
	}

ENTRY(_start)
PHDRS { 
				text PT_LOAD ; 
				data PT_LOAD ; 
			} 

SECTIONS {
  /* . = _pmem_start + _entry_offset; */
  .text : AT(ORIGIN(flash))
	{
		. = ALIGN(0x8);
		_text_start = .;
    *(entry)
    *(.text*)
		. = ALIGN(0x8);
		_text_end = .;
  } > flash :text

	.rodata : AT (ORIGIN(flash) + SIZEOF(.text)) 
	{
		. = ALIGN(0x8);
		_rodata_start = . ;
    *(.rodata*)
		*(.srodata.mainargs)
		. = ALIGN(0x8);
		_rodata_end   = . ;
  } > flash 

  .bss (NOLOAD) :  
	{
		. = ALIGN(0x8);
    *(.bss*)
    *(.sbss*)
    *(.scommon)
		/* KEEP (*(SORT(.bss*))) */
		. = ALIGN(0x8);
  } > flash AT> flash 

	/* 加上 NOLOAD 之后，生成的 ELF, .bin 文件都会没有该部分内容了。 */
  .data (NOLOAD) :  
	{
		. = ALIGN(0x8);
		_data_start = .;
    *(.data*)
		*(.sdata.*)
		. = ALIGN(0x8);
		_data_end = .;
  } > psram AT> flash
}

_data_size = SIZEOF(.data);
_data_Load_addr = LOADADDR(.data);

_bss_start = ADDR(.bss);
_bss_end = _bss_start + SIZEOF(.bss);

_heap_start = ALIGN(0x8) ;
_heap_start = ORIGIN (psram) + SIZEOF (.data);
_heap_end = ORIGIN (psram) + LENGTH (psram) - 0x8000 ;

_stack_point = ALIGN(0x8) ;
_stack_pointer = ORIGIN (psram) + LENGTH (psram) ;
