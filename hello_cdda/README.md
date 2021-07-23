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

##  Creating the disk image

```bash
mkpsxiso -y isoconfig.xml
```

## Using ffmpeg to generate a CDDA compliant Wav file

Needed Specification : `RIFF (little-endian) data, WAVE audio, Microsoft PCM, 16 bit, stereo 44100 Hz`

### Conversion

```bash
ffmpeg -i input.mp3 -acodec pcm_s16le -ac 2 -ar 44100 output.wav
```

### Merging two mono audio channels into one stereo channel 

```bash
ffmpeg -i herb.wav.new -filter_complex "[0:a][0:a]amerge=inputs=2[a]" -map "[a]" herbi.wav
```

### Adding the audio track to the CD

Add a track section **after** your data track in `isoconfig.xml` :  

```xml
<track type="audio" source="file.wav"/>
```

## Music credits

Track 1 :
Beach Party by Kevin MacLeod
Link: https://incompetech.filmmusic.io/song/3429-beach-party
License: https://filmmusic.io/standard-license  

Track 2:
Funk Game Loop by Kevin MacLeod
Link: https://incompetech.filmmusic.io/song/3787-funk-game-loop
License: https://filmmusic.io/standard-license
