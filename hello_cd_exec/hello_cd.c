// Load files from CD and execute them
// Schnappy 07-2021
// based on Lameguy64's tutorial : http://lameguy64.net/svn/pstutorials/chapter1/1-display.html
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <libapi.h>
#include <malloc.h>
// CD library
#include <libcd.h>

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
#define CD_SECTOR_SIZE 2048
// Converting bytes to sectors SECTOR_SIZE is defined in words, aka int
#define BtoS(len) ( ( len + CD_SECTOR_SIZE - 1 ) / CD_SECTOR_SIZE ) 

// Name of file to load
static char * exeFile;
CdlFILE filePos = {0};
struct EXEC * exeStruct;
void * ramAddr = (void *)0x80030D40; // https://discord.com/channels/642647820683444236/663664210525290507/864936962199781387
//~ static unsigned char gHeapBuffer[0x40000];

u_long * buffer;              
u_char CtrlResult[8];
int CDreadOK = 0;
int CDreadResult = 0;
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
    setRGB0(&draw[0], 255, 50, 50); // set color for first draw area
    setRGB0(&draw[1], 255, 50, 50); // set color for second draw area
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
    int i = 0;
    // init() display
    init();          
    // Init CD system
    CdInit();
    // Init heap
    InitHeap((void *)0x80030D40, 0x40000);
    //~ InitHeap((u_long *)gHeapBuffer, sizeof(gHeapBuffer));
    // Set name of file to load
    exeFile = "\\POLY.EXE;1";
    // Get file position from filename
    CdSearchFile( &filePos, exeFile);
    // Allocate memory
    //~ EnterCriticalSection();
    buffer = malloc( BtoS(filePos.size)*CD_SECTOR_SIZE);
    //~ ExitCriticalSection();
    // Issue  CdlSetloc CDROM command : Set the seek target position
    // Beware of a misnomed 'sector' member in the CdlLOC struct that should really be named 'frame'.
    // https://discord.com/channels/642647820683444236/663664210525290507/864912470996942910
    CdControl(CdlSetloc, (u_char *)&filePos.pos, CtrlResult);
    // Read data 
    CDreadOK = CdRead( (int)BtoS(filePos.size), (u_long *)buffer, CdlModeSpeed);
    // Wait for operation to complete
    CDreadResult = CdReadSync(0, 0);

    while (1)  // infinite loop
    {   

        // Load file from CD
        //~ CdReadExec(exeFile);
        // Execute file 
        //~ StopCallback(); 
        //~ ResetGraph(3);
        //~ Exec(exeStruct,0,0);
        i++;
        FntPrint("Hello CD! %d\n", i);  // Send string to print stream
        FntPrint("Heap: - Buf: %x\n", buffer);  // Send string to print stream
        FntPrint("CdCtrl : %d\nRead  : %d %d\n", CtrlResult[0], CDreadOK, CDreadResult); 
        FntPrint("Size B: %d S: %d", filePos.size, BtoS(filePos.size));  // Send string to print stream
        
        FntFlush(-1);               // Draw printe stream
        display();                  // Execute display()
    }
    return 0;
    }
