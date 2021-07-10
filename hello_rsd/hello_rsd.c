#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <kernel.h>
#include <libgs.h>
#include <libapi.h>
#include <malloc.h>

#define VMODE 0         // Video Mode : 0 : NTSC, 1: PAL

#define SCREENXRES 320
#define SCREENYRES 240

#define CENTERX SCREENXRES/2
#define CENTERY SCREENYRES/2

#define MARGINX 0      // margins for text display
#define MARGINY 32

#define FONTSIZE 8 * 7           // Text Field Height

#define OTLEN 8              // Ordering Table Length 

DISPENV disp[2];             // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];

u_long ot[2][OTLEN];     // double ordering table of length 8 * 32 = 256 bits / 32 bytes

char primbuff[2][32768] = {1};     // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes

char *nextpri = primbuff[0]; // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]

short db = 0;                // index of which buffer is used, values 0, 1

//~ short BgColor[3] = {0,0,0}; 

void init(void)
{
    ResetGraph(0);
    
    GsInitGraph(320, 240, 0, 0, 0);
    
    // Initialize and setup the GTE : Not needed ?
	
    InitGeom();
	SetGeomOffset(0,0);
	SetGeomScreen(1);
    
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
    
    POLY_F4 *poly = {0};
    SVECTOR RotVector = {0, 0, 0};
    VECTOR  MovVector = {CENTERX, CENTERY, 0};

    SVECTOR VertPos[4] = {
            {-32, -32, 1},
            {-32, 32, 1 },
            {32, -32, 1 },
            {32, 32, 1  }
        };
    MATRIX PolyMatrix = GsIDMATRIX;    
    long polydepth;
    long polyflag;
    
    init();
    
    while (1)
    {
        ClearOTagR(ot[db], OTLEN);
        
        poly = (POLY_F4 *)nextpri;
        
        // Set transform matrices for this polygon
                
        RotMatrix(&RotVector, &PolyMatrix);
        TransMatrix(&PolyMatrix, &MovVector);
        
        SetRotMatrix(&PolyMatrix);
        SetTransMatrix(&PolyMatrix);
        
        setPolyF4(poly);
        setRGB0(poly, 255, 0, 255);        
        //~ setXY4(poly, 0, 0, 0, 32, 32, 0, 32, 32);
        RotTransPers4(
                    &VertPos[0],      &VertPos[1],      &VertPos[2],      &VertPos[3],
                    (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2, (long*)&poly->x3,
                    &polydepth,
                    &polyflag
                    );
        
        
        RotVector.vz+=16;

        
        addPrim(ot[db], poly);
        
        nextpri += sizeof(POLY_F4);
    
        FntPrint("Hello world !");
    
        FntFlush(-1);
        
        display();
        }
    return 0;
    }
