.PHONY: all cleansub
all:
	mkpsxiso -y ./isoconfig.xml
cleansub:
	$(MAKE) clean
	rm -f hello_xa.cue hello_xa.bin
	
TARGET = hello_xa

SRCS = hello_xa.c \

include ../common.mk 
