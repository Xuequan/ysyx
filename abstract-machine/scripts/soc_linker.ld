MEMORY 
	{
	 mrom : ORIGIN = 0X20000000, LENGTH = 4K
	 sram : ORIGIN = 0x0f000000, LENGTH = 8K
	}

ENTRY(_start)
/*
PHDRS { 
				text PT_LOAD ; 
				data PT_LOAD ; 
			} 
*/

SECTIONS {
  /* _pmem_start and _entry_offset are defined in LDFLAGS */
  . = _pmem_start + _entry_offset;
  .text : {
		_text_start = .;
    *(entry)
    *(.text*)
		_text_end = .;
  } > mrom AT> mrom
	.rodata : {
		_rodata_start = .;
    *(.rodata*)
		_rodata_end = .;
  } > mrom AT> sram
	. = SIZEOF(.rodata) ;
	.data : {
		_data_start = .;
    *(.data)
		_data_end = .;
  } > sram AT> sram
	. = SIZEOF(.data) ;
  .bss : {
		_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
		_bss_end = .;
  } > sram AT> sram
	. = SIZEOF(.bss) ;
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
