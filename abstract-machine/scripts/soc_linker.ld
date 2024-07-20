MEMORY 
	{
	 mrom : ORIGIN = 0x20000000, LENGTH = 4K
	 sram : ORIGIN = 0x0f000000, LENGTH = 8K
	 flash : ORIGIN = 0x30000000, LENGTH = 256M
	}

ENTRY(_start)
PHDRS { 
				/*text PT_LOAD ; */
				data PT_LOAD ; 
			} 

SECTIONS {
	/* . = SIZEOF_HEADERS; */
  /* _pmem_start and _entry_offset are defined in LDFLAGS */
  . = _pmem_start + _entry_offset;
  .text : {
		. = ALIGN(0x8);
		_text_start = .;
    *(entry)
    *(.text*)
		. = ALIGN(0x8);
		_text_end = .;
  } > flash AT> flash

	.rodata ORIGIN (mrom) : {
		. = ALIGN(0x8);
		_rodata_start = . ;
    *(.rodata*)
		*(.srodata.mainargs)
		. = ALIGN(0x8);
		_rodata_end   = . ;
  } > mrom AT> mrom 

	.data ORIGIN (sram) : 
		/*AT (ADDR (.text) + SIZEOF (.rodata) + SIZEOF (.text) ) */
	{
		. = ALIGN(0x8);
		_data_start = .;
    *(.data*)
		. = ALIGN(0x8);
		_data_end = .;
  } > sram AT> mrom :data

  .bss : 
	{
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  } > sram
}

_data_size = SIZEOF(.data);
_data_Load_addr = LOADADDR(.data);

_bss_start = ADDR(.bss);
_bss_end = _bss_start + SIZEOF(.bss);

_heap_start = ALIGN(0x8) ;
_heap_start = ORIGIN (sram) + SIZEOF (.data) ;
_heap_end = ORIGIN (sram) + LENGTH (sram) - 0x8000 ;

_stack_point = ALIGN(0x8) ;
_stack_pointer = ORIGIN (sram) + LENGTH (sram) ;
