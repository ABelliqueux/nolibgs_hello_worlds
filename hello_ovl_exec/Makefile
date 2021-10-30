.PHONY: all cleansub
all:
	$(MAKE) -C hello_poly clean all
	$(MAKE) -C hello_ovl_world clean all
	$(MAKE) -C hello_tile clean all
	mkpsxiso -y ./isoconfig.xml
cleansub:
	$(MAKE) -C hello_poly clean
	$(MAKE) -C hello_tile clean
	$(MAKE) -C hello_ovl_world clean
	$(MAKE) clean
	rm -f hello_ovl_exec.cue hello_ovl_exec.bin
	rm -f *.mcd *.frag *.lua *.vert
	rm -f */*.mcd */*.frag */*.lua */*.vert */*.elf
#~ 	rm -f */*.mcd */*.frag */*.lua */*.vert */*.elf  */*.map  */*.ps-exe */*.o */*.dep
	
TARGET = hello_ovl_exec

OVERLAYSCRIPT  ?= overlay.ld
OVERLAYSECTION ?= .ovly0 .ovly1 .ovly2

SRCS = hello_ovl_exec.c \
hello_ovl_world/hello_ovl_world.c \
hello_tile/hello_ovl_tile.c \
hello_poly/hello_ovl_poly.c \

include ../common.mk 
