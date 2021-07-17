.PHONY: all cleansub
all:
	mkpsxiso -y ./isoconfig.xml
cleansub:
	$(MAKE) clean
	rm -f hello_cd.cue hello_cd.bin
	
TARGET = hello_cd

SRCS = hello_cd.c \

include ../common.mk 
