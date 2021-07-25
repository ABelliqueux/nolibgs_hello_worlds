// hello_pad example
//
// We're using libetc PadInit() and PadRead() that only supports the 16 buttons pad
// but doesn't need the libpad lib. It's fine for prototyping and simple stuff.
// Schnappy 2021
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

#define MARGINX 32               // margins for text display
#define MARGINY 32

#define FONTSIZE 8 * 2           // Text Field Height

#define OTLEN 8                  // Ordering Table Length 

DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];

u_long ot[2][OTLEN];             // double ordering table of length 8 * 32 = 256 bits / 32 bytes

char primbuff[2][32768] = {1};   // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes

char *nextpri = primbuff[0];     // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]

short db = 0;                    // index of which buffer is used, values 0, 1

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
    
    DrawOTag(&ot[db][OTLEN - 1]);
    
    db = !db;
    
    nextpri = primbuff[db];
}


int main(void)
{
    
    TILE * PADL;                    // Tile primitives
    TILE * TRIGGERL;
    
    TILE * PADR;
    TILE * TRIGGERR;
    
    TILE * START, * SELECT;
    
    int pad = 0;                    
    
    init();
    
    PadInit(0);                     // Initialize pad.  Mode is always 0
    
    while (1)
    {
        ClearOTagR(ot[db], OTLEN);
        
        // D-cross
        
        PADL = (TILE *)nextpri;
        
        setTile(PADL);
        setRGB0(PADL, 0, 0, 255);        
        setXY0(PADL, CENTERX - 80, CENTERY);
        setWH(PADL, 24, 24);
            
        
        addPrim(ot[db], PADL);
        
        nextpri += sizeof(TILE);
        
        // L1+L2
        
        TRIGGERL = (TILE *)nextpri;
        
        setTile(TRIGGERL);
        setRGB0(TRIGGERL, 255, 0, 0);        
        setXY0(TRIGGERL, CENTERX - 80, CENTERY - 80);
        setWH(TRIGGERL, 24, 24);
            
        
        addPrim(ot[db], TRIGGERL);
        
        nextpri += sizeof(TILE);
        
        // /\, X, O, [] 
        
        PADR = (TILE *)nextpri;
        
        setTile(PADR);
        setRGB0(PADR, 0, 255, 0);        
        setXY0(PADR, CENTERX + 50, CENTERY);
        setWH(PADR, 24, 24);
        
        addPrim(ot[db], PADR);
        
        nextpri += sizeof(TILE);
        
        // R1+R2
        
        TRIGGERR = (TILE *)nextpri;
        
        setTile(TRIGGERR);
        setRGB0(TRIGGERR, 255, 0, 255);        
        setXY0(TRIGGERR, CENTERX + 50, CENTERY -80);
        setWH(TRIGGERR, 24, 24);
        
        addPrim(ot[db], TRIGGERR);
                
        nextpri += sizeof(TILE);

        // START + SELECT
        
        START = (TILE *)nextpri;
        
        setTile(START);
        setRGB0(START, 240, 240, 240);        
        setXY0(START, CENTERX - 16, CENTERY - 36);
        setWH(START, 24, 24);
        
        addPrim(ot[db], START);
                
        nextpri += sizeof(TILE);

        // Pad stuff

        pad = PadRead(0);                             // Read pads input. id is unused, always 0.
                                                      // PadRead() returns a 32 bit value, where input from pad 1 is stored in the low 2 bytes and input from pad 2 is stored in the high 2 bytes. (https://matiaslavik.wordpress.com/2015/02/13/diving-into-psx-development/)
                                                      
        // D-pad        
        
        if(pad & PADLup)   {PADL->y0 = CENTERY - 16;} // 🡩           // To access pad 2, use ( pad >> 16 & PADLup)...
        if(pad & PADLdown) {PADL->y0 = CENTERY + 16;} // 🡫
        if(pad & PADLright){PADL->x0 = CENTERX - 64;} // 🡪
        if(pad & PADLleft) {PADL->x0 = CENTERX - 96;} // 🡨
        
        // Buttons
        
        if(pad & PADRup)   {PADR->y0 = CENTERY - 16;} //   △
        if(pad & PADRdown) {PADR->y0 = CENTERY + 16;} // ╳
        if(pad & PADRright){PADR->x0 = CENTERX + 66;} //   ⭘
        if(pad & PADRleft) {PADR->x0 = CENTERX + 34;} // ⬜
        
        // Shoulder buttons
        
        if(pad & PADL1){TRIGGERL->y0 = CENTERY - 64;} // L1
        if(pad & PADL2){TRIGGERL->y0 = CENTERY - 96;} // L2
        if(pad & PADR1){TRIGGERR->y0 = CENTERY - 64;} // R1
        if(pad & PADR2){TRIGGERR->y0 = CENTERY - 96;} // R2
        
        // Start & Select
        
        if(pad & PADstart){START->w = 32; START->h = 32;START->x0 -= 4;START->y0 -= 4;} // START
        if(pad & PADselect){START->r0 = 0;}                                             // SELECT
    
        FntPrint("Hello Pad!");
    
        FntFlush(-1);
        
        display();
        }
    return 0;
    }
