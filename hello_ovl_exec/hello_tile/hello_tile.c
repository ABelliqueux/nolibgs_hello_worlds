#include "../common.h"

DISPENV disp[2];             // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
u_long ot[2][OTLEN];          // double ordering table of length 8 * 32 = 256 bits / 32 bytes
char primbuff[2][32768];      // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes
char *nextpri = primbuff[0];  // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]
uint8_t db = 0;                 // index of which buffer is used, values 0, 1
CVECTOR BGcolor = { 50, 255, 150 };

void init(void)
{
    ResetGraph(0);
    SetDefDispEnv(&disp[0], 0, 0, SCREENXRES, SCREENYRES);
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[1], 0, 0, SCREENXRES, SCREENYRES);
    if (VMODE)
    {
        SetVideoMode(MODE_PAL);
        disp[0].screen.y += 8;
        disp[1].screen.y += 8;
        }
    SetDispMask(1);                 // Display on screen    
    setRGB0(&draw[0], BGcolor.r, BGcolor.g, BGcolor.b); // set color for first draw area
    setRGB0(&draw[1], BGcolor.r, BGcolor.g, BGcolor.b); // set color for second draw area
    draw[0].isbg = 1;
    draw[1].isbg = 1;
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    FntLoad(960, 0);
    FntOpen(MARGINX, SCREENYRES - MARGINY - FONTSIZE, SCREENXRES - MARGINX * 2, FONTSIZE, 0, 280 );
}
void display(void)
{
    DrawSync(0);
    VSync(0);
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    // We're using a reverse OT, so we want to display the last item first. See PsyQ's LibRef47.pdf, p.277
    DrawOTag(&ot[db][OTLEN - 1]);
    // Comment above line, and uncomment the following line to use a regular oredered OT. Comment l.71 and Uncomment l.73 accordingly
    db = !db;
    nextpri = primbuff[db];
}

#include "hello_ovl_tile.c"

int main(void)
{
    ovl_main_tile();
};
