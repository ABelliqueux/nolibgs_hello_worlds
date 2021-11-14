This example is adapted from PsyQ's sample : `psyq/psx/sample/sound/CDVOL, main.c,v 1.14 1997/05/02 13:05:21 by ayako`.
It was edited to fix typos, have a hopefully better code organization with hopefully more usefull variable names.

What it does is demonstrate how to transfer data from the PSX 's SPU to main memory in order to analyze / process the audio signal and dostuff accordingly.  
In this instance, it's used to determine the coordinates of a few primitives to display a [VU-meter](https://en.wikipedia.org/wiki/VU_meter).  

This technique is known to be used in certain games for lipsynching or audio visualization ( Crash team racing, Hercules, Vib Ribbon ...).

## pcsx-redux : no animation 

Pcsx-redux does not yet support these specific SPU buffers nor triggerring IRQ from them so as of 11-2021 this example doesn't work in this particular emulator.  
If looking for an alternative, check [duckstation](https://www.duckstation.org/) out.  

## PsyQ's SpuReadDecodedData() doc errata

The main function for transferring data from the SPU to the RAM is `SpuReadDecodedData()`, and is documented in **LibRef47.pdf, p1054**.  
The table on this page (Table 15-2) contains erroneous data and should read : 

![Spu addresses range](https://wiki.arthus.net/assets/spureaddecodeddata_errata.png)

The correct address ranges for the SPU buffer is :

| Map (bytes) | Data contents |
|-------------|---------------|
| 0x000 - 0x3ff | CD Left channel |
| 0x400 - 0x7ff | CD Right channel |
| 0x800 - 0xbff | Voice 1 |
| 0xC00 - 0xfff | Voice 3 |

## Compiling

You need [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) in your $PATH to generate a PSX disk image.
Typing 
```bash
make
```
in a terminal will compile and generate the bin/cue files.  

Typing
```bash
make cleansub
``` 
will clean the current directory

## More on CDDA 

See the [hello_cdda](https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/main/hello_cdda) example in this repo.

## Docs and links

Original psyq example : `psyq/psx/sample/sound/CDVOL, main.c,v 1.14 1997/05/02 13:05:21 by ayako` 

## Music credits

Track 1 :
Beach Party by Kevin MacLeod
Link: https://incompetech.filmmusic.io/song/3429-beach-party
License: https://filmmusic.io/standard-license  

Track 2:
Funk Game Loop by Kevin MacLeod
Link: https://incompetech.filmmusic.io/song/3787-funk-game-loop
License: https://filmmusic.io/standard-license
