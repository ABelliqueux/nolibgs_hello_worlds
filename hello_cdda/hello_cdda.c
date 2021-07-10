// CDDA track playing example
// based on Lameguy64's tutorial : http://lameguy64.net/svn/pstutorials/chapter1/1-display.html
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
// CD library
#include <libds.h>
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
    init();                         // init() display
    // Init extended CD system
    DsInit();
    // Init Spu
    SpuInit();
    // Set master & CD volume to max
    spuSettings.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX);
    spuSettings.mvol.left  = 0x7FFF;
    spuSettings.mvol.right = 0x7FFF;
    spuSettings.cd.volume.left = 0x7FFF;
    spuSettings.cd.volume.right = 0x7FFF;
    // Enable CD input ON
    spuSettings.cd.mix = SPU_ON;
    // Apply settings
    SpuSetCommonAttr(&spuSettings);
    // Set transfer mode 
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);

    int count = 0;
    int flip = 0;

    while (1)  // infinite loop
    {   
        if ( count == 0){
            DsPlay(2, tracks, 0);
        }
        
        count ++;
        
        // Afer 5 seconds
        if ( count == 300 ){
            // Stop playback
            DsPlay(0, tracks, 0);
        }
        // Wait one second
        if ( count == 360 ){
            // Flip value
            flip = !flip;
            // Switch track (can be 2 or 3)
            tracks[0] = 2 + flip;
            // Reset counter
            count = 0;
        }

        FntPrint("Hello CDDA !\n");  // Send string to print stream
        FntPrint("Playback status: %d", DsPlay(3, tracks, 0));  // Send string to print stream
        
        FntFlush(-1);               // Draw printe stream
        display();                  // Execute display()
    }
    return 0;
    }
