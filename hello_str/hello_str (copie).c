// Stream a STR file from CD, decompress and play it.
// Schnappy 07-2021
// based on Lameguy64 strplay library : http://www.psxdev.net/forum/viewtopic.php?t=507
// Original PsyQ sample code : /psyq/addons/cd/MOVIE
// Video to STR conversion : https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/main/hello_str
#include <sys/types.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <libapi.h>
#include <stddef.h>
#include <stdio.h>
// CD library
#include <libcd.h>
// CODEC library
#include <libpress.h>
#include <malloc.h>

#define VMODE 0                 // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320          // Screen width
#define SCREENYRES 240 + (VMODE << 4)          // Screen height : If VMODE is 0 = 240, if VMODE is 1 = 256 
#define CENTERX SCREENXRES/2    // Center of screen on x 
#define CENTERY SCREENYRES/2    // Center of screen on y
#define MARGINX 8                // margins for text display
#define MARGINY 16
#define FONTSIZE 8 * 7           // Text Field Height
DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
short db = 0;                      // index of which buffer is used, values 0, 1

#define BPP 16
#define RING_SIZE 32

// CD specifics
#define CD_SECTOR_SIZE 2048
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

// STR playback
// Display area for the str file
RECT strDispArea = { 0, 0, SCREENXRES, SCREENYRES };
// Number of frame in video file - 1
u_int strFrameCnt = 893;
// Flag set when playback has reached end of file
u_int strEndReached = 0;


// STR decompression
// Store size of uncompressed data
long strBufferSize[2];
// Allocated memory address
u_long strWorkBuffer[2][SCREENXRES/2*SCREENYRES];
// Define first slice draw area
RECT strDrawArea[2] = { 
    {0, 0, BPP, SCREENYRES},
    {0, SCREENYRES, BPP, SCREENYRES}
};

// Used to store a 16x240 image strip
u_long strStrip[2][ BPP * SCREENYRES ];

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
    setRGB0(&draw[0], 155, 0, 150); // set color for first draw area
    setRGB0(&draw[1], 155, 0, 150); // set color for second draw area
    draw[0].isbg = 1;               // set mask for draw areas. 1 means repainting the area with the RGB color each frame 
    draw[1].isbg = 1;
    PutDispEnv(&disp[0]);          // set the disp and draw environnments
    PutDrawEnv(&draw[0]);
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
int main(void)
{   
    u_char cdSpeed = {CdlModeSpeed};
    // Init display
    init();
    // Init CD system
    CdInit();
    // Initialize image processing subsystem
    DecDCTReset(0);
    // Streaming
    // Init CD ring buffer
    StSetRing(cdDataBuffer, RING_SIZE);
    // Set streaming parameters
    StSetStream(0, 0, 0xffffffff, 0, 0);
    // Set name of file to load
    loadFile = "\\COPY.STR;1";
    // Get file position from filename
    CdSearchFile( &filePos, loadFile);
    // Issue  CdlSetloc CDROM command : Set the seek target position
    CdControl(CdlSetloc, (u_char *)&filePos.pos, 0);
    // Set CD parameters
    CdControl(CdlSetmode, &cdSpeed, 0);
    CdSync(0,0);
    
    // Start streaming 
    CdRead2(CdlModeStream|CdlModeSpeed|CdlModeRT);
    
    u_long * cdDataBufferFrameAddr;
    StHEADER * cdDataBufferSector;
    int cnt = WAIT_TIME;
    //Get frame's address 
    while (StGetNext((u_long **)cdDataBufferFrameAddr, (u_long **)cdDataBufferSector) == 1){
        cnt--;
        if (cnt == 0){
            FntPrint("Time out !");
        }
    }
     //~ // Decode the VLC
    DecDCTvlc(cdDataBufferFrameAddr, strWorkBuffer[0]);
    // Free the ring buffer's address
    StFreeRing(cdDataBufferFrameAddr);
    DecDCTin( (u_long*) strWorkBuffer[0], 2);
    // Fetch decoded image in 16x240 strips
    for( strDrawArea[0].x = 0; strDrawArea[0].x < SCREENXRES; strDrawArea[0].x += 16 ){
        // Request decoded data from MDEC
        // Request 16 * 240 pixel high lines.
        // But size is in long words (4B), so divide by 2 to get words (2B) ?
        DecDCTout( strStrip[0],  (16*SCREENYRES)/2 );
        // Wait for transfer to complete 
        DecDCToutSync(0);
        // Load image data to fb
        LoadImage( &strDrawArea[0], strStrip[0] );
        DrawSync(0);
    }
    while (1)  // infinite loop
    {   
        if(!strEndReached){
            
            if(cdDataBufferSector->frameCount < strFrameCnt){
                // Send decoded data to MDEC for RLE decoding.
                DecDCTin( (u_long*) strWorkBuffer[0], 2);
                // Fetch decoded image in 16x240 strips
                for( strDrawArea[0].x = 0; strDrawArea[0].x < SCREENXRES; strDrawArea[0].x += 16 ){
                    // Request decoded data from MDEC
                    // Request 16 * 240 pixel high lines.
                    // But size is in long words (4B), so divide by 2 to get words (2B) ?
                    DecDCTout( strStrip[0],  (16*SCREENYRES)/2);
                    // Wait for transfer to complete 
                    DecDCToutSync(0);
                    // Load image data to fb
                    LoadImage( &strDrawArea[0], strStrip[0] );
                }
                
                //Get frame's address 
                cnt = WAIT_TIME;
                while (StGetNext((u_long **)cdDataBufferFrameAddr, (u_long **)cdDataBufferSector)){
                    cnt--;
                    if (cnt == 0){
                        FntPrint("Time out !");
                    }
                }
                //~ // Reset timeout counter
                cnt = WAIT_TIME;
                 //~ // Decode the VLC
                DecDCTvlc(cdDataBufferFrameAddr, strWorkBuffer[0]);
                // Free the ring buffer
                StFreeRing(cdDataBufferFrameAddr);
            } else {
                strEndReached = 1;
            }
        } else {
            StUnSetRing();
            CdControlB(CdlPause, 0, 0);
        }
        
        FntPrint("Hello STR! %d\n", VSync(-1));
        FntPrint("cdDataBuf: %x %x\n", &cdDataBufferFrameAddr, &cdDataBufferSector);
        FntPrint("Cnt: %d \n", cnt);
        FntFlush(-1);               // Draw print stream
        display();                  // Execute display()
    }
    return 0;
    }
