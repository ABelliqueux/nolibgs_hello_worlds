// Using the strplay library.
// Schnappy 07-2021
// based on Lameguy64 strplay library : http://www.psxdev.net/forum/viewtopic.php?t=507
// Original PsyQ sample code : /psyq/addons/cd/MOVIE
// Video to STR conversion : https://github.com/ABelliqueux/nolibgs_hello_worlds/wiki/STR
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
// CD library
#include <libcd.h>
// CODEC library
#include <libpress.h>
// include Lameguy64's library
#include "strplay.c"

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

STRFILE StrFile[] = {
	// File name	Resolution		Frame count
	"\\COPY.STR;1", 320, 240, 893
};

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

void main() {
	
	// Reset and initialize stuff
	ResetCallback();
	CdInit();
	PadInit(0);
	ResetGraph(0);
	SetGraphDebug(0);
	
	// Play the video in loop
	while (1) {
		
		if (PlayStr(320, 240, 0, 0, &StrFile[0]) == 0)	// If player presses Start
			break;	// Exit the loop
		
	}
	
}

static void strCheckRGB24() {

    /* From http://psx.arthus.net/sdk/Psy-Q/DOCS/, p.713
     * When playing a movie in 24-bit mode, there is a potential hardware conflict between the CD subsystem
     * and the MDEC image decompression system which can result in corrupted data. To avoid this,
     * StCdInterrupt() may defer transferring a sector and instead set a flag variable called StCdInterFlag to
     * indicate that a CD sector is ready to be transferred. Once the MDEC is finished transferring data, your
     * application should check StCdIntrFlag and call StCdInterrupt() directly if it is set.
     */
    #if TRUECOL
        extern u_long StCdIntrFlag;
        // If flag was set
        if ( StCdIntrFlag ) {
            // Trigger data transfer
            StCdInterrupt();
            // Reset flag
            StCdIntrFlag = 0;
        }
    #endif
}
