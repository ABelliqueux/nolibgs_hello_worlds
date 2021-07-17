.PHONY: all cleansub
all:
	mkpsxiso -y ./isoconfig.xml
cleansub:
	$(MAKE) clean
	rm -f hello_cdda.cue hello_cdda.bin

TARGET = hello_cdda

SRCS = hello_cdda.c \

include ../common.mk 
