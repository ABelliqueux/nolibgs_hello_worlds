# Loading a BS still image

You need [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) in your $PATH to generate a PSX disk image.

## Compile

```bash
make all
```

## Clean directory

```bash
make cleansub
```

## Converting a still image to BS

`MC32` can convert these formats to BS : TIM, RGB, YUV.

### Image > TIM with img2tim

Convert your image to a 24bpp TIM with [`img2tim`](https://github.com/Lameguy64/img2tim):

```bash
img2tim -bpp 24 -o output.tim input.png
```

Then use `MC32` as instructed below.

Result :  

```bash
identify bace.tim 
bace.tim TIM 320x240 320x240+0+0 8-bit sRGB 230420B 0.000u 0:00.000
```

### Image > RGB with imagemagick

You can convert your image to RGB with:

```bash
convert input.png RGB:output.rgb
```
Result :  
```bash
identify -size 320x240 -depth 8 RGB:bace.rgb
RGB:bace.rgb=>bace.rgb RGB 320x240 320x240+0+0 8-bit sRGB 230400B 0.000u 0:00.003
```


### Image > YUV422 UYVY with imagemagick

You can convert your image to YUV with:

```bash
convert input.png UYVY:output.yuv
```
Result :  
```bash
dentify -size 320x240 UYVY:bace.yuv 
UYVY:bace.yuv=>bace.yuv UYVY 320x240 320x240+0+0 8-bit YCbCr 153600B 0.000u 0:00.005
```


### TIM/RGB/UYVY > BS conversion

Use the [`MC32` tool](http://psx.arthus.net/tools/pimp-psx.zip) conversion tool to import the image, specifying the right dimensions, and convert to `bs` with those settings :

**Note that a BS image must have a width and height that is a multiple of 16**

```
Input: RGB, Output: bs
MDEC version : 2
Custom: Size in sectors or (2048 * sector number) bytes, Variable frame size
```

![MC32 bs conversion](https://wiki.arthus.net/assets/mc32-bs-conv.png)

## Sources & Refs

img2tim : https://github.com/Lameguy64/img2tim  
MC32 : http://psx.arthus.net/tools/pimp-psx.zip  

mdecnote : http://psx.arthus.net/sdk/Psy-Q/DOCS/TECHNOTE/mdecnote.pdf  
PSX RGB and YUV format : http://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Dataconv.pdf , p.68
