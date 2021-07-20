##  XA playback

You need [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) in your $PATH to generate a PSX disk image.
You also need [`psxavenc` and `xainterleave`](https://github.com/ABelliqueux/candyk-psx/tree/master/toolsrc/).

### Generate interleaved XA file

```bash
psxavenc -f 37800 -t xa -b 4 -c 2 -F 1 -C 0 "../hello_cdda/audio/beach.wav" "xa/beach.xa"
psxavenc -f 37800 -t xa -b 4 -c 2 -F 1 -C 0 "../hello_cdda/audiofunk.wav" "xa/funk.xa"
xainterleave 1 xa/interleave4.txt xa/inter4.xa
xainterleave 1 xa/interleave8.txt xa/inter8.xa
```

Alternatively, you can use the windows tool [`MC32.EXE`](https://psx.arthus.net/tools/pimp-psx.zip) to interleave several PSX media files.

### Compile

This will compile and build an iso image :

```bash
make
```

### Clean directory

```bash
make cleansub
```

## Encoding to XA 

You can use a modified version of [`psxavenc`](https://github.com/ABelliqueux/candyk-psx/tree/master/toolsrc/psxavenc) to convert your audio file to a 2336 bytes XA file :

```bash
./psxavenc -f 37800 -t xa -b 4 -c 2 -F 1 -C 1 "input.wav" "output.xa"
```

You can read it back with `XAPLAY.EXE`, that's in `psyq/bin/XAplay`.

### PSXavenc usage

```
./psxavenc 
Usage: psxavenc [-f freq] [-b bitdepth] [-c channels] [-F num] [-C num] [-t xa|xacd|spu|str2] <in> <out>

    -f freq          Use specified frequency
    -t format        Use specified output type:
                       xa     [A.] .xa 2336-byte sectors
                       xacd   [A.] .xa 2352-byte sectors
                       spu    [A.] raw SPU-ADPCM data
                       str2   [AV] v2 .str video 2352-byte sectors
    -b bitdepth      Use specified bit depth (only 4 bits supported)
    -c channels      Use specified channel count (1 or 2)
    -F num           [.xa] Set the file number to num (0-255)
    -C num           [.xa] Set the channel number to num (0-31)
```

## Interleaving XA files

You can use [`MC32.EXE`](https://psx.arthus.net/tools/pimp-psx.zip) or [`xainterleave`](https://github.com/ABelliqueux/candyk-psx/tree/master/toolsrc/xainterleave) to interleave several PSX media files.

## xainterleave usage

`xainterleave <mode> <in.txt> <out.raw>`

`mode` can be 0 for full raw sectors or 1 for just XA (divisible by 2336)

`in.txt` is a manifest txt file as seen [here](https://github.com/ChenThread/fromage/blob/master/res/music.txt)

Example for 1 music file, to be played at 1x CD speed (4 channels):

```
1 xa test.xa 1 0
1 null
1 null
1 null
```

Add 4 more 1 null lines for 2x (8 channels).

```
   1     xa  menu.xa        1                          0  
sectors type file    xa_file number (0-255) xa_channel number (0-31)
```

The format seems to correspond to the [entry_t struct](https://github.com/ABelliqueux/candyk-psx/blob/db71929903cc09398f5efc23973f9e136d123bbb/toolsrc/xainterleave/xainterleave.c#L35).

## mkpsxiso

You can use the following syntax to include your XA file in the CD image :

```xml
<file name="mymusic1.xa" type="xa" source="mymusic1.xa"/>
```

See here for more details : https://github.com/Lameguy64/mkpsxiso/blob/c44b78e37bbc115591717ac4dd534af6db499ea4/examples/example.xml#L85

## PsyQ XA Tools

[XAPLAY.EXE](https://docs.google.com/uc?export=download&confirm=G9cM&id=0B_GAaDjR83rLZGVaZ2pvV2tjSVE) : Single channel XA playback
[XATOOL.EXE](http://psx.arthus.net/code/XA/xatut.zip) : XA structure inspector
[MC32.EXE](https://psx.arthus.net/tools/pimp-psx.zip)   : Converts WAV > XA > Interleaved XA

## More

XA tutorial : http://psx.arthus.net/code/XA/XATUT.pdf  

Full XAtut archive : http://psx.arthus.net/code/XA/xatut.zip  

XA ADPCM documentation : http://psx.arthus.net/code/XA/XA%20ADPCM%20documentation.txt  

PsyQ XA player example : `psyq/addons/scee/CD/XAPLAYER`

XA SCEE Technical note - July 1998 : http://psx.arthus.net/sdk/Psy-Q/DOCS/CONF/SCEE/98July/xa_sound.pdf

PSX audio tools : https://forum.xentax.com/viewtopic.php?t=10136

PIMP tools : https://psx.arthus.net/tools/pimp-psx.zip

Source : https://discord.com/channels/642647820683444236/663664210525290507/843211084609617930
