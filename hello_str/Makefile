.PHONY: all cleansub
all:
	mkpsxiso -y ./isoconfig.xml
cleansub:
	$(MAKE) clean
	rm -f hello_str.cue hello_str.bin
	
TARGET = hello_str

SRCS = hello_str.c \

include ../common.mk 
