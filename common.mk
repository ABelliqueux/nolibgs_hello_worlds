# If you change this to exe, you'll have to rename the file ./thirdparty/nugget/ps-exe.ld too.
TYPE = ps-exe
SRCS += ../thirdparty/nugget/common/crt0/crt0.s

CPPFLAGS += -I../thirdparty/nugget/psyq/include -I../psyq-4_7-converted/include -I../psyq-4.7-converted-full/include -I../psyq/include 
LDFLAGS += -L../thirdparty/nugget/psyq/lib -L../psyq-4_7-converted/lib -L../psyq-4.7-converted-full/lib -L../psyq/lib
LDFLAGS += -Wl,--start-group
LDFLAGS += -lapi
LDFLAGS += -lc
LDFLAGS += -lc2
LDFLAGS += -lcard
LDFLAGS += -lcomb
LDFLAGS += -lds
LDFLAGS += -letc
LDFLAGS += -lgpu
LDFLAGS += -lgs
LDFLAGS += -lgte
LDFLAGS += -lgun
LDFLAGS += -lhmd
LDFLAGS += -lmath
LDFLAGS += -lmcrd
LDFLAGS += -lmcx
LDFLAGS += -lpad
LDFLAGS += -lpress
LDFLAGS += -lsio
LDFLAGS += -lsnd
LDFLAGS += -lspu
LDFLAGS += -ltap
LDFLAGS += -lcd
LDFLAGS += -Wl,--end-group

THISDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include $(THISDIR)/thirdparty/nugget/common.mk

# convert TIM file to bin
%.o: %.tim
	$(PREFIX)-objcopy -I binary --set-section-alignment .data=4 --rename-section .data=.rodata,alloc,load,readonly,data,contents -O $(FORMAT) -B mips $< $@

# convert VAG files to bin
%.o: %.vag
	$(PREFIX)-objcopy -I binary --set-section-alignment .data=4 --rename-section .data=.rodata,alloc,load,readonly,data,contents -O $(FORMAT) -B mips $< $@
