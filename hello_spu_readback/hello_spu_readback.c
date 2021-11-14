// SPU readback example
// adapted from PsyQ's sample : psyq/psx/sample/sound/CDVOL, main.c,v 1.14 1997/05/02 13:05:21 by ayako 
// Schnappy 11-2021
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <kernel.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
// CD library
#include <libcd.h>
// SPU library
#include <libspu.h>
#include "../thirdparty/nugget/common/syscalls/syscalls.h"
#define printf ramsyscall_printf

#define VMODE 0                     // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320              // Screen width
#define SCREENYRES 240 + (VMODE << 4) // Screen height : If VMODE is 0 = 240, if VMODE is 1 = 256 
#define CENTERX SCREENXRES/2        // Center of screen on x 
#define CENTERY SCREENYRES/2        // Center of screen on y
#define MARGINX 0                   // margins for text display
#define MARGINY 32
#define FONTSIZE 8 * 7              // Text Field Height
#define OTLEN 8                     // Ordering Table Length 
// Number of bars 
#define BARNUM 2
// Peak cursor width
#define TSIZE 10
// Bar size / 2
#define BSIZE 128
// Top Y coordinate of Left volume bar
#define BARTOP	100
// Bottom Y coordinates of Left volume bar
#define BARBOTTOM ((BARTOP)+5)
// Vertical spacing
#define MARGIN 40
// Bars left coordinates
#define MINBAR	CENTERX - BSIZE
// Bars right coordinates
#define MAXBAR	( CENTERX + BSIZE + TSIZE )
// Bars IDs
#define LEFTBAR 0
#define RIGHTBAR 1
// Return absolute value of a number
#define ABS(x) (((x)<0)?(-(x)):(x))

DISPENV disp[2];                    // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
u_long ot[2][OTLEN];                // double ordering table of length 8 * 32 = 256 bits / 32 bytes
uint8_t primbuff[2][32768];            // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes
uint8_t *nextpri = primbuff[0];        // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]
uint8_t db = 0;                     // index of which buffer is used, values 0, 1
// SPU attributes
SpuCommonAttr spuSettings;
// SPU IRQ address
uint16_t SpuIrqAddr;
// SPU decoded data buffer
SpuDecodedData decodedData;
// CD volume: current sample's max values
ulong leftMax,  rightMax;
// Last 2 seconds's peak volume values
ulong leftPeak, rightPeak;

// Primitives for drawing the VU-metre, double buffered
// Blue : background bar
POLY_F4 * bar[BARNUM];
// White : current value
POLY_F4	* current[BARNUM];
// Red : volume peak in the last 3 seconds
POLY_F4	* peak[BARNUM];
// Colors for the VU-metre
CVECTOR bg = {20, 10, 0};
CVECTOR fg = {10,200,20};
CVECTOR cursor = {200,40,10};

void init(void)
{
    ResetGraph(0);                 // Initialize drawing engine with a complete reset (0)
    InitGeom();
    SetGeomOffset(CENTERX,CENTERY);
    SetGeomScreen(CENTERX);
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
    // Top bar
    FntOpen (MINBAR, BARTOP - 10, 200, 150, 0, 64);
    // Bottom bar
    FntOpen (MINBAR, BARTOP - 10 + (MARGIN), 200, 150, 0, 64);
    // Debug
    FntOpen (32, SCREENYRES - 74, SCREENXRES - 64, 64, 0, 200);
}

void display(void)
{
    DrawSync(0);
    VSync(0);
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    DrawOTag(&ot[db][OTLEN - 1]);
    db = !db;
    nextpri = primbuff[db];
}

void initPrimitives(void)
{
    // Set primitives from primbuff[]
    bar[0] = (POLY_F4 *)nextpri;
    bar[1] = (POLY_F4 *)nextpri + sizeof(POLY_F4);
    
    current[0] = (POLY_F4 *)nextpri + (sizeof(POLY_F4) * 2);
    current[1] = (POLY_F4 *)nextpri + (sizeof(POLY_F4) * 3);
    
    peak[0] = (POLY_F4 *)nextpri + (sizeof(POLY_F4) * 4);
    peak[1] = (POLY_F4 *)nextpri + (sizeof(POLY_F4) * 5);

    // Set each primitive to their default settings
    for (int i = 0; i < BARNUM; i++)
    {        
        // Volume bar background is blue
        SetPolyF4 ( bar[i] );
        setRGB0 ( bar[i], bg.r,bg.g,bg.b );
        setXY4 ( bar[i],
            // Top-left
            MINBAR, BARTOP + i * MARGIN,
            // Top-right
            MAXBAR, BARTOP + i * MARGIN,
            // Bottom-left
            MINBAR, BARBOTTOM + i * MARGIN,
            // Bottom-right
            MAXBAR, BARBOTTOM + i * MARGIN);

        // Current volume is light purple-ish
        SetPolyF4 (current[i]);
        setRGB0 ( current[i], fg.r,fg.g,fg.b);
        setXY4 ( current[i],
            MINBAR,         BARTOP + i * MARGIN,
            MINBAR + TSIZE, BARTOP + i * MARGIN,
            MINBAR,         BARBOTTOM + i * MARGIN,
            MINBAR + TSIZE, BARBOTTOM + i * MARGIN);

        // Initialize peak cursor
        SetPolyF4 ( peak[i] );
        setRGB0 ( peak[i], cursor.r,cursor.g,cursor.b);
        setXY4 ( peak[i],
            MINBAR,         BARTOP + i * MARGIN,
            MINBAR + TSIZE, BARTOP + i * MARGIN,
            MINBAR,         BARBOTTOM + i * MARGIN,
            MINBAR + TSIZE, BARBOTTOM + i * MARGIN);
    }
}

// Unused - should be called whenever this madness needs to be ended
void terminate(void)
{
    // Turn SPU irq off
    SpuSetIRQ (SPU_OFF); 
    // Clear callback functions 
    SpuSetIRQCallback ((SpuIRQCallbackProc) NULL);
    SpuSetTransferCallback ((SpuTransferCallbackProc) NULL);
    // Reset SPU settings
    spuSettings.mask = (SPU_COMMON_MVOLL |
		   SPU_COMMON_MVOLR |
		   SPU_COMMON_CDVOLL |
		   SPU_COMMON_CDVOLR |
		   SPU_COMMON_CDMIX
		   );
    spuSettings.mvol.left       = 0;
    spuSettings.mvol.right      = 0;
    spuSettings.cd.volume.left  = 0;
    spuSettings.cd.volume.right = 0;
    spuSettings.cd.mix	   = SPU_OFF;

    SpuSetCommonAttr (&spuSettings);

    // Stop CD
    CdStop ();
    // Stop SPU processing
    SpuQuit ();
    // Re-init display env
    ResetGraph (3);
    // stop callback processing
    StopCallback ();		
}

// Print corresponding data for each volume bar
void printDataInfo(void)
{
    // We're using 2 streams
    FntPrint (0, "L: %04x peak/%04x\n", leftMax, leftPeak);
    FntPrint (1, "R: %04x peak/%04x\n", rightMax, rightPeak);
    FntFlush (0);
    FntFlush (1);
}

// SPU IRQ calback function
void eachIRQ (void)
{
    SpuSetIRQ (SPU_OFF); /**/
    SpuReadDecodeData (&decodedData, SPU_CDONLY); /**/
}

// DMA Transfer callback function
void eachDMA (void)
{
    if (SpuIrqAddr == 0x0)
        SpuIrqAddr = 0x200;
    else
        SpuIrqAddr = 0x0;
    // Change IRQ address
    SpuSetIRQAddr (SpuIrqAddr);
    // Turn SPU IRQ requests on
    SpuSetIRQ (SPU_ON);
}

void findSampleMaxVolume(void)
{
    // Search maximum volume value of the SPU decoded data 
    // SPU buffer data range adresses
    long dataLowerAdress, dataUpperAdress;
    // Current sample's max and working value
    short maxL = 0, tmpL;
    short maxR = 0, tmpR;
    // Timers for the Peak cursor, reset after 120 iterations.
    static long timeCursorL = 0, timeCursorR = 0;
    // Find SPU data range according to current half we're working on
    if (SpuIrqAddr == 0x0) {
        /* 1st part is available */
        dataLowerAdress = 0x0;
        dataUpperAdress = 0x1ff;
    } else {
        /* 2nd part is available */
        dataLowerAdress = 0x200;
        dataUpperAdress = 0x3ff;
    }
    // Examine and find max volume in the data range
    for (long i = dataLowerAdress; i < dataUpperAdress; i ++) {
        // Examine SPU decoded data 
        tmpL = ABS(decodedData.cd_left[i]);
        tmpR = ABS(decodedData.cd_right[i]);
        // Only keep maximum value for this sample
        if (maxL < tmpL ) {
            maxL = tmpL ;
        }
        if (maxR < tmpR ) {
            maxR = tmpR;
        }
    }
    leftMax  = (long) maxL;
    rightMax = (long) maxR;
    // Peak level
    if (leftPeak < leftMax) {
        leftPeak = leftMax;
        timeCursorL = 0;
    }
    if (rightPeak < rightMax) {
        rightPeak = rightMax;
        timeCursorR = 0;
    }
    // Peak cursors: hold 2s@60fps.
    // Increment counters until 120 is reached, then set cursors position to current leftMax/rightMax values
    if (timeCursorL < 120) {
        timeCursorL ++;  
    } else {
        timeCursorL = 0;
        leftPeak = leftMax;
    }
    if (timeCursorR < 120) {
        timeCursorR ++;
    } else {
        timeCursorR = 0;
        rightPeak = rightMax;
    }
}


int main(void)
{    
    // Values used to switch CD track
    u_int counter = 0;
    int8_t flip = 1;
    // These will hold the normalised values of leftMax/rightMax, leftPeak/rightPeak
    long lMax, rMax, lPeak, rPeak;
    // Init display
    init();                         
    // Init CD system
    CdInit ();
    // Init Spu
    SpuInit();
    // Initialize SPU related variables
    leftMax  = rightMax  = 0;
    leftPeak = rightPeak = 0;
    // Fill SPU data buffers with 0s
    for (int i = 0; i < SPU_DECODEDDATA_SIZE; i ++) {
        decodedData.cd_left[i] = 0;
        decodedData.cd_right[i] = 0;
    }
    // SPU setup
    // Set master & CD volume to max
    spuSettings.mask = (SPU_COMMON_MVOLL |
                        SPU_COMMON_MVOLR |
                        SPU_COMMON_CDVOLL |
                        SPU_COMMON_CDVOLR |
                        SPU_COMMON_CDMIX);
    // Master volume should be in range 0x0000 - 0x3fff
    spuSettings.mvol.left  = 0x3fff;
    spuSettings.mvol.right = 0x3fff;
    // Cd volume should be in range 0x0000 - 0x7fff
    spuSettings.cd.volume.left = 0x7fff;
    spuSettings.cd.volume.right = 0x7fff;
    // Enable CD input ON
    spuSettings.cd.mix = SPU_ON;
    // Apply settings
    SpuSetCommonAttr(&spuSettings);
    // Set transfer mode 
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    // Callbacks setup
    // Set Transfer callback
    (void) SpuSetTransferCallback ((SpuTransferCallbackProc) eachDMA);
    // set IRQ callback
    SpuSetIRQCallback ((SpuIRQCallbackProc) eachIRQ);
    // Initialize SPU IRQ address
    SpuIrqAddr = 0x200;
    // Set IRQ address
    SpuSetIRQAddr (SpuIrqAddr); 
    // Turn interrupt request ON
    SpuSetIRQ(SPU_ON);
    // CD Playback setup
    // Play second audio track
    // Get CD TOC
    CdlLOC loc[100];
    int ntoc;
    while ((ntoc = CdGetToc(loc)) == 0) { 		/* Read TOC */
        printf("No TOC found: please use CD-DA disc...\n");
        FntPrint(2, "No TOC found: please use CD-DA disc...\n");
    }
    // Prevent out of bound pos
    for (int i = 1; i < ntoc; i++) {
        CdIntToPos(CdPosToInt(&loc[i]) - 74, &loc[i]);
    }
    // Those array will hold the return values of the CD commands
    u_char param[4], result[8];
    // Set CD parameters ; Report Mode ON, CD-DA ON. See LibeOver47.pdf, p.188
    param[0] = CdlModeRept|CdlModeDA;
    // Set CD mode
    CdControlB (CdlSetmode, param, 0);	
    // Wait 3 vsync
    VSync (3);
    // Play second track in toc array
    CdControlB (CdlPlay, (u_char *)&loc[3], 0);
    // Graphics setup 
    initPrimitives();    
    while (1) 
    { 
        counter++;
        ClearOTagR(ot[db], OTLEN);
        
        // Normalize volume 
        lMax  = (leftMax   * 256) / 0x8000 + MINBAR;
        rMax  = (rightMax  * 256) / 0x8000 + MINBAR;
        lPeak = (leftPeak  * 256) / 0x8000 + MINBAR;
        rPeak = (rightPeak * 256) / 0x8000 + MINBAR;
        
        // Update primitives XY coordinates
        // Set coordinates for volume bar polygons
        setXY4 ( current[LEFTBAR],
            MINBAR,     BARTOP,
            lMax + TSIZE, BARTOP,
            MINBAR,     BARBOTTOM,
            lMax + TSIZE, BARBOTTOM);
        setXY4 (current[RIGHTBAR],
            MINBAR,     BARTOP + MARGIN,
            rMax + TSIZE, BARTOP + MARGIN,
            MINBAR,     BARBOTTOM + MARGIN,
            rMax + TSIZE, BARBOTTOM + MARGIN);
        // Set coordinates for peak cursor polygons
        setXY4 (peak[LEFTBAR],
            lPeak,         BARTOP,
            lPeak + TSIZE, BARTOP,
            lPeak,         BARBOTTOM,
            lPeak + TSIZE, BARBOTTOM);
        setXY4 (peak[RIGHTBAR],
            rPeak,         BARTOP + MARGIN,
            rPeak + TSIZE, BARTOP + MARGIN,
            rPeak,         BARBOTTOM + MARGIN,
            rPeak + TSIZE, BARBOTTOM + MARGIN);

        // Add prims to ordering table from bottom to top
        for ( int i = 0; i < BARNUM; i++) {
            addPrim(ot[db][OTLEN - 1], bar[i]);    
            addPrim(ot[db][OTLEN - 2], current[i]);
            addPrim(ot[db][OTLEN - 3], peak[i]);   
        }

        // Get current track number ~ every second
        // See LibeOver47.pdf, p.188
        if (counter%50 == 0){
            CdReady(1, &result[0]);
            // current track number can also be obtained with 
            // CdControlB (CdlGetlocP, 0, &result[0]);
        }
        // Switch track after ~ 20 seconds
        if (counter%(50*20) == 0){
            // Flip can have a value of 1 or -1
            flip *= -1;
            uint8_t nextTrackIndex = result[1] + flip;
            // Send CD command to switch track
            CdControlB (CdlPlay, (u_char *)&loc[ nextTrackIndex ], 0);
        }
        // Update current and peak values
        findSampleMaxVolume();
        // Print bar's infos
        printDataInfo();
        // Draw debug stream
        FntPrint(2, "Hello SPU readback ! %d\n", counter);
        FntPrint(2, "Current track: %d\n", result[1] );
        FntPrint(2, "L: %08d, R: %08d\n", leftMax, rightMax);
        FntPrint(2, "SPU Addr: 0x%03x ", SpuIrqAddr );
        FntFlush(2);        
        // Update display
        display();
    }
    return 0;
}
