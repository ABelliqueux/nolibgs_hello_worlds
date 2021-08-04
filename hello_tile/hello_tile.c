// Draw a tile primitive
// Schnappy 2021
// based on Lameguy64's tutorial : http://lameguy64.net/svn/pstutorials/chapter1/2-graphics.html
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#define VMODE 0         // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320
#define SCREENYRES 240
#define CENTERX SCREENXRES/2
#define CENTERY SCREENYRES/2
#define MARGINX 0            // margins for text display
#define MARGINY 32
#define FONTSIZE 8 * 7       // Text Field Height
#define OTLEN 8              // Ordering Table Length 
DISPENV disp[2];             // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
u_long ot[2][OTLEN];          // double ordering table of length 8 * 32 = 256 bits / 32 bytes
char primbuff[2][32768];// double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes
char *nextpri = primbuff[0];  // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]
short db = 0;                 // index of which buffer is used, values 0, 1
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
    setRGB0(&draw[0], 50, 50, 50);
    setRGB0(&draw[1], 50, 50, 50);
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
    //~ DrawOTag(ot[db]);
    db = !db;
    nextpri = primbuff[db];
}
int main(void)
{
    // These two tiles are added at the same OT index
    TILE * blue_tile;
    TILE * pink_tile;
    // This one is added at a different OT index
    TILE * yellow_tile;
    init();
    while (1)
    {
        // Initialize the reversed ordering table. This means the elements at index OTLEN - 1 is drawn first.
        ClearOTagR(ot[db], OTLEN);
        // Use regular order OT, uncomment l.53 accordingly
        //~ ClearOTag(ot[db], OTLEN); 
        // yellow_tile is before pink and blue tile in the code, 
        // and it displays behind because it is added to a different ot index (od[db] + OTLEN - 1)
        // Using a Regular or Reverse OT will have an effect on drawing order. (See lines 53 and 73)
        yellow_tile = (TILE * ) nextpri;                   // yellow_tile is a pointer to primbuf content at adress nextpri, that's cast (type converted) to a TILE struc.   
        setTile(yellow_tile);                              // initialize the TILE structure ( fill the length and tag(?) value )
        setXY0(yellow_tile, CENTERX - 32 , CENTERY - 48);  // Set X,Y
        setWH(yellow_tile, 128, 40);                       // Set Width, Height
        setRGB0(yellow_tile, 255, 255, 0);                 // Set color
        addPrim(ot[db][OTLEN - 1], yellow_tile);           // Add primitive to ordering table
        nextpri += sizeof(TILE);     
        // blue_tile added at od[db] + OTLEN - 2
        blue_tile = (TILE * ) nextpri;                  // blue_tile is a pointer to primbuf content at adress nextpri, that's cast (type converted) to a blue_tile struc.   
        setTile(blue_tile);                              // initialize the blue_tile structure ( fill the length and tag(?) value )
        setXY0(blue_tile, CENTERX - 16, CENTERY - 32);   // Set X,Y
        setWH(blue_tile, 32, 64);                        // Set Width, Height
        setRGB0(blue_tile, 60, 180, 255);                // Set color
        addPrim(ot[db][OTLEN - 2], blue_tile);           // Add primitive to ordering table
        nextpri += sizeof(TILE);                         // Increment the adress nextpri points to by the size of TILE struct
        // pink_tile is after blue_tile in the code, 
        // so it is drawn before, thus under blue_tile.
        // However, it is added at the same ot index (od[db] + OTLEN - 2)
        // so using a Regular or Reverse OT won't have an effect on drawing order.
        pink_tile = (TILE * ) nextpri;                  // pink_tile is a pointer to primbuf content at adress nextpri, that's cast (type converted) to a TILE struc.   
        setTile(pink_tile);                              // initialize the TILE structure ( fill the length and tag(?) value )
        setXY0(pink_tile, CENTERX, CENTERY - 64);        // Set X,Y
        setWH(pink_tile, 64, 64);                        // Set Width, Height
        setRGB0(pink_tile, 255, 32, 255);                // Set color
        addPrim(ot[db][OTLEN - 2], pink_tile);           // Add primitive to ordering table
        nextpri += sizeof(TILE);     
        FntPrint("Hello tile !");
        FntFlush(-1);
        display();
        }
    return 0;
    }
