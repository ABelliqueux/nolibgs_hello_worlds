// This is stolen from Lameguy64 tutorial : http://lameguy64.net/svn/pstutorials/chapter1/1-display.html
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
#define SCREENYRES 240          // Screen height
#define CENTERX SCREENXRES/2    // Center of screen on x 
#define CENTERY SCREENYRES/2    // Center of screen on y
#define MARGINX 0                // margins for text display
#define MARGINY 32
#define FONTSIZE 8 * 7           // Text Field Height

DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
short db = 0;                      // index of which buffer is used, values 0, 1

SpuCommonAttr spuSettings;
// CD specifics
#define CD_SECTOR_SIZE 2048
#define RING_SIZE 32
// Converting bytes to sectors SECTOR_SIZE is defined in words, aka int
#define BtoS(len) ( ( len + CD_SECTOR_SIZE - 1 ) / CD_SECTOR_SIZE ) 
// Name of file to load
static char * loadFile;
// libcd's CD file structure contains size, location and filename
CdlFILE filePos = {0};
// Load data to this buffer - Use an array to reserve memory
u_long cdDataBuffer[RING_SIZE * CD_SECTOR_SIZE];
// Those are not strictly needed, but we'll use them to see the commands results.
// They could be replaced by a 0 in the various functions they're used with.
u_char CtrlResult[8];
// Value returned by CDread() - 1 is good, 0 is bad
int CDreadOK = 0;
// Value returned by CDsync() - Returns remaining sectors to load. 0 is good.
int CDreadResult = 0;

// XA playback
// Number of frame in video file - 1
u_int strFrameCnt = 893;
// Flag set when playback has reached end of file
u_int strEndReached = 0;

void init(void)
{
    ResetGraph(0);                 // Initialize drawing engine with a complete reset (0)
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);     // Set display area for both &disp[0] and &disp[1]
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // &disp[0] is on top  of &disp[1]
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // Set draw for both &draw[0] and &draw[1]
    SetDefDrawEnv(&draw[1], 0, 0         , SCREENXRES, SCREENYRES);     // &draw[0] is below &draw[1]
    if (VMODE)                  // PAL
    {
        SetVideoMode(MODE_PAL);
        disp[0].screen.y += 8;  // add offset : 240 + 8 + 8 = 256
        disp[1].screen.y += 8;
        }
    SetDispMask(1);                 // Display on screen    
    setRGB0(&draw[0], 150, 200, 50); // set color for first draw area
    setRGB0(&draw[1], 150, 200, 50); // set color for second draw area
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
    u_char cdSpeed = {CdlModeSpeed};
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
    // Streaming
    // Init CD ring buffer
    StSetRing(cdDataBuffer, RING_SIZE);
    // Set name of file to load
    loadFile = "\\COPY.STR;1";
    // Get file position from filename
    CdSearchFile( &filePos, loadFile);
    // Issue  CdlSetloc CDROM command : Set the seek target position
    CdControl(CdlSetloc, (u_char *)&filePos.pos, 0);
    // Set streaming parameters
    StSetStream(0, 200, 0xffffffff, 0, 0);
    // Set CD parameters
    CdControl(CdlSetmode, &cdSpeed, 0);
    CdSync(0,0);
    // Start streaming 
    CdRead2(CdlModeStream|CdlModeSpeed|CdlModeRT);
    while (1)                       // infinite loop
    {   
        FntPrint("Hello XA streaming !");  // Send string to print stream
        FntFlush(-1);               // Draw printe stream
        display();                  // Execute display()
    }
    return 0;
    }
