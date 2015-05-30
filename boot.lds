# /*
# * Platform  for: s3c2440
# * author  by jiangdou
# * you  have anything,plese to QQ:344283973
# * time  at: 2009-0801
# *
# */






SECTIONS {
    . = 0x33f80000;
	
	. = ALIGN(4);
	
    .text : 
	{ 
		start.o	(.text)
		
		init.o (.text)
	*(.text)
	}
    
    . = ALIGN(4);
    .rodata : {*(.rodata*)} 
    
    . = ALIGN(4);
    .data : { *(.data) }
    
    . = ALIGN(4);
    __bss_start = .;
    .bss : { *(.bss)  *(COMMON) }
    __bss_end = .;
}

