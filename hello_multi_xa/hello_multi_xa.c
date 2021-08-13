// XA multi sample playback example
//
// Use an interleaved XA file to store multiple sound effects and use pre-calculated offset to play them back.  
// Use up, down, left, right, triangle, cross, circle, square to play various samples.  
// If this seems complicated, first study the simple XA playback example : https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/main/hello_xa  
//
// base on `psyq/addons/scee/CD/XAPLAYER`
// Refs : http://psx.arthus.net/code/XA/XATUT.pdf
//        http://psx.arthus.net/code/XA/xatut.zip
//        http://psx.arthus.net/code/XA/XA%20ADPCM%20documentation.txt
// Schnappy 2021
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
// CD library
#include <libcd.h>
// SPU library
#include <libspu.h>

#define VMODE 0                  // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320           // Screen width
#define SCREENYRES 240 + (VMODE << 4)          // Screen height : If VMODE is 0 = 240, if VMODE is 1 = 256 
#define CENTERX SCREENXRES/2     // Center of screen on x 
#define CENTERY SCREENYRES/2     // Center of screen on y
#define MARGINX 0                // margins for text display
#define MARGINY 32
#define FONTSIZE 8 * 16          // Text Field Height
DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
short db = 0;                    // index of which buffer is used, values 0, 1

// SPU attributes
SpuCommonAttr spuSettings;
#define CD_SECTOR_SIZE 2048
// XA
// Sector offset for XA data 4: simple speed, 8: double speed
#define XA_CHANNELS 8
#define XA_CDSPEED XA_CHANNELS >> 2
// Number of XA samples ( != # of XA files )
#define XA_TRACKS 8

typedef struct XAsound {
    u_int id;
    u_int size;
    // We can find size in sector : size / 2336, start value begins at 23, end value is at start + offset ( (size / 2336)-1 * #channels )
    // subsequent start value have an 8 bytes offset (n-1.end + 8)
    u_char file, channel;
    u_int start, end;
    int cursor;
} XAsound;

typedef struct XAbank {
    u_int index;
    int offset;
    XAsound samples[];
} XAbank;

XAbank soundBank = {
        8,
        0,
        {
            // channel 5
            // id   size   file  channel start end cursor
            {   0,  18688,   0,     5,     0,   56,  -1 }, 
            {   1,  44384,   0,     5 ,   144,  288, -1 }, 
            // channel 6                 
            {   2,  32704,   0,     6 ,   0,   104, -1  }, 
            {   3,  56064,   0,     6 ,   196, 380, -1  }, 
            {   4,  53728,   0,     6 ,   468, 644, -1  }, 
            // channel 7                               
            {   5,  84096,   0,     7 ,   0,   260, -1  }, 
            {   6,  16352,   0,     7 ,   368, 440, -1  }, 
            // channel 8                               
            {  7,  114464,   0,     8 ,   0,   384, -1  }
        }
};
// XA file to load
static char * loadXA = "\\INTER8.XA;1";
// File informations : pos, size, name
CdlFILE XAPos = {0};
// CD filter
CdlFILTER filter;
// File position in m/s/f
CdlLOC  loc;

void init(void)
{
    ResetGraph(0);                 // Initialize drawing engine with a complete reset (0)
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);     // Set display area for both &disp[0] and &disp[1]
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // &disp[0] is on top  of &disp[1]
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // Set draw for both &draw[0] and &draw[1]
    SetDefDrawEnv(&draw[1], 0, 0         , SCREENXRES, SCREENYRES);     // &draw[0] is below &draw[1]
    // Set video mode
    if (VMODE){ SetVideoMode(MODE_PAL);}
    SetDispMask(1);                 // Display on screen    
    setRGB0(&draw[0], 50, 50, 50); // set color for first draw area
    setRGB0(&draw[1], 50, 50, 50); // set color for second draw area
    draw[0].isbg = 1;               // set mask for draw areas. 1 means repainting the area with the RGB color each frame 
    draw[1].isbg = 1;
    PutDispEnv(&disp[db]);          // set the disp and draw environnments
    PutDrawEnv(&draw[db]);
    FntLoad(960, 0);                // Load font to vram at 960,0(+128)
    FntOpen(MARGINX, MARGINY, SCREENXRES - MARGINX * 2, FONTSIZE, 0, 512 ); // FntOpen(x, y, width, height,  black_bg, max. nbr. chars
}
void display(void)
{
    DrawSync(0);                    // Wait for all drawing to terminate
    VSync(0);                       // Wait for the next vertical blank
    PutDispEnv(&disp[db]);          // set alternate disp and draw environnments
    PutDrawEnv(&draw[db]);  
    db = !db;                       // flip db value (0 or 1)
}

void spuSetup(SpuCommonAttr * spuSettings)
{
    // Init Spu
    SpuInit();
    // Set master & CD volume to max
    spuSettings->mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX);
    spuSettings->mvol.left  = 0x6000;
    spuSettings->mvol.right = 0x6000;
    spuSettings->cd.volume.left = 0x6000;
    spuSettings->cd.volume.right = 0x6000;
    // Enable CD input ON
    spuSettings->cd.mix = SPU_ON;
    // Apply settings
    SpuSetCommonAttr(spuSettings);
    // Set transfer mode 
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
}

void XAsetup()
{   
    u_char param[4];
    // ORing the parameters we need to set ; drive speed,  ADPCM play, Subheader filter, sector size
    // If using CdlModeSpeed(Double speed), you need to load an XA file that has 8 channels.
    // In single speed, a 4 channels XA is to be used.
    param[0] = CdlModeSpeed|CdlModeRT|CdlModeSF|CdlModeSize1;
    // Issue primitive command to CD-ROM system (Blocking-type)
    // Set the parameters above
    CdControlB(CdlSetmode, param, 0);
    // Pause at current pos
    CdControlF(CdlPause,0);
}
int main(void)
{
    // Init display
    init();
    // Set SPU
    spuSetup(&spuSettings);
    // Init CD system
    CdInit();
    // Pad
    PadInit(0);                   
    // Load XA file from cd
    // Find XA file pos
    CdSearchFile( &XAPos, loadXA);
    // Set cd head to start of file
    soundBank.offset = CdPosToInt(&XAPos.pos);
    // Set cd XA playback parameters and pause cd
    XAsetup();
    // Pad variables to avoid multifire
    int pad, oldpad;
    // Keep track of XA Sample currently playing
    int sample = -1;
    while (1)  // infinite loop
    {   
        // if sample is set
        if (sample != -1 ){
            // Begin XA file playback...
            // if sample's cursor is 0
            if (soundBank.samples[sample].cursor == 0){
                // Convert sector number to CD position in min/second/frame and set CdlLOC accordingly.
                CdIntToPos( soundBank.samples[sample].start + soundBank.offset , &loc);
                // Send CDROM read command
                CdControlF(CdlReadS, (u_char *)&loc);
                // Set playing flag
            }
            // if sample's cursor is close to sample's end position, stop playback
            if ((soundBank.samples[sample].cursor += XA_CDSPEED) >= soundBank.samples[sample].end - soundBank.samples[sample].start  ){
                CdControlF(CdlStop,0);
                soundBank.samples[sample].cursor = -1;
                sample = -1;
            }
        }
        
        // Pad 
        // Use up, down, left, right, triangle, cross, circle, square to play various samples
        // Read pad values
        pad = PadRead(0);
        
        // Left
        if (pad & PADLleft && !(oldpad & PADLleft)){
            // Set sample ID for playback
            sample = 7;
            // Change file/channel in the filter struct
            filter.chan = soundBank.samples[sample].channel;
            filter.file = soundBank.samples[sample].file;
            // Set filter
            CdControlF(CdlSetfilter, (u_char *)&filter);
            // Reset sample's cursor
            soundBank.samples[sample].cursor = 0;
            // Store pad value for release
            oldpad = pad;
            }
            
        if (pad & PADLright && !(oldpad & PADLright)){
            sample = 6;
            filter.chan = soundBank.samples[sample].channel;
            filter.file = soundBank.samples[sample].file;
            // Set filter
            CdControlF(CdlSetfilter, (u_char *)&filter);
            soundBank.samples[sample].cursor = 0;
            oldpad = pad;
            }
        if (pad & PADLup && !(oldpad & PADLup)){
            sample = 5;
            filter.chan = soundBank.samples[sample].channel;
            filter.file = soundBank.samples[sample].file;
            // Set filter
            CdControlF(CdlSetfilter, (u_char *)&filter);
            soundBank.samples[sample].cursor = 0;
            oldpad = pad;
            }
            
        if (pad & PADLdown && !(oldpad & PADLdown)){
            sample = 4;
            filter.chan = soundBank.samples[sample].channel;
            filter.file = soundBank.samples[sample].file;
            // Set filter
            CdControlF(CdlSetfilter, (u_char *)&filter);
            soundBank.samples[sample].cursor = 0;
            oldpad = pad;
            }
            
        if (pad & PADRleft && !(oldpad & PADRleft)){
            sample = 3;
            filter.chan = soundBank.samples[sample].channel;
            filter.file = soundBank.samples[sample].file;
            // Set filter
            CdControlF(CdlSetfilter, (u_char *)&filter);
            soundBank.samples[sample].cursor = 0;
            oldpad = pad;
            }
            
        if (pad & PADRright && !(oldpad & PADRright)){
            sample = 2;
            filter.chan = soundBank.samples[sample].channel;
            filter.file = soundBank.samples[sample].file;
            // Set filter
            CdControlF(CdlSetfilter, (u_char *)&filter);
            soundBank.samples[sample].cursor = 0;
            oldpad = pad;
            }
        if (pad & PADRup && !(oldpad & PADRup)){
            sample = 1;
            filter.chan = soundBank.samples[sample].channel;
            filter.file = soundBank.samples[sample].file;
            // Set filter
            CdControlF(CdlSetfilter, (u_char *)&filter);
            soundBank.samples[sample].cursor = 0;
            oldpad = pad;
            }
            
        if (pad & PADRdown && !(oldpad & PADRdown)){
            sample = 0;
            filter.chan = soundBank.samples[sample].channel;
            filter.file = soundBank.samples[sample].file;
            // Set filter
            CdControlF(CdlSetfilter, (u_char *)&filter);
            soundBank.samples[sample].cursor = 0;
            oldpad = pad;
            }
        // Reset oldpad    
        if (!(pad & PADRdown) ||
            !(pad & PADRup)   ||
            !(pad & PADRleft) ||
            !(pad & PADRright) ||
            !(pad & PADLdown) ||
            !(pad & PADLup)   ||
            !(pad & PADLleft) ||
            !(pad & PADLright)
            ){oldpad = pad;}
        
        FntPrint("Hello multi XA ! %d\n", VSync(-1));
        FntPrint("Use the pad to play various samples.\n");
        for (int i=0;i<soundBank.index;i++){
            if (i == sample){
                FntPrint(">");
            }
            FntPrint("%d: %d %d\n", i, soundBank.samples[i].start, soundBank.samples[i].end);  
        }
        FntPrint("Cursor: %d\n", soundBank.samples[sample].cursor );
        FntPrint("File offset: %d\n", soundBank.offset );
        
        FntFlush(-1);               // Draw print stream
        display();                  // Execute display()
    }
    return 0;
}
