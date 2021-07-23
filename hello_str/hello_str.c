// Stream a STR file from CD, decompress and play it.
// Schnappy 07-2021
// based on Lameguy64 strplay library : http://www.psxdev.net/forum/viewtopic.php?t=507
// Original PsyQ sample code : /psyq/addons/cd/MOVIE
// Video to STR conversion : https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/main/hello_str
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
// CD library
#include <libcd.h>
// CODEC library
#include <libpress.h>

#define VMODE 0                 // Video Mode : 0 : NTSC, 1: PAL
#define TRUECOL 1               // 0 : 16bpp, 1: 24bpp
#define SCREENXRES 320          // Screen width
#define SCREENYRES 240 + (VMODE << 4)          // Screen height : If VMODE is 0 = 240, if VMODE is 1 = 256 
#define CENTERX SCREENXRES/2    // Center of screen on x 
#define CENTERY SCREENYRES/2    // Center of screen on y
#define MARGINX 8                // margins for text display
#define MARGINY 16
#define FONTSIZE 8 * 7           // Text Field Height
DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
short db = 0;                    // index of which buffer is used, values 0, 1

#define STR_POS_X 0
// If PAL mode, add 8 pixels offset on Y (256-240)/2 
#define STR_POS_Y (VMODE << 4)/2
// Ring Buffer size (32 sectors seems good enough)
#define RING_SIZE   32  

#if TRUECOL
    // pixels per short word (16b/2B)
    // 1px is 3B in 24bpp
    // 1px is 2B(one word) in 16bpp
    // therefore 2B will hold 3/2 pixels
    #define PPW         3/2 
    // DCT mode - bit 0 : depth (0 = 16b, 1 = 24b), bit 1: in 16b mode, set STP(Semi-Transparency) bit 15.
    // 24bpp = 01b => 1
    #define DCT_MODE    1
#else
    #define PPW         1
    // 16bpp = 10b => 2
    #define DCT_MODE    2
#endif

// Stop playback if set
static int endPlayback = 0;
// STR file infos : Filename on CD, Width, Height, Length in frames
static char * StrFileName = "\\COPY.STR;1";
static int StrFileX = 320; 
static int StrFileY  = 240;
static int StrFileLength  = 893;

// When using RGB24, a special routine has to be setup as a callback function to avoid MDEC/CDROM conflict
// See http://psx.arthus.net/sdk/Psy-Q/DOCS/LibRef47.pdf , p.713
static void strCheckRGB24();

void init(void)
{
    ResetCallback();
    ResetGraph(0);                 // Initialize drawing engine with a complete reset (0)
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);     // Set display area for both &disp[0] and &disp[1]
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // &disp[0] is on top  of &disp[1]
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // Set draw for both &draw[0] and &draw[1]
    SetDefDrawEnv(&draw[1], 0, 0         , SCREENXRES, SCREENYRES);     // &draw[0] is below &draw[1]
    // Set video mode
    #if VMODE
        SetVideoMode(MODE_PAL);
        disp[0].disp.y = 8;
        disp[1].disp.y = 8;
    #endif
    SetDispMask(1);                 // Display on screen    
    setRGB0(&draw[0], 155, 0, 150); // set color for first draw area
    setRGB0(&draw[1], 155, 0, 150); // set color for second draw area
    draw[0].isbg = 0;               // set mask for draw areas. 1 means repainting the area with the RGB color each frame 
    draw[1].isbg = 0;
    #if TRUECOL
        disp[0].isrgb24 = 1;
        disp[1].isrgb24 = 1;
    #endif
    PutDispEnv(&disp[db]);          // set the disp and draw environnments
    PutDrawEnv(&draw[db]);
    FntLoad(960, 0);                // Load font to vram at 960,0(+128)
    FntOpen(MARGINX, MARGINY, SCREENXRES - MARGINX * 2, FONTSIZE, 0, 280 ); // FntOpen(x, y, width, height,  black_bg, max. nbr. chars
}
void display(void)
{
    DrawSync(0);                    // Wait for all drawing to terminate
    VSync(0);                       // Wait for the next vertical blank
    PutDispEnv(&disp[db]);          // set alternate disp and draw environnments
    PutDrawEnv(&draw[db]);  
    db = !db;                       // flip db value (0 or 1)
}

int main() {
   // CD File descriptor
    CdlFILE STRfile;
    // Parameter we want to set the CDROM with
    u_char param=CdlModeSpeed;
    // Buffers total ~110KB in memory 
    // SECTOR_SIZE is defined in words : 512 * 4 == 2048 Bytes/sector
    // Ring buffer : 32 * 2048 = 65536 Bytes
    u_long  RingBuff[ RING_SIZE * SECTOR_SIZE ];
    // VLC buffers : display area in words (hence /2), 160*320 == 38400 Bytes
    u_long  VlcBuff[2][ SCREENXRES / 2 * StrFileY ];  
    // If using 16bpp, fetch 16xYres strips, if 24bpp, fetch 24xYres strips, 5120*PPW Bytes 
    u_short ImgBuff[2][ 16 * PPW * StrFileY];
    u_long * curVLCptr = VlcBuff[db];
    // Init Disp/Draw env, Font, etc.
    init();
    // Init CDrom system
    CdInit();
    // Reset the MDEC
    DecDCTReset(0);
    // Set callback routine
    DecDCToutCallback(strCheckRGB24);
    // Set ring buffer
    StSetRing(RingBuff, RING_SIZE);
    // Set streaming parameters
    StSetStream(TRUECOL, 1, StrFileLength, 0, 0);
    // Get the CD location of the STR file to play
    if ( CdSearchFile(&STRfile, StrFileName) == 0 ) {
        printf("File not found :%s\n", StrFileName);
        //~ SetDispMask(1);
    }
    // Set the seek target position
    CdControl(CdlSetloc, (u_char *)&STRfile.pos, 0);
    // Set CD mode to CdlModeSpeed
    CdControl(CdlSetmode, &param, 0);
    // Read from CD at position &STRfile.pos
    // Enable streaming, double speed and ADPCM playback
    CdRead2(CdlModeStream|CdlModeSpeed|CdlModeRT);
    // Use a counter to avoid deadlocks
    int wait = WAIT_TIME;
    // Next Ring Buffer Frame address
    u_long * nextFrame = 0;
    // Ring buffer frame address
    u_long * frameAddr = 0;
    // Ring buffer frame header
    StHEADER * sectorHeader;
    
    // Main loop
    while (1) {
        u_long * curVLCptr = &VlcBuff[db][0];
        u_short * curIMGptr = &ImgBuff[db][0];

        // While end of str is not reached, play it
        while (!endPlayback) {
            // Use this area to draw the slices
            RECT curSlice = { 0, 
                               (db * StrFileY) + STR_POS_Y,
                              // In 24bpp, use 24 pixels wide slices
                              16 * PPW ,
                              StrFileY};
            int frameDone = 0;
            // Reset counter
            wait = WAIT_TIME;
            // Dont try decoding if not data has been loaded from ring buffer
            if ( frameAddr ){
                // Begin decoding RLE-encoded MDEC image data
                DecDCTin( &VlcBuff[db][0] , DCT_MODE);
                // Prepare to receive the decoded image data from the MDEC
                while (curSlice.x < STR_POS_X + SCREENXRES * PPW) {
                    // Receive decoded data : a 16*ppw*240 px slice 
                    DecDCTout( (u_long *) &ImgBuff[db][0], curSlice.w * curSlice.h / 2);
                    // Wait for transfer end
                    DecDCToutSync(1);
                    // Transfer data from main memory to VRAM
                    LoadImage(&curSlice, (u_long *) &ImgBuff[db][0]);
                    // Increment drawArea's X with slice width (16 or 24 pix)
                    curSlice.x += 16 * PPW;
                }
                 // Set frameDone flag to 1
                frameDone = 1;

                curSlice.x = STR_POS_X;
                curSlice.y = (db * StrFileY) + STR_POS_Y;
            }
            // Get one frame of ring buffer data
            // StGetNext is non-blocking, so we wait for it to return 0.
            // StGetNext will lock the region at &frameAddr until StFreeRing() is called.
            while ( StGetNext((u_long **)&frameAddr,(u_long **)&sectorHeader) ) {
                wait--;
                if (wait == 0)
                    break;
            }
            // If the current frame's number is bigger than the number of frames in STR,
            // set the endPlayback flag.
            if (sectorHeader->frameCount >= StrFileLength)
                endPlayback = 1;
            // Grab a frame from the stream
            wait = WAIT_TIME;
            while ((nextFrame = frameAddr) == 0) {
                wait--;
                if ( wait == 0 ){ 
                    break;
                }
            }
            // Decode the Huffman/VLC compressed data
            DecDCTvlc(nextFrame, &VlcBuff[!db][0]);
            // Unlock area obtained by StGetNext()
            StFreeRing(nextFrame);
            // Reset counter
            wait = WAIT_TIME;
            // Wait until the whole frame is loaded to VRAM
            while ( frameDone == 0 ) {
                wait--;
                if ( wait == 0 ) { 
                    // If a timeout occurs, force switching buffers
                    frameDone = 1;
                    curSlice.x = STR_POS_X;
                    curSlice.y = (db * StrFileY) + STR_POS_Y;
                }
            }
            FntFlush(-1);
            display();
        }
        // Disable callback
        DecDCToutCallback(0);
        // Release two interrupt functions CdDataCallback() and CdReadyCallback() hooked by CDRead2()
        StUnSetRing();
        // Put CDROM on pause at current position
        CdControlB(CdlPause, 0, 0);
    }
    return 0;
};

static void strCheckRGB24() {

    /* From http://psx.arthus.net/sdk/Psy-Q/DOCS/, p.713
     * When playing a movie in 24-bit mode, there is a potential hardware conflict between the CD subsystem
     * and the MDEC image decompression system which can result in corrupted data. To avoid this,
     * StCdInterrupt() may defer transferring a sector and instead set a flag variable called StCdInterFlag to
     * indicate that a CD sector is ready to be transferred. Once the MDEC is finished transferring data, your
     * application should check StCdIntrFlag and call StCdInterrupt() directly if it is set.
     */
    #if TRUECOL
        extern int StCdIntrFlag;
        // If flag was set
        if ( StCdIntrFlag ) {
            // Trigger data transfer
            StCdInterrupt();
            // Reset flag
            StCdIntrFlag = 0;
        }
    #endif
}
