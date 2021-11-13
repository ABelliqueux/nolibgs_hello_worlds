// CDDA track playback example
// Schnappy 07-2021
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
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
// CD tracks 
int playing = -1;
int tracks[] = {2, 0};  // Track to play , 1 is data, 2 is beach.wav, 3 is funk.wav. See isoconfig.xml

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
    int count = 0;
    int flip = 1;
    CdlLOC loc[100];
    int ntoc;
    // Init display
    init();                       
    // Init extended CD system
    CdInit();
    // Init Spu
    SpuInit();
    // Set master & CD volume to max
    spuSettings.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX);
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
    // CD Playback setup
    // Play second audio track
    // Get CD TOC
    while ((ntoc = CdGetToc(loc)) == 0) { 		/* Read TOC */
        FntPrint("No TOC found: please use CD-DA disc...\n");
    }
    // Prevent out of bound pos
    for (int i = 1; i < ntoc; i++) {
        CdIntToPos(CdPosToInt(&loc[i]) - 74, &loc[i]);
    }
    // Those array will hold the return values of the CD commands
    u_char param[4], result[8];
    // Set CD parameters ; Report Mode ON, CD-DA ON. See LibeOver47.pdf, p.188
    param[0] = CdlModeRept|CdlModeDA;	
    CdControlB (CdlSetmode, param, 0);	/* set mode */
    VSync (3);				/* wait three vsync times */
    // Play second track in toc array
    CdControlB (CdlPlay, (u_char *)&loc[3], 0);	/* play */


    while (1)  // infinite loop
    {           
        count ++;
        // Get current track number ~ every second
        // See LibeOver47.pdf, p.188
        if (count%50 == 0){
            CdReady(1, &result[0]);
            // current track number can also be obtained with 
            // CdControlB (CdlGetlocP, 0, &result[0]);
        }
        // Switch track after ~ 20 seconds
        if (count%(50*20) == 0){
            // Flip can have a value of 1 or -1
            flip *= -1;
            uint8_t nextTrackIndex = result[1] + flip;
            // Send CD command to switch track
            CdControlB (CdlPlay, (u_char *)&loc[ nextTrackIndex ], 0);
        }

        FntPrint("Hello CDDA !\n");  // Send string to print stream
        FntPrint("Playback status: %d", result[1]);  // Send string to print stream
        
        FntFlush(-1);               // Draw printe stream
        display();                  // Execute display()
    }
    return 0;
    }
