# VAG files

> VAG is the PlayStation single waveform data format for ADPCM-encoded data of sampled sounds, such as
piano sounds, explosions, and music. The typical extension in DOS is “.VAG”.

See [FileFormat47.pdf](http://psx.arthus.net/sdk/Psy-Q/DOCS/FileFormat47.pdf), p.209


## Audio to VAG conversion

We have to convert the audio file to RAW data first :

```bash
ffmpeg -i input.mp3 -f s16le -ac 1 -ar 44100 tmp.dat
```

then use [`wav2vag`](https://github.com/ColdSauce/psxsdk/blob/master/tools/wav2vag.c) to convert the data, making sure the `-freq=` parameter matches the `-ar` value used above  :

```bash
wav2vag tmp.dat output.vag -sraw16 -freq=44100
```

## VAGedit

You can find a graphical editor in the [PsyQ sdk](http://psx.arthus.net/sdk/Psy-Q/PSYQ_SDK.zip) named `VAGEDIT.exe`.

## VAG & SPU Docs

See 
  * libformat47.pdf p.209
  * libover47.pdf,  p.271
  * libref47.pdf,   p.980

  * [http://psx.arthus.net/code/VAG/](http://psx.arthus.net/code/VAG)

## Ressources 

 * [wav2vag utility](https://github.com/ColdSauce/psxsdk/blob/master/tools/wav2vag.c)  
