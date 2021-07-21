.PHONY: all cleansub
all:
	mkpsxiso -y ./isoconfig.xml
cleansub:
	$(MAKE) clean
	rm -f hello_bs.cue hello_bs.bin
	
TARGET = hello_bs

SRCS = hello_bs.c \

include ../common.mk 
