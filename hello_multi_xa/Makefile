.PHONY: all cleansub
all:
	mkpsxiso -y ./isoconfig.xml
cleansub:
	$(MAKE) clean
	rm -f hello_multi_xa.cue hello_multi_xa.bin
	
TARGET = hello_multi_xa

SRCS = hello_multi_xa.c \

include ../common.mk 
