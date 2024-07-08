MEMORY 
	{
	 mrom(rx) : ORIGIN = 0X20000000, LENGTH = 4K
	 sram(rw!x) : ORIGIN = 0x0f000000, LENGTH = 8K
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
  } > mrom  :text

	.rodata : {
		_rodata_start = .;
    *(.rodata*)
		_rodata_end = .;
  } > mrom  
  /* } > mrom AT> sram */

	_rodata_size = SIZEOF(.rodata);
	_rodata_load_start = LOADADDR(.rodata);

	.data : {
		_data_start = .;
    *(.data)
		_data_end = .;
  } > sram :data

	_data_size = SIZEOF(.data);
	_data_load_start = LOADADDR(.data);

  .bss : {
		_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
		_bss_end = .;
  } > mrom AT> sram

	_bss_size = SIZEOF(.bss);
	_bss_load_start = LOADADDR(.bss);

	. = SIZEOF(.bss) + SIZEOF(.rodata) + SIZEOF(.data) ;
  _heap_start = ALIGN(0x1000);
	. = _heap_start;
	 /*. = _heap_start + 0xff0ffc; */
	. = _heap_end;
	/*
	_stack_top = ALIGN(0x1000);
	. = _stack_top + 0x8000;
	*/
	. = _stack_pointer;
  _end = .;
}
