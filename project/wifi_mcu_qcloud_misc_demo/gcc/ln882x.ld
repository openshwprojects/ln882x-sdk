OUTPUT_FORMAT("elf32-littlearm", "elf32-gigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(Reset_Handler)

/* Linker script to configure memory regions. */
MEMORY
{
  FLASH     (rx)  : ORIGIN = 0x1000D100, LENGTH = 1000K
  RAM0      (rwx) : ORIGIN = 0x1FFE0000, LENGTH = 128K
  RAM1      (rwx) : ORIGIN = 0x20000000, LENGTH = 160K
  RETENTION (rwx) : ORIGIN = 0x20028000, LENGTH = 8K
}


SECTIONS
{
	.flash_copysection :
	{
		__copysection_ram0_load = LOADADDR(.flash_copysection);
		__copysection_ram0_start = .;
		KEEP(*(.vectors)) /* startup code: ARM vectors */
		*ll_qspi.o*(.text*)
		*ll_cache.o*(.text*)
		*ll_dma.o*(.text*)
		*ll_syscon.o*(.text*)
		*ll_sleep.o*(.text*)
		*ll_rtc.o*(.text*)
		*cache.o*(.text*)
		*qspi.o*(.text*)
		*flash.o*(.text*)
		*hal_dma.o*(.text*)
		*hal_syscon.o*(.text*)
		*hal_sleep.o*(.text*)
		*hal_rtc.o*(.text*)
		*port.o*(.text*)

		/* Optional .text part*/
		/*
		*ll_uart.o(.text)
		*hal_uart.o(.text)
		*log.o(.text)
		*serial.o(.text)
		*serial_hw.o(.text)
		*fifobuf.o(.text)
		*/

		. = ALIGN(4);
		/*all RW data in ram0 wlan shared memory and lwip pubf pool are not in this section, they are in .bss section*/
		*(.data*)
		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);

		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		KEEP(*(.jcr*))
		. = ALIGN(4);
		/* All data end */
		__copysection_ram0_end = .;

	} >RAM0 AT>FLASH

	.flash_text :
	{
		PROVIDE(_stext = .);
		*(.text*)
		PROVIDE(_etext = .);

		KEEP(*(.init))
		KEEP(*(.fini))

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		/* .dtors */
 		*crtbegin.o(.dtors)
 		*crtbegin?.o(.dtors)
 		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
 		*(SORT(.dtors.*))
 		*(.dtors)

		*(.rodata*)

		KEEP(*(.eh_frame*))
	} >FLASH

	/* .stack_dummy section doesn't contain any symbols. It is only
	* used for linker to calculate size of stack sections, and assign
	* values to stack symbols later */
	.stack_dummy (COPY):
	{
		PROVIDE(__stack_start__ = .);
		KEEP(*(.stack*))
		PROVIDE(__c_stack_top__ = .);
		PROVIDE(__stack_end__ = .);
	} >RAM0

	/* Location counter can end up 2byte aligned with narrow Thumb code but
	   __etext is assumed by startup code to be the LMA of a section in RAM
	   which must be 4byte aligned */
	__etext = ALIGN (4);
	.bss_ram0 (NOLOAD):
	{
		. = ALIGN(4);
		__bss_ram0_start__ = .;
		*memp.o(.bss* COMMON)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_ram0_end__ = .;
	} >RAM0

	.bss_ram1 (NOLOAD):
	{
		. = ALIGN(4);
		__bss_ram1_start__ = .;
		*(wlan_mem_local)
		*(wlan_mem_pkt)
		*(wlan_mem_dscr)
		. = ALIGN(4);
		__bss_ram1_end__ = .;
	} >RAM1

	.no_init_data (NOLOAD):
	{
		. = ALIGN(4);
		*(.no_init_data)
		*(.noinit .noinit.*)
		. = ALIGN(4);
		__retention_start__ = .;
		*(retention_data)
		. = ALIGN(4);
		__retention_end__ = .;
	} >RETENTION

	/* Group unwind sections together: */
	.ARM.extab : { *(.ARM.extab*) }
	.ARM.exidx : { *(.ARM.exidx*) }

	/DISCARD/ : {
        *(.ARM.exidx.exit.text)
        *(.ARM.extab.exit.text)
        *(.ARM.exidx.devexit.text)
        *(.ARM.extab.devexit.text)
	}

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	PROVIDE(__StackTop = __c_stack_top__);
	PROVIDE(__StackLimit = __stack_start__);
	PROVIDE(__stack = __StackTop);

	PROVIDE(heap0_start = __bss_ram0_end__);
	PROVIDE(heap0_end = ORIGIN(RAM0) + LENGTH(RAM0)); /* RAM0 end */
	PROVIDE(heap0_len = heap0_end - heap0_start);
	PROVIDE(heap1_start = __bss_ram1_end__);
	PROVIDE(heap1_end = ORIGIN(RAM1) + LENGTH(RAM1)); /* RAM1 end */
	PROVIDE(heap1_len = heap1_end - heap1_start);
	PROVIDE(heap2_start = __retention_end__);
	PROVIDE(heap2_end = ORIGIN(RETENTION) + LENGTH(RETENTION)); /* RETENTION end */
	PROVIDE(heap2_len = heap2_end - heap2_start);

	/* Check if data + heap + stack exceeds RAM limit */
	/* ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack") */
	ASSERT(heap0_start < heap0_end, "region RAM0 overflowed with .bss")
	ASSERT(heap1_start < heap1_end, "region RAM1 overflowed with .bss")
	ASSERT(heap2_start < heap2_end, "region RETENTION overflowed with .bss")

}
