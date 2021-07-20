// XA track playback example
// base on `psyq/addons/scee/CD/XAPLAYER`
// Refs : http://psx.arthus.net/code/XA/XATUT.pdf
//        http://psx.arthus.net/code/XA/xatut.zip
//        http://psx.arthus.net/code/XA/XA%20ADPCM%20documentation.txt
// based on Lameguy64's tutorial : http://lameguy64.net/svn/pstutorials/chapter1/1-display.html
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
// CD library
#include <libcd.h>
// SPU library
#include <libspu.h>

#define VMODE 0                 // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320          // Screen width
#define SCREENYRES 240 + (VMODE << 4)          // Screen height : If VMODE is 0 = 240, if VMODE is 1 = 256 
#define CENTERX SCREENXRES/2    // Center of screen on x 
#define CENTERY SCREENYRES/2    // Center of screen on y
#define MARGINX 0                // margins for text display
#define MARGINY 32
#define FONTSIZE 8 * 7           // Text Field Height
DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
short db = 0;                      // index of which buffer is used, values 0, 1

// SPU attributes
SpuCommonAttr spuSettings;
#define CD_SECTOR_SIZE 2048
// XA
// Sector offset for XA data 4: simple speed, 8: double speed
#define XA_SECTOR_OFFSET 4
// Number of XA files
#define XA_TRACKS 1
// Number of populated XA streams/channels in each XA file
#define INDEXES_IN_XA   1
#define TOTAL_TRACKS    (XA_TRACKS*INDEXES_IN_XA)

typedef struct {
    int start;
    int end;
} XA_TRACK;
// Declare an array of XA_TRACK
XA_TRACK XATrack[XA_TRACKS];
// Name of file to load
static char * loadXA = "\\INTER4.XA;1";
CdlFILE XAPos = {0};
// Start and end position of XA data, in sectors
static int      StartPos, EndPos;
// Current pos in file
static int      CurPos = -1;
// Playback status : 0 not playing, 1 playing
static int gPlaying = 0;
// Current XA channel 
static char channel = 0;

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
    FntOpen(MARGINX, SCREENYRES - MARGINY - FONTSIZE, SCREENXRES - MARGINX * 2, FONTSIZE, 0, 280 ); // FntOpen(x, y, width, height,  black_bg, max. nbr. chars
}
void display(void)
{
    DrawSync(0);                    // Wait for all drawing to terminate
    VSync(0);                       // Wait for the next vertical blank
    PutDispEnv(&disp[db]);          // set alternate disp and draw environnments
    PutDrawEnv(&draw[db]);  
    db = !db;                       // flip db value (0 or 1)
}

int main(void)
{
    // Init display
    init();     
    // SPU setup
    // Init Spu
    SpuInit();
    // Set master & CD volume to max
    spuSettings.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX);
    spuSettings.mvol.left  = 0x6000;
    spuSettings.mvol.right = 0x6000;
    spuSettings.cd.volume.left = 0x6000;
    spuSettings.cd.volume.right = 0x6000;
    // Enable CD input ON
    spuSettings.cd.mix = SPU_ON;
    // Apply settings
    SpuSetCommonAttr(&spuSettings);
    // Set transfer mode 
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    // Init CD system
    CdInit();                   
    // Optional : Set CD Attenuation volume
    //
    // CdlATV cd_vol;
    // cd_vol.val0 = cd_vol.val1 = cd_vol.val2 = cd_vol.val3 = 0x40;
    // CdMix(&cd_vol);
    //
    // Load XA file from cd
    // Find XA file pos
    CdSearchFile( &XAPos, loadXA);
    XATrack[0].start = CdPosToInt(&XAPos.pos);
    XATrack[0].end = XATrack[0].start + (XAPos.size/CD_SECTOR_SIZE) - 1;    
    StartPos = XATrack[0].start;
    EndPos = XATrack[0].end;
    // XA setup
    u_char param[4];
    // ORing the parameters we need to set ; drive speed,  ADPCM play, Subheader filter, sector size
    param[0] = CdlModeSpeed|CdlModeRT|CdlModeSF|CdlModeSize1;
    // Issue primitive command to CD-ROM system (Blocking-type)
    // Set the parameters above
    CdControlB(CdlSetmode, param, 0);
    // Pause at current pos
    CdControlF(CdlPause,0);
    // Set filter 
    // This specifies the file and channel number to actually read data from.
    CdlFILTER filter;
    // Use file 1, channel 0
    filter.file = 1;
    filter.chan = channel;
    // Set filter
    CdControlF(CdlSetfilter, (u_char *)&filter);
    // Position of file on CD
    CdlLOC  loc;
    // Set CurPos to StartPos
    CurPos = StartPos;
    while (1)  // infinite loop
    {   
        // Begin XA file playback
        if (gPlaying == 0 && CurPos == StartPos){
            // Convert sector number to CD position in min/second/frame and set CdlLOC accordingly.
            CdIntToPos(StartPos, &loc);
            // Send CDROM read command
            CdControlF(CdlReadS, (u_char *)&loc);
            // Set playing flag
            gPlaying = 1;
        }
        // When endPos is reached, set playing flag to 0
        if ((CurPos += XA_SECTOR_OFFSET) >= EndPos){
            gPlaying = 0;
        }
        // If XA file end is reached, stop playback
        if ( gPlaying == 0 && CurPos >= EndPos ){
            // Stop XA playback
            // Stop CD playback
            CdControlF(CdlStop,0);
            // Optional
            // Reset parameters
            // param[0] = CdlModeSpeed;
            // Set CD mode
            // CdControlB(CdlSetmode, param, 0);
            // Switch to next channel and start play back
            channel = !channel;
            filter.chan = channel;
            // Set filter
            CdControlF(CdlSetfilter, (u_char *)&filter);
            CurPos = StartPos;
        }

        FntPrint("Hello XA ! %d\n", VSync(-1));  
        FntPrint("Start, End Pos: %d %d\n", StartPos, EndPos);  
        FntPrint("Current Pos: %d\n", CurPos );
        FntPrint("Playback status: %d\n", gPlaying );  
        
        FntFlush(-1);               // Draw printe stream
        display();                  // Execute display()
    }
    return 0;
}
