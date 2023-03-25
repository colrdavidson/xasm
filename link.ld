ENTRY(_start)

SECTIONS
{
	. = 0x100000 + SIZEOF_HEADERS;
	.text : {
		*(.text)
	}
	.rodata : {
		*(.rodata)
		*(.rodata.*)
	}
	.data : {
		 *(.data)
	}
	.bss : {
		*(.bss)
		*(COMMON)
	}

	/DISCARD/ : { *(.comment .note .eh_frame) }
}
