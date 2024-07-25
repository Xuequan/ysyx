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

	/* . = SIZEOF_HEADERS; */
  /* _pmem_start and _entry_offset are defined in LDFLAGS */
  . = _pmem_start + _entry_offset;
  .text : 
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

  /*} > sram AT> flash */

	.data : AT (ORIGIN(psram))  
	{
		. = ALIGN(0x8);
		_data_start = .;
    *(.data*)
		. = ALIGN(0x8);
		_data_end = .;
  }

  .bss :  
	{
		. = ALIGN(0x8);
		. = ORIGIN(psram) + SIZEOF(.data);
    *(.bss*)
    *(.sbss*)
    *(.scommon)
		. = ALIGN(0x8);
  } > psram

}

_data_size = SIZEOF(.data);
_data_Load_addr = LOADADDR(.data);

_bss_start = ADDR(.bss);
_bss_end = _bss_start + SIZEOF(.bss);

_heap_start = ALIGN(0x8) ;
_heap_start = ORIGIN (psram) + SIZEOF (.data) + SIZEOF (.bss);
_heap_end = ORIGIN (psram) + LENGTH (psram) - 0x8000 ;

_stack_point = ALIGN(0x8) ;
_stack_pointer = ORIGIN (psram) + LENGTH (psram) ;
