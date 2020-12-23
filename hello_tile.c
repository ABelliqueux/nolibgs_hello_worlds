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

char primbuff[2][32768] = {1};// double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes

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
    
    SetDispMask(1);
    
    DrawOTag(ot[db] + OTLEN - 1);
    
    db = !db;
    
    nextpri = primbuff[db];
}


int main(void)
{
    
    TILE * blue_tile;
    TILE * pink_tile;
    
    init();
    
    while (1)
    {
        ClearOTagR(ot[db], OTLEN);                      // Initialize the reversed ordering table. Last element will be drawn first.
        
        blue_tile = (TILE * ) nextpri;		            // blue_tile is a pointer to primbuf content at adress nextpri, that's cast (type converted) to a blue_tile struc.   
		
		setTile(blue_tile);                              // initialize the blue_tile structure ( fill the length and tag(?) value )
		setXY0(blue_tile, CENTERX - 16, CENTERY - 32);   // Set X,Y
		setWH(blue_tile, 32, 64);                        // Set Width, Height
		setRGB0(blue_tile, 60, 180, 255);                // Set color
		addPrim(ot[db], blue_tile);                      // Add primitive to ordering table
            
		nextpri += sizeof(TILE);                    // Increment the adress nextpri points to by the size of TILE struct
    
        // pink_tile is after blue_tile in the code, 
        // but we're using a reversed ordering table
        // so it is drawn before, thus bellow blue_tile.
    
        pink_tile = (TILE * ) nextpri;		            // pink_tile is a pointer to primbuf content at adress nextpri, that's cast (type converted) to a TILE struc.   
		
		setTile(pink_tile);                              // initialize the TILE structure ( fill the length and tag(?) value )
		setXY0(pink_tile, CENTERX, CENTERY - 64);   // Set X,Y
		setWH(pink_tile, 64, 64);                        // Set Width, Height
		setRGB0(pink_tile, 255, 32, 255);                // Set color
		addPrim(ot[db], pink_tile);                      // Add primitive to ordering table
            
		nextpri += sizeof(TILE);     
    
        FntPrint("Hello tile !");
    
        FntFlush(-1);
        
        display();
        }
    return 0;
    }
