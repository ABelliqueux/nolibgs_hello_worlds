.PHONY: all cleansub
all:
	mkpsxiso -y ./isoconfig.xml
cleansub:
	$(MAKE) clean
	rm -f hello_spu_readback.cue hello_spu_readback.bin

TARGET = hello_spu_readback

SRCS = hello_spu_readback.c \

include ../common.mk 
