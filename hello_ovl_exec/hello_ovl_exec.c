// Load overlay files from CD and execute them
// Schnappy 10-2021
// With the help of Nicolas Noble and impiaa 
// https://discord.com/channels/642647820683444236/663664210525290507/894624082367229952
#include "common.h"

DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
char primbuff[2][32768];        // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes
u_long ot[2][OTLEN];
char *nextpri = primbuff[0];  
uint8_t db = 0;    
CVECTOR BGcolor = { 0, 150, 255 };

extern u_long load_all_overlays_here;

typedef struct Overlay {
  char filename[0x7c];
  int (*main)();
  char commandline[0x180];
} Overlay;

int ovl_main_hello();
int ovl_main_tile();
int ovl_main_poly();

Overlay g_chainload[] = {
    {"\\HELLO.OVL;1", ovl_main_hello, ""},
    {"\\TILE.OVL;1", ovl_main_tile, ""},
    {"\\POLY.OVL;1", ovl_main_poly, ""},
    //~ { "\\POLY.OVL;1" , 0 },
};

enum OverlayNumber next_overlay = MOTHERSHIP;
enum OverlayNumber prev_overlay = MOTHERSHIP;

int main(void);
int loadOverlayAndStart(Overlay *);
void EmptyOTag(u_long ot[2][OTLEN], char  primbuff[2][32768], char * nextpri );

void (*dispp) (void);
int CDreadOK = 0;

void init(void)
{
    ResetCallback();
    ResetGraph(0);                 // Initialize drawing engine with a complete reset (0)
    // Initialize and setup the GTE
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
    setRGB0(&draw[0], BGcolor.r, BGcolor.g, BGcolor.b); // set color for first draw area
    setRGB0(&draw[1], BGcolor.r, BGcolor.g, BGcolor.b); // set color for second draw area
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
    DrawOTag(&ot[db][OTLEN - 1]);
    db = !db;                       // flip db value (0 or 1)
    nextpri = primbuff[db];
}

int loadOverlayAndStart(Overlay * overlay)
{
    int CDreadResult = 0;
    int next_ovl = -1;
    // Load overlay file
    CDreadResult = CdReadFile(overlay->filename, &load_all_overlays_here, 0);
	CdReadSync(0, 0);
    printf( "CD read: %d", CDreadResult);
    // If file loaded sucessfully
    if (CDreadResult)
    {
        //~ StopCallback(); 
        ResetGraph(3);
        // Execute
        next_ovl = overlay->main();
        EmptyOTag(&ot[db], &primbuff[db], nextpri);
        setRGB(&BGcolor, 0, 150, 255);
        init();
    }
    return next_ovl;
}

void EmptyOTag(u_long ot[2][OTLEN], char primbuff[2][32768], char * nextpri )
{
    for (uint16_t p; p < OTLEN; p++)
    {
        ot[0][p] = 0;
        ot[1][p] = 0;
    }
    nextpri = primbuff[!db];
}

int main(void)
{   
    int t = 0;
    // init() display
    init();          
    // Init CD system
    CdInit();
    
    prev_overlay = next_overlay; 
    while(1){
        if (t == 100){
            next_overlay = OVERLAY_HELLO;
        }
        if (next_overlay != -1 && next_overlay != prev_overlay){
            prev_overlay = next_overlay;
            next_overlay = loadOverlayAndStart(&g_chainload[next_overlay]);
            t = 0;
        }
        t++;
        FntPrint("Mothership:  %d\nOvl: %d %d", t, prev_overlay, next_overlay);
        FntFlush(-1);
        display();
    }
    //
    return 0;
}
