# VAG files

> VAG is the PlayStation single waveform data format for ADPCM-encoded data of sampled sounds, such as
piano sounds, explosions, and music. The typical extension in DOS is “.VAG”.

See [FileFormat47.pdf](http://psx.arthus.net/sdk/Psy-Q/DOCS/FileFormat47.pdf), p.209


## WAV creation

Use ffmpeg to create a 16-bit ADPCM mono WAV file - change -ar to reduce filesize (and quality)

```bash
$ ffmpeg -i input.mp3 -acodec pcm_s16le -ac 1 -ar 44100 output.wav
```

You can use Audacity to edit sound.

## WAV to VAG convertion using WAV2VAG

Get here : [WAV2VAG](https://github.com/ColdSauce/psxsdk/blob/master/tools/wav2vag.c)

Change -freq according to the -ar setting above 

```bash
$ wav2vag input.wav output.vag -sraw16 -freq=44100 (-L) 
```

### Bug ? 

After conversion with WAV2VAG, the resulting VAG will sometimes have a pop at the very beginning and/or end of the file.

You can check (and delete) this with PsyQ's VAGEDIT.EXE.

You can also force the sampling frequency of an existing VAG file.

## VAG & SPU Docs

See 
  * libformat47.pdf p.209
  * libover47.pdf,  p.271
  * libref47.pdf,   p.980

  * [http://psx.arthus.net/code/VAG/](http://psx.arthus.net/code/VAG)
