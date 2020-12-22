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

  * For 8bpp, < 256 colors 

  * For 4bpp, < 16 colors, and size must be a multiple of 4

You can use TIMTOOL.EXE from legacy PsyQ to check your TIM files, or use Lameguy64's [TIMedit](https://github.com/Lameguy64/TIMedit)


# Links 

  * [TIMexample on psxdev.net](http://www.psxdev.net/forum/viewtopic.php?f=64&t=313)
  * [Lameguy64's Github repo](https://github.com/Lameguy64)
