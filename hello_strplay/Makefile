.PHONY: all cleansub
all:
	mkpsxiso -y ./isoconfig.xml
cleansub:
	$(MAKE) clean
	rm -f hello_strplay.cue hello_strplay.bin
	
TARGET = hello_strplay

SRCS = hello_strplay.c \

include ../common.mk 
