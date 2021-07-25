// Hello poly subdiv ! Inline / DMPSX version
//
// Ref :     /psyq/DOCS/Devrefs/Inlinref.pdf, p.18
//           https://psx-spx.consoledev.net/geometrytransformationenginegte/
// PSX                    / Z+
//  screen               / 
//coordinate         +-----X+
//system           / | 
//              eye  | Y+      
//
// Credits, thanks : Nicolas Noble, Sickle, Lameguy64 @ psxdev discord : https://discord.com/invite/N2mmwp
// https://discord.com/channels/642647820683444236/663664210525290507/834831466100949002
// Schnappy 07-2021
#include <sys/types.h>
#include <stdio.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
// OldWorld PsyQ has a inline_c.h file for inline GTE functions. We have to use the one at https://github.com/grumpycoders/pcsx-redux/blob/07f9b02d1dbb68f57a9f5b9773041813c55a4913/src/mips/psyq/include/inline_n.h
// because the real GTE commands are needed in nugget : https://psx-spx.consoledev.net/geometrytransformationenginegte/#gte-coordinate-calculation-commands 
#include <inline_n.h>
//~ #include <gtemac.h> // gtemac contains macro versions of the libgte functions, worth checking out to see the operations order.

#define VMODE 0                         // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320                  // Screen width
#define SCREENYRES 240 + (VMODE << 4)     // Screen height : If VMODE is 0 = 240, if VMODE is 1 = 256 
#define CENTERX ( SCREENXRES >> 1 )       // Center of screen on x 
#define CENTERY ( SCREENYRES >> 1 )       // Center of screen on y
#define MARGINX 0                       // margins for text display
#define MARGINY 32
#define FONTSIZE 8 * 7                  // Text Field Height
#define OTLEN 10                        // Ordering Table Length 

DISPENV disp[2];                        // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
u_long ot[2][OTLEN];               // double ordering table of length 8 * 32 = 256 bits / 32 bytes
char primbuff[2][32768] = {1};     // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes
char *nextpri = primbuff[0];       // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]
short db = 0;                      // index of which buffer is used, values 0, 1

short subdiv = 1;

void init(void)
{
    ResetGraph(0);
    // Initialize and setup the GTE
    InitGeom();
    //~ SetGeomOffset(CENTERX,CENTERY);
    gte_SetGeomOffset(CENTERX,CENTERY);
    gte_SetGeomScreen(CENTERX);
    // Set display environment
    SetDefDispEnv(&disp[0], 0, 0, SCREENXRES, SCREENYRES);
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    // Set draw environment
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[1], 0, 0, SCREENXRES, SCREENYRES);
    // If PAL, use 320x256, hence 256 - 240 = 16 / 2 = 8 px vertical offset
    if (VMODE)
    {
        SetVideoMode(MODE_PAL);
        disp[0].screen.y += 8;
        disp[1].screen.y += 8;
    }
    SetDispMask(1);
    // Set background color
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
    // Wait for drawing
    DrawSync(0);
    // Wait for vsync
    VSync(0);
    // Flip DISP and DRAW env
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    DrawOTag(&ot[db][OTLEN - 1]);
    // Flip db index
    db = !db;
    // Get next primitive in buffer
    nextpri = primbuff[db];
}

int main(void)
{
    long p, flag, OTz;
    SVECTOR rotVector = {0};
    SVECTOR rotVector4 = {0};                  // Initialize rotation vector {x, y, z}
    VECTOR  transVector = {0, 0, CENTERX, 0};         // Initialize translation vector {x, y, z}
    SVECTOR vertPos[4] = {                    // 0 ______ 1
        { -64, -32, 0, 0 },           // Vert 1  |        |
        { 32,  -32, 0, 0 },           // Vert 2  | _______|
        { -32, 48, 0, 0 },              //      2          3
        { 22,  50, 0, 0 }
    };           // Vert 3
    MATRIX workMatrix = {0};                       
    POLY_F4 * poly  = {0};                            // pointer to a POLY_F4 
    POLY_F4 * poly4 = {0};                            // pointer to a POLY_F4 
    POLY_F4 * polySub[4] = {0};                            // pointer to a POLY_F4 
    init();
    while (1)
    {
        // Set Ordering table
        ClearOTagR(ot[db], OTLEN);
        rotVector.vz += 4;
        // Find rotation matrix from vector, store in 
        RotMatrix_gte(&rotVector, &workMatrix);
        // Ditto for translation        
        TransMatrix(&workMatrix, &transVector);
        // Set the matrices we just found
        gte_SetRotMatrix(&workMatrix);
        gte_SetTransMatrix(&workMatrix);
    
        poly4 = (POLY_F4 *)nextpri;
        nextpri += sizeof(POLY_F4);    // increment nextpri address with size of a POLY_F3 struct 

        polySub[0] = (POLY_F4 *)nextpri;
        nextpri += sizeof(POLY_F4);
        polySub[1] = (POLY_F4 *)nextpri;
        nextpri += sizeof(POLY_F4);
        polySub[2] = (POLY_F4 *)nextpri;
        nextpri += sizeof(POLY_F4);
        polySub[3] = (POLY_F4 *)nextpri;
        
        polySub[0]->x0 = poly4->x0;
        polySub[0]->y0 = poly4->y0;
        
        polySub[0]->x1 = (poly4->x0 + poly4->x1) >> 1;
        polySub[0]->y1 = (poly4->y0 + poly4->y1) >> 1;
        
        polySub[0]->x2 = (poly4->x0 + poly4->x2) >> 1;
        polySub[0]->y2 = (poly4->y0 + poly4->y2) >> 1;
        
        polySub[0]->x3 = (poly4->x0 + poly4->x3) >> 1;
        polySub[0]->y3 = (poly4->y0 + poly4->y3) >> 1;

        
        polySub[1]->x0 = polySub[0]->x1;
        polySub[1]->y0 = polySub[0]->y1;
        
        polySub[1]->x1 = poly4->x1;
        polySub[1]->y1 = poly4->y1;
        
        polySub[1]->x2 = polySub[0]->x3;
        polySub[1]->y2 = polySub[0]->y3;
        
        polySub[1]->x3 = (poly4->x1 + poly4->x3) >> 1;
        polySub[1]->y3 = (poly4->y1 + poly4->y3) >> 1;
        
        polySub[2]->x0 = polySub[0]->x2;
        polySub[2]->y0 = polySub[0]->y2;
        
        polySub[2]->x1 = polySub[0]->x3;
        polySub[2]->y1 = polySub[0]->y3;
        
        polySub[2]->x2 = poly4->x2;
        polySub[2]->y2 = poly4->y2;
        
        polySub[2]->x3 = (poly4->x2 + poly4->x3) >> 1;
        polySub[2]->y3 = (poly4->y2 + poly4->y3) >> 1;
        
        polySub[3]->x0 = polySub[0]->x3;
        polySub[3]->y0 = polySub[0]->y3;
        
        polySub[3]->x1 = polySub[1]->x3;
        polySub[3]->y1 = polySub[1]->y3;
        
        polySub[3]->x2 = polySub[2]->x3;
        polySub[3]->y2 = polySub[2]->y3;
        
        polySub[3]->x3 = poly4->x3;
        polySub[3]->y3 = poly4->y3;

        // Draw a Quad

        // Transform 3 first vertices
        gte_ldv3(&vertPos[0], &vertPos[1], &vertPos[2]);
        gte_rtpt();
        gte_stsxy3_f4(poly4);
        // Transform remaining vertex
        gte_ldv0(&vertPos[3]);
        gte_rtps();
        // SXY3 is set with gte_stsxy() or gte_stsxy2() ¯\_(ツ)_/¯
        gte_stsxy(&poly4->x3);
        // Get p, flag and OTz 
        gte_stdp(&p);
        gte_stflg(&flag);
        gte_stszotz(&OTz);
        
        if ( subdiv == 0){
            setPolyF4(poly4);           
            setRGB0(poly4, 0, 255, 255);
            addPrim(ot[db], poly4); 
        } else {
            setPolyF4(polySub[0]);                
            setRGB0(polySub[0], 255, 0, 255);                   
            addPrim(ot[db], polySub[0]);

            setPolyF4(polySub[1]);         
            setRGB0(polySub[1], 128, 0, 255);
            addPrim(ot[db], polySub[1]);
            
            setPolyF4(polySub[2]);                
            setRGB0(polySub[2], 0, 128, 255);                   
            addPrim(ot[db], polySub[2]);
            
            setPolyF4(polySub[3]);                
            setRGB0(polySub[3], 100, 255, 0);                   
            addPrim(ot[db], polySub[3]);
        }
        
        // Display text
        FntPrint("Hello poly subdiv !\n");                   
        FntFlush(-1);
        
        display();
        }
    return 0;
    }
