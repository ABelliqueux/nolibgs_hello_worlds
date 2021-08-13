##  Multi XA samples playback

Use an interleaved XA file to store multiple sound effects and use pre-calculated offset to play them back.  
Use up, down, left, right, triangle, cross, circle, square to play various samples.  

If this seems complicated, first study the simple XA playback example : https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/main/hello_xa  

You need [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) in your $PATH to generate a PSX disk image.
You also need [ffmpeg](https://ffmpeg.org/), [`psxavenc` and `xainterleave`](https://github.com/ABelliqueux/candyk-psx/tree/master/toolsrc/).

### Compile

This will compile and build an iso image :

```bash
make
```

### Clean directory

```bash
make cleansub
```

## Producing an interleaved XA sound bank

### Sound to WAV conversion with ffmpeg

```bash
ffmpeg -i input.mp3 -acodec pcm_s16le -ac 1 -ar 44100 output.wav
``` 

### WAV to XA conversion 

See further down how you should adapt the `-F` and `-C` parameters.

```bash
psxavenc -f 37800 -t xa -b 4 -c 2 -F 1 -C 0 input.wav output.xa
```

### XA to multi XA track

You can use a concatenation tool to build your tracks like so :

```bash
cat track1.xa track2.xa track3.xa > channelX.xa
```

On windows, use :
```
copy track1.xa+track2.xa+track3.xa channelX.xa
```

### XA to interleaved XA

Use `xainterleave` as instructed here : https://github.com/ABelliqueux/nolibgs_hello_worlds/wiki/XA#interleaving-xa-files  

## Multi XA specifics

## Silence file

You should use a silent XA file to intersperse between your samples, so that you have a margin of error to avoid pops and noises.I.e:

```bash
cat track1.xa silence.xa track3.xa silence.xa track4.xa  > channelX.xa
```

### XA file size and vertical density

An interleaved XA (4 or 8 channels) file will have the size of its largest channel multiplied by the number of channels.  
Therefore, you should try to obtain a vertical density on all channels for your sample, i.e :

That :

```
channel 0   sample1.xa
channel 1   sample2.xa
channel 2   sample3.xa
channel 3   sample4.xa
```

instead of :

```
channel 0   sample1.xa sample2.xa sample2.xa sample4.xa
channel 1   
channel 2   
channel 3   
```

You should use your longest sample as a base duration, and create the other channels to obtain a similar duration, i.e :

```
channel 0   <------------- sample1.xa ---------------> <-- silence.xa --> <-sample2.xa ->
channel 1   <-- sample3.xa --> <-- silence.xa --> < -------- sample4.xa ----------->
```

### Calculating sample's start and end offsets

Using the size of your XA sample file, you can find it's size in sector like so :

`(size/2336)-1 * 8`

For example, file with size `84096B` is `(((84096/2336) == 36) - 1 == 35) * 8 == 280`.

First sample's start will then be `0`, and end `280`.
The following sample's start position will have an offset of `8`, hence `288`, etc.

#### Example 

These are the calculations for the files on 4 channels

```
// channel 1
// name       size  start end
5_come.xa    18688  0      56
5_sile_h.xa  23360  64     136     
5_erro.xa    44384  144    288
 
// channel 2
6_cuek.xa    32704  0      104
6_sile_h.xa  23360  112    184
6_hehe.xa    56064  196    380
6_sile_h.xa  23360  388    460
6_wron.xa    53728  468    644

// channel 3
7_m4a1.xa    84096  0      280
7_sile_h.xa  23360  288    360 
7_punch.xa   16352  368    416

// channel 4
8_yooo.xa    114464 0      384
```

and the corresponding structure in code :

```c
XAbank soundBank = {
        8, // index
        0, // file offset
        {
            // channel 5
            // id   size   file  channel start end cursor
            {   0,  18688,   0,     5,     0,   56,  -1 }, 
            // Ommit the silence.xa file
            {   1,  44384,   0,     5 ,   144,  288, -1 }, 
            // channel 6                 
            {   2,  32704,   0,     6 ,   0,   104, -1  }, 
            // Ommit the silence.xa file
            {   3,  56064,   0,     6 ,   196, 380, -1  }, 
            // Ommit the silence.xa file
            {   4,  53728,   0,     6 ,   468, 644, -1  }, 
            // channel 7                               
            {   5,  84096,   0,     7 ,   0,   260, -1  }, 
            // Ommit the silence.xa file
            {   6,  16352,   0,     7 ,   368, 440, -1  }, 
            // channel 8                               
            {  7,  114464,   0,     8 ,   0,   384, -1  }
        }
};
```

## Tools, sources and documentation

See the wiki for general informations, tools and sources about the XA format : https://github.com/ABelliqueux/nolibgs_hello_worlds/wiki/XA

## Sound credits

All the sound effects come from https://www.myinstants.com and are :  

```
comedy_pop_finger_in_mouth_001(1).mp3
cuek.swf.mp3
erro.mp3
m4a1_single-kibblesbob-8540445.mp3
punch.mp3
wrong-answer-sound-effect.mp3
yooooooooooooooooooooooooo_4.mp3
hehehehhehehehhehehheheehehe.mp3
```
