## Hello world
TARGET = hello_world
TYPE = ps-exe

SRCS = hello_world.c \
../common/crt0/crt0.s \

## Hello tile
#~ TARGET = hello_tile
#~ TYPE = ps-exe

#~ SRCS = hello_tile.c \
#~ ../common/crt0/crt0.s \

#~ ## Hello pad
#~ TARGET = hello_pad
#~ TYPE = ps-exe

#~ SRCS = hello_pad.c \
#~ ../common/crt0/crt0.s \

## Hello pad 2
#~ TARGET = hello_2pads
#~ TYPE = ps-exe

#~ SRCS = hello_2pads.c \
#~ ../common/crt0/crt0.s \

## Hello poly
#~ TARGET = hello_poly
#~ TYPE = ps-exe

#~ SRCS = hello_poly.c \
#~ ../common/crt0/crt0.s \

#~ ## Hello textured
#~ TARGET = hello_poly_ft
#~ TYPE = ps-exe

#~ SRCS = hello_poly_ft.c \
#~ ../common/crt0/crt0.s \
#~ TIM/bousai.tim \

#~ ## Hello shaded
#~ TARGET = hello_poly_gt
#~ TYPE = ps-exe

#~ SRCS = hello_poly_gt.c \
#~ ../common/crt0/crt0.s \
#~ TIM/bousai.tim \

#~ ## Hello sprt
#~ TARGET = hello_sprt
#~ TYPE = ps-exe

#~ SRCS = hello_sprt.c \
#~ ../common/crt0/crt0.s \
#~ TIM/TIM16.tim \
#~ TIM/TIM8.tim \
#~ TIM/TIM4.tim \

#~ ## hello vag
#~ TARGET = hello_vag
#~ TYPE = ps-exe

#~ SRCS = hello_vag.c \
#~ ../common/crt0/crt0.s \
#~ VAG/hello_poly.vag

#~ ## hello multivag
#~ TARGET = hello_multivag
#~ TYPE = ps-exe

#~ SRCS = hello_multivag.c \
#~ ../common/crt0/crt0.s \
#~ VAG/hello.vag
#~ VAG/poly.vag

## Poly fun !
#~ TARGET = fun_with_poly
#~ TYPE = ps-exe

#~ SRCS = fun_with_poly.c \
#~ ../common/crt0/crt0.s \



CPPFLAGS += -I../psyq/include
LDFLAGS += -L../psyq/lib
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
LDFLAGS += -Wl,--end-group

include ../common.mk \
