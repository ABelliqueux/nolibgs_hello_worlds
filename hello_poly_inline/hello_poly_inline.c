// Hello poly ! Inline / DMPSX version
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
    SVECTOR rotVector, rotVector4 = {0};                  // Initialize rotation vector {x, y, z}
    VECTOR  transVector = {0, 0, CENTERX, 0};         // Initialize translation vector {x, y, z}
    SVECTOR vertPos[4] = {
        { 0, -32, 0, 0 },           // Vert 1 
        { 32,  0, 0, 0 },           // Vert 2
        { -32, 0, 0, 0 },
        { 0,  32, 0, 0 }
    };           // Vert 3
    MATRIX workMatrix = {0};                       
    POLY_F3 * poly  = {0};                            // pointer to a POLY_F4 
    POLY_F4 * poly4 = {0};                            // pointer to a POLY_F4 
    init();
    while (1)
    {
        // Set Ordering table
        ClearOTagR(ot[db], OTLEN);
        
        
        // Draw on the left part of the screen
        transVector.vx = -CENTERX/2;
        // Increment rotation angle on Y axis
        rotVector.vy += 8;
        rotVector.vx -= 4 ;
        // Find rotation matrix from vector, store in 
        RotMatrix_gte(&rotVector, &workMatrix);
        // Ditto for translation        
        TransMatrix(&workMatrix, &transVector);
        // Set the matrices we just found
        gte_SetRotMatrix(&workMatrix);
        gte_SetTransMatrix(&workMatrix);
        
        // Cast next primitive in buffer as a POLY_F4 (see display() )
        poly = (POLY_F3 *)nextpri;        
        
        // Draw a Tri
        
        // Initialize poly as a POLY_F3
        setPolyF3(poly);                
        // Set poly color - Hot pink
        setRGB0(poly, 255, 0, 255);                   
        // Store vertex positions for current polygon in registers v0,v1,v2 
        // Can be replaced by one gte_ldv3 call :
        // gte_ldv3(&vertPos[0], &vertPos[1], &vertPos[2]);
        gte_ldv0(&vertPos[0]);
        gte_ldv1(&vertPos[1]);
        gte_ldv2(&vertPos[2]);
        
        // RotTransPers3 : Perform coordinate and perspective transformation for three vertices.
        // Use gte_rtps() for one vertex.
        gte_rtpt();
        // Get screen coordinates from cop2 registers XY0,XY1,XY2 and store them in primitive's x0, y0, x1, y1, x2, y2 members.
        // Can be replace with one gte_stsxy3() call :
        // gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
        // Can also be replaced with a primitive type dependant version :
        // gte_stsxy3_f3(poly);
        gte_stsxy0(&poly->x0);
        gte_stsxy1(&poly->x1);
        gte_stsxy2(&poly->x2);
        // Get depth interpolation coefficient p
        gte_stdp(&p);
        // Get the flag - see libover47.pdf, p.143 for details on ppossible values
        gte_stflg(&flag);
        // Get screen coordinate Z/4  
        gte_stszotz(&OTz);
        // GTE macro version - needs 'gtemac.h' to be included - uncomment l.21
        //~ gte_RotTransPers3( &VertPos[0], &VertPos[1], &VertPos[2],
                           //~ &poly->x0, &poly->x1, &poly->x2,
                           //~ &p, &flag, &OTz );
        // add poly to the Ordering table
        addPrim(ot[db], poly);
        // increment nextpri address with size of a POLY_F3 struct 
        nextpri += sizeof(POLY_F3);
        
        // Draw a Quad
        //
        // The GTE rtpt can only transform 3 vertices at a time, so we have to do all operations as 3 + 1.
        
        // Move to right of screen
        transVector.vx = CENTERX/2;
        // Increment rot on X/Y axis
        rotVector4.vy -= 8 ;
        rotVector4.vx -= 4 ;
        // Set matrices
        RotMatrix_gte(&rotVector4, &workMatrix);
        TransMatrix(&workMatrix, &transVector);
        gte_SetRotMatrix(&workMatrix);
        gte_SetTransMatrix(&workMatrix);
        
        // Cast a POLY_F4 at the address we just incremented.
        poly4 = (POLY_F4 *)nextpri;
        
        // Initialize poly as a POLY_F4
        setPolyF4(poly4);           
        // Set Poly color - Blue                    
        setRGB0(poly4, 0, 255, 255);
        
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
        
        addPrim(ot[db], poly4);         // add poly to the Ordering table
        nextpri += sizeof(POLY_F4);    // increment nextpri address with size of a POLY_F3 struct 

        // Display text
        FntPrint("Hello Inline GTE !\n");                   
        FntFlush(-1);
        
        display();
        }
    return 0;
    }
