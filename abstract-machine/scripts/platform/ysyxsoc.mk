AM_SRCS := riscv/ysyxSoC/start.S \
           riscv/ysyxSoC/trm.c \
           #riscv/ysyxSoC/ioe.c \
           riscv/ysyxSoC/timer.c \
           riscv/ysyxSoC/input.c \
           riscv/ysyxSoC/cte.c \
           riscv/ysyxSoC/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections
LDFLAGS   += -T $(AM_HOME)/scripts/soc_linker.ld \
						 --defsym=_pmem_start=0x20000000 --defsym=_entry_offset=0x0 
						#--defsym=_stack_pointer=0x0f001fff  \
						 --defsym=_heap_end=0x0eff9fff  
						 #--defsym=_heap_start=0x0f000000  \
						# --defsym=_stack_pointer=0x0fff1000 

LDFLAGS   += --gc-sections -e _start

CFLAGS += -DMAINARGS=\"$(mainargs)\"
CFLAGS += -I/$(AM_HOME)/am/src/riscv/ysyxSoC/include
.PHONY: $(AM_HOME)/am/src/riscv/ysyxSoC/trm.c


NPCFLAGS += -b
NPCFLAGS += -l $(NPC_HOME)/build/ysyxsoc-log.txt
NPCFLAGS += -f $(IMAGE).elf
NPCFLAGS += -d $(NEMU_HOME)/build/riscv32-nemu-interpreter-so

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin
	#@$(OBJCOPY) --dump-section .text=$(IMAGE)_text.bin $(IMAGE).elf 

run: image
	@$(MAKE) -s -C $(NPC_HOME) sim ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin

gdb: image
	@$(MAKE) -s -C $(NPC_HOME) gdb  IMG=$(IMAGE).bin
print: image
	@$(MAKE) -s -C $(NPC_HOME) print ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin
	
