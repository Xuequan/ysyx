MEMORY 
	{
	 mrom : ORIGIN = 0X20000000, LENGTH = 4K
	 sram : ORIGIN = 0x0f000000, LENGTH = 8K
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
  .text : {
		_text_start = .;
    *(entry)
    *(.text*)
		_text_end = .;
  } > mrom :text

	.rodata : {
		_rodata_start = . ;
    *(.rodata*)
		_rodata_end   = . ;
  } > mrom

	.data ORIGIN (sram) : 
		/*AT (ADDR (.text) + SIZEOF (.rodata) + SIZEOF (.text) ) */
	{
		_data_start = .;
    *(.data)
		_data_end = .;
  } > sram AT> mrom :data

  .bss : 
	{
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  } > sram

  _heap_start = ALIGN(0x1000);
	_heap_start = 0x0f000000 + SIZEOF (.data) ;
	 /*. = _heap_start + 0xff0ffc; */
	/*
	. = _heap_end;
	_stack_top = ALIGN(0x1000);
	. = _stack_end;
	. = _stack_pointer;
	*/
	/*
	_stack_top = ALIGN(0x1000);
	. = _stack_top + 0x8000;
	. = _stack_pointer;
  _end = .;
	*/
}

_data_size = SIZEOF(.data);
_data_Load_addr = LOADADDR(.data);

_bss_start = ADDR(.bss);
_bss_end = _bss_start + SIZEOF(.bss);
