__heap_base = MAX(__ovly0_end, __ovly2_end);

SECTIONS {		
	OVERLAY __bss_end : NOCROSSREFS SUBALIGN(4)
	{
		.ovlyload
        {
            load_all_overlays_here = .;
        }
	.ovly0
		{			
			KEEP(hello_poly/*.o(.text))
			__ovly0_ctor = .;
			KEEP(hello_poly/*.o(.text.startup._GLOBAL__*))
			KEEP(hello_poly/*.o(.text.*))
			KEEP(hello_poly/*.o(.rodata*))
			KEEP(hello_poly/*.o(.sdata*))
			KEEP(hello_poly/*.o(.data*))
			KEEP(hello_poly/*.o(.sbss*))
			KEEP(hello_poly/*.o(.bss*))
			KEEP(hello_poly/*.o(.ctors))
			
			. = ALIGN(4);
			__ovly0_end = .;			
		}

        .ovly1
		{
			KEEP(hello_tile/*.o(.text))
			__ovly1_ctor = .;
			KEEP(hello_tile/*.o(.text.startup._GLOBAL__*))
			KEEP(hello_tile/*.o(.text.*))
			KEEP(hello_tile/*.o(.rodata*))
			KEEP(hello_tile/*.o(.sdata*))
			KEEP(hello_tile/*.o(.data*))
			KEEP(hello_tile/*.o(.sbss*))
			KEEP(hello_tile/*.o(.bss*))
			KEEP(hello_tile/*.o(.ctors))
			
			. = ALIGN(4);
			__ovly1_end = .;			
		}
	.ovly2
		{
			KEEP(hello_ovl_world/*.o(.text))
			__ovly2_ctor = .;
			KEEP(hello_ovl_world/*.o(.text.startup._GLOBAL__*))
			KEEP(hello_ovl_world/*.o(.text.*))
			KEEP(hello_ovl_world/*.o(.rodata*))
			KEEP(hello_ovl_world/*.o(.sdata*))
			KEEP(hello_ovl_world/*.o(.data*))
			KEEP(hello_ovl_world/*.o(.sbss*))
			KEEP(hello_ovl_world/*.o(.bss*))
			KEEP(hello_ovl_world/*.o(.ctors))
			
			. = ALIGN(4);
			__ovly2_end = .;			
		}
	}
}
