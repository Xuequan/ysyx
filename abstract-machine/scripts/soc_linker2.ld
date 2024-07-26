/* 这个对应"完整测试 PSRAM的访问" 
 * 将 mem-test 完全加载到 SRAM
 * 将 只读数据段一起加载到 SRAM
 */
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
  . = _pmem_start + _entry_offset; 
  .text :
	{
		. = ALIGN(0x8);
		_text_start = .;
    *(entry)
    *(.text*)
		*(.copy_to_sram)
		. = ALIGN(0x8);
		_text_end = .;
  } > sram AT> flash :text

	.rodata : 
	{
		. = ALIGN(0x8);
		_rodata_start = . ;
    *(.rodata*)
		*(.srodata.mainargs)
		. = ALIGN(0x8);
		_rodata_end   = . ;
  } > sram AT> flash

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
  } > sram AT> flash : data

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
_heap_start = ORIGIN (sram) + SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data);
_heap_end = ORIGIN (sram) + LENGTH(sram) - 0x800;

_stack_point = ALIGN(0x8) ;
_stack_pointer = ORIGIN (sram) + LENGTH (sram) ;
