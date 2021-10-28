# If you change this to exe, you'll have to rename the file ./thirdparty/nugget/ps-exe.ld too.
TYPE = ps-exe

THISDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

SRCS += $(THISDIR)thirdparty/nugget/common/crt0/crt0.s
SRCS += $(THISDIR)thirdparty/nugget/common/syscalls/printf.s 

CPPFLAGS += -I$(THISDIR)thirdparty/nugget/psyq/include -I$(THISDIR)psyq-4_7-converted/include -I$(THISDIR)psyq-4.7-converted-full/include -I$(THISDIR)psyq/include 
LDFLAGS += -L$(THISDIR)thirdparty/nugget/psyq/lib -L$(THISDIR)psyq-4_7-converted/lib -L$(THISDIR)psyq-4.7-converted-full/lib -L$(THISDIR)psyq/lib
# add support for NDR008's VScode setup
CPPFLAGS += -I$(THISDIR)../third_party/psyq/include
LDFLAGS += -L$(THISDIR)../third_party/psyq/lib
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


include $(THISDIR)thirdparty/nugget/common.mk

define OBJCOPYME
$(PREFIX)-objcopy -I binary --set-section-alignment .data=4 --rename-section .data=.rodata,alloc,load,readonly,data,contents -O $(FORMAT) -B mips $< $@
endef

# convert TIM file to bin
%.o: %.tim
	$(call OBJCOPYME)

# convert VAG files to bin
%.o: %.vag
	$(call OBJCOPYME)
	
# convert HIT to bin
%.o: %.HIT
	$(call OBJCOPYME)