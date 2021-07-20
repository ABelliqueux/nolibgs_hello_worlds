This is a STR playback example by @Lameguy64 : http://www.psxdev.net/forum/viewtopic.php?t=507

> One thing that I find somewhat missing here is a decent piece of code for playing STR video files easily. So, what I did was take the old and messy PsyQ STR player example, clean it up entirely, and finally make it into a cute little c library for easy implementation.

Original download link : https://www.mediafire.com/download/s61u86sxd1djncy/strplay.7z  
Mirror : http://psx.arthus.net/code/strplay.7z  

## Converting to AVI

You need `AVI file 320x240, 15 fps, 24-bit color, Stereo 16-bit sound @ 44100 Hz`.  

```
Stream #0:0: Video: rawvideo, bgr24, 320x240, 27763 kb/s, 15 fps, 15 tbr, 15 tbn, 15 tbc
Stream #0:1: Audio: pcm_u8 ([1][0][0][0] / 0x0001), 44100 Hz, 2 channels, u8, 705 kb/s
```

### Video to AVI

Use Virtualdub or ffmpeg :  

```bash
ffmpeg -i $INPUT.MKV -vcodec rawvideo -pix_fmt bgr24 -vf scale=320:240,setsar=1:1 -acodec pcm_u8 -ar 44100 -r 15 $OUTPUT.avi
```

### AVI to STR

Use [`MC32.EXE`](http://psx.arthus.net/tools/pimp-psx.zip) to convert the AVI file to STR using these settings :  

```
Format : Input : Avi (Uncompressed), Output : str (MDEC)
Sound: 37.8 KHz, Stereo;
Easy: Double Speed, 15 fps, 1ch, Leap Sector;
MDEC: version 2
```

![MC32-avi-str](https://wiki.arthus.net/assets/MC32-avi-str.png)

**If `MC32.exe` crashes when hitting the 'Go' button, you have to open the ffmpeg AVI file in virtualdub, then save it again ; `File > Save as AVI...` or `F7` key, then retry.**  

You should now have a STR file and a XA file that you have to interleave in `MC32`:

```
Format : Input : str (MDEC), Output: str (MDEC)
Sound: Input: XA , 37.8 KHz, Stereo;
Frame rate: 15 fps, # Channels : 1(150sectors/s), Leap Sector;
CD-ROM speed : Double Speed;
```

![MC32-avi-str-interleave](https://wiki.arthus.net/assets/MC32-avi-str-interleaved.png)

### Finding a video's frame count

With `ffmpeg` :

```bash
ffprobe -v error -select_streams v:0 -count_packets -show_entries stream=nb_read_packets -of csv=p=0 VIDEOFILE.AVI
```

Alternatively, open the STR file in `MC32.exe` and look at the bottom left of the window.

### Tools & Refs

MC32 : http://psx.arthus.net/tools/pimp-psx.zip  
STR converter : http://psx.arthus.net/tools/str_converter.rar  

Original PsyQ sample code : `/psyq/addons/cd/MOVIE`  
Original post : http://www.psxdev.net/forum/viewtopic.php?t=507  
Video to STR conversion tutorial : http://www.psxdev.net/forum/viewtopic.php?f=51&t=277  
MDEC notes : http://psx.arthus.net/sdk/Psy-Q/DOCS/TECHNOTE/mdecnote.pdf  
