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

# Transparency

In 16bpp mode, only 15b are used for colors (R 5, G 5, B 5). The 15th bit is defined as the STP or Semi-Transparency flag.  
A primitive transparency is set with `SetSemiTrans()`.  The only case where a primitive with unset (=0) STP is transparent is when all values are 0.  
i.e ; using STP 0, B 0, G 0, R 0 will result in a transparent pixel wether the primitive is set to semi-tranparent or not.  

Here are the transparency modes for various values on semi-transparent and opaque primitives :

 | STP, B, G, R | (0, 0, 0, 0) | (1, 0, 0, 0) | (0, n, n, n) | (1, n, n, n) |
 | :-: | :-: | :-: | :-: | :-: |
 | Non-transparent primitive | Transparent | Black | Non-transparent |  Non-transparent |  
 | Semi-transparent primitive | Transparent | Semi-transparent  | Non-transparent black |  Semi-transparent  |  

See [FileFormat47.pdf](http://psx.arthus.net/sdk/Psy-Q/DOCS/FileFormat47.pdf), p.56, p.192,   
[FileFormat47.pdf](http://psx.arthus.net/sdk/Psy-Q/DOCS/LibOver47.pdf), p.107

## img2tim semi-transparency options

`img2tim` has several options related to pixel transparency :  

 * -t            - Set semi-transparent bit (STP) on non fully black pixels. This will set the STP to 1 on pixels with RGB values different from B0,G0,R0.
 * -usealpha     - Use alpha channel (if available) as transparency mask. This will use the converted image's alpha channel (PNG, TGA, TIFF, GIF)
 * -alpt <value> - Threshold value when alpha channel is used as transparency mask (Default: 127). Transparency values above this wil be treated as opaque.
 * -tindex <col> - Specify color index to be treated as transparent (ignored on non palletized images). When using 4bpp/8bpp, specified color to be used as transparent.
 * -tcol <r g b> - Specify RGB color value to be treated as transparent. Same as above for 16bpp.

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

## Image > 4bpp, 8bpp

To convert your images to palettized 4bpp and 8bpp pngs, you can use [pngquant](https://pngquant.org/) :

4bpp (16 colors) image :

```bash
pngquant 16 input.png -o output.png --force 
```
8bpp (256 colors) image :

```bash
pngquant 256 input.png -o output.png --force 
```
 
Alternatively, you can use imagemagick :

4bpp (16 colors) image :

```bash
convert input.png -colors 16 output.png 
```
8bpp (256 colors) image :

```bash
convert input.png -colors 256 output.png
```

## PNG > Tim

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
