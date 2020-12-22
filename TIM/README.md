# TIM files

Standard bitmap images that can be transferred directly to the PSX VRAM.

Can be 4bit or 8bit with a CLUT, 16bit or 24 bit in direct color.

You can access the TIM mode through TIM_IMAGE->mode. 
Mode can be :

  * 0: 4bits   b0
  * 1: 8 bits  b1
  * 2: 16 bits b10
  * 3: 24bits  b11
  * 4: mixed   b100

See [FileFormat47.pdf](http://psx.arthus.net/sdk/Psy-Q/DOCS/FileFormat47.pdf), p.179

# Tools

You can use open source tools : Gimp, Aseprite

To convert your image files to TIM, use [IMG2TIM](https://github.com/Lameguy64/img2tim) :

## 4bpp and 8bpp specificities 

If you want to generate 4bpp and 8bpp TIMs, your original image must be in indexed mode with a palette.

  * For 8bpp, < 256 colors , and dimensions must be a multiple of 2

  * For 4bpp, < 16 colors, and dimensions must be a multiple of 4
  
See [FileFormat47.pdf](http://psx.arthus.net/sdk/Psy-Q/DOCS/FileFormat47.pdf), p.182

You can use TIMTOOL.EXE from legacy PsyQ to check your TIM files, or use Lameguy64's [TIMedit](https://github.com/Lameguy64/TIMedit)


# Reproducing the TIM in this example

```bash
img2tim -bpp 4 -org 512 0 -plt 0 481 -usealpha -o TIM4.tim TIM4.png 
img2tim -bpp 8 -org 512 256 -plt 0 480 -usealpha -o TIM8.tim TIM8.png 
img2tim -bpp 16 -org 768 0 -usealpha -o TIM16.tim TIM16.png 
```
## Content of Makefile :

```mk
SRCS = hello_sprt.c \
../common/crt0/crt0.s \
TIM/TIM16.tim \
TIM/TIM8.tim \
TIM/TIM4.tim \
```

# Links 

  * [TIMexample on psxdev.net](http://www.psxdev.net/forum/viewtopic.php?f=64&t=313)
  * [Lameguy64's Github repo](https://github.com/Lameguy64)
