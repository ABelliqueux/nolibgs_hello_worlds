This example show how to use Lameguy64's  [STR playback library](https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/main/hello_str#str-playback-library).

For a barebone example, see the [hello_str](https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/main/hello_str) example.

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
will clean the current directory.

## STR playback library

@Lameguy64 has spent some time making a STR playback library that's easily included in a project :

> One thing that I find somewhat missing here is a decent piece of code for playing STR video files easily. So, what I did was take the old and messy PsyQ STR player example, clean it up entirely, and finally make it into a cute little c library for easy implementation.

Original post : http://www.psxdev.net/forum/viewtopic.php?t=507  
Original download link : https://www.mediafire.com/download/s61u86sxd1djncy/strplay.7z  
Mirror : http://psx.arthus.net/code/strplay.7z  

## Video encoding and more informations

See the [wiki](https://github.com/ABelliqueux/nolibgs_hello_worlds/wiki/STR).

## Video credits 

The video and song used in this example are by Nina Paley : https://archive.org/details/CopyingIsNotTheft-ScratchTrack1280X720Hdv  
