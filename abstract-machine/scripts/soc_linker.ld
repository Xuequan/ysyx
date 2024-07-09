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
		. = ALIGN(0x4);
		_text_start = .;
    *(entry)
    *(.text*)
		. = ALIGN(0x4);
		_text_end = .;
  } > mrom :text

	.rodata : {
		. = ALIGN(0x4);
		_rodata_start = . ;
    *(.rodata*)
		. = ALIGN(0x4);
		_rodata_end   = . ;
  } > mrom

	.data ORIGIN (sram) : 
		/*AT (ADDR (.text) + SIZEOF (.rodata) + SIZEOF (.text) ) */
	{
		. = ALIGN(0x4);
		_data_start = .;
    *(.data*)
		. = ALIGN(0x4);
		_data_end = .;
  } > sram AT> mrom :data

  .bss : 
	{
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  } > sram

	/*
	.heap ORIGIN (sram) + SIZEOF (.data) (NOLOAD) :
	{
		. = ALIGN(4);
		_heap_start = .;
		. = . + LENGTH (sram) + SIZEOF (.data) - 0x8000 ;
		. = ALIGN(0x4);
		_heap_end = .;
	} > sram	

	.stack ORIGIN (sram) + LENGTH (sram) - 0x8000 (NOLOAD) :
	{
		. = ALIGN(0x4);
		_stack_end = .;
		. = . + 0x8000; 
		// KEEP(*(.stack_buffer)) 
		_stack_pointer = .;
		. = ALIGN(0x4);
	} > sram
	*/
}

_data_size = SIZEOF(.data);
_data_Load_addr = LOADADDR(.data);

_bss_start = ADDR(.bss);
_bss_end = _bss_start + SIZEOF(.bss);

_heap_start = ORIGIN (sram) + SIZEOF (.data) ;
_heap_end = ORIGIN (sram) + LENGTH (sram) - 0x8000 ;

_stack_pointer = ORIGIN (sram) + LENGTH (sram) ;
