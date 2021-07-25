![Hello_stp](https://wiki.arthus.net/assets/hello-stp.png)

# STP : Semi-Transparency usage

This example shows the various way of converting an image with transparency to a TIM and use it in code.  
It also shows the effect of activating Semi-Transparency on a primitive textured with those images.  

Use the `SELECT` button to switch primitive semi-transparency on and off.  

It also features a few C struct to facilitate access to the TIM file / pixel data.

You can use Lameguy64's [img2tim](https://github.com/Lameguy64/img2tim) tool to convert most of image formats to the psx [TIM format.](https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/main/TIM).    

## Semi-transparency rates

You can find another example with the various transparency rates demoed here : https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/main/hello_cubetex_stp  

## Important 

**By default, the PSX will consider black pixels (0,0,0,0) as transparent**.  
In order to display those black pixels as black, you have to set the STP on black (1,0,0,0).   
Black pixels and non-black pixels with the STP bit will display as semi-transparent when using `SetSemiTrans()`.  

## STP on black 
 
Use this to display black pixels as black, not transparent.
The **inverted** alpha mask of the TIM  corresponds to the position of black (0,0,0) pixels in the image.

```bash
img2tim -b -org 640 0 -o stpOnBlack.tim av.png
```

## STP on non-black 

Black pixels will be considered as transparent, and non-black pixels will receive semi-transparency with `SetSemiTrans()`.  

The alpha mask of the TIM  corresponds to the position of non-black (n,n,n) pixels in the image.
Additionally, a setting allows you to define the RGB value to be considered transparent ; `-tcol` . This does not set any STP flag. 

```bash
img2tim -t -org 320 0 -o stpOnNonBlack.tim av.png
```

## Use alpha channel

The alpha mask of the TIM  corresponds to the existing alpha channel of the image (PNG, GIF, TGA, TIFF).
Additionally, a setting allows you to define the threshold for the alpha value to be considered transparent ; `-alpt` . This does not set any STP flag.

```bash
img2tim -usealpha -org 640 256 -o stpOnNonBlack.tim av.png
```

## Use color index

When using 8/4bpp palettized images, you can specify the index number of the color to be considered transparent. This does not set any STP flag.

You can set the STP bit by CLUT color with PsyQ's `TIMTOOL.EXE`. This allows you do do cool stuff like oly having specific colors being rendered as semi-transparent by `SetSemiTrans()`.

```bash
img2tim -b -bpp 8 -tindex 0 -org 640 256 -plt 0 481 -o stpOnColIndex.tim av8.png
```

## Black transparency work-around

Using a pseudo-black color with one of the channels value to 10, i.e : `255,255,10` can be done so you dont have to set the STP bit on full black.  
This allows you to keep the pseudo-black opaque when using `SetSemiTrans()`.
