// Hello free cycles !
//
// Ref :     /psyq/DOCS/Devrefs/Inlinref.pdf, p.18
//           /psyq/psx/sample/scea/GTE
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
// RAM -> CPU and CPU -> GTE macros : 
#include "../includes/CPUMAC.H"

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

// DCache setup
#define  dc_camdirp ((sshort*)  getScratchAddr(0))
#define  dc_ip      ((uchar*)   getScratchAddr(1))
#define  dc_opzp    ((slong*)   getScratchAddr(2))
#define  dc_wmatp   ((MATRIX*)  getScratchAddr(3))
#define  dc_cmatp   ((MATRIX*)  getScratchAddr(9))
#define  dc_sxytbl  ((DVECTOR*) getScratchAddr(15))

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
    VSync(1);
    // Flip DISP and DRAW env
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    //~ SetDispMask(1);
    DrawOTag(ot[db] + OTLEN - 1);
    // Flip db index
    db = !db;
    // Get next primitive in buffer
    nextpri = primbuff[db];
}

int main(void)
{
    long p, flag, OTz;
    SVECTOR rotVector = {0};                    
    SVECTOR rotVector4 = {0};                  // Initialize rotation vector {x, y, z} - ALWAYS !
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
    
    // Declare registers
    register ulong   ur0     asm("$16");
    register ulong   ur1     asm("$17");
    register ulong   ur2     asm("$18");
    register ulong   ur3     asm("$19");
    register ulong   ur4     asm("$20");
    register ulong   ur5     asm("$21");
    
    while (1)
    {
        // Set Ordering table
        ClearOTagR(ot[db], OTLEN);
        // Cast next primitives in buffer as a POLY_F3 and a POLY_F4 (see display() )
        poly = (POLY_F3 *)nextpri;  
        nextpri += sizeof(POLY_F3);
        poly4 = (POLY_F4 *)nextpri;    
        // Set matrices - Move to left of screen
        // Draw on the left part of the screen
        transVector.vx = -CENTERX/2;
        // Increment rotation angle on Y axis
        rotVector.vy += 1;
        // Find rotation matrix from vector, store in 
        RotMatrix_gte(&rotVector, &workMatrix);
        // Ditto for translation        
        TransMatrix(&workMatrix, &transVector);
        // Set the matrices we just found
        gte_SetRotMatrix(&workMatrix);
        gte_SetTransMatrix(&workMatrix);
        // Draw a Tri and a Quad
        // Copy Tri vertices from ram to cpu registers casting as ulong so that ur0 (len 32bits) contains vx and vy (2 * 8bits) 
        // Hence the use of vx, vz members
        cpu_ldr(ur0,(ulong*)&vertPos[0].vx); // Put vx, vy value in ur0
        cpu_ldr(ur1,(ulong*)&vertPos[0].vz); // Put vz, pad value in ur1
        cpu_ldr(ur2,(ulong*)&vertPos[1].vx);
        cpu_ldr(ur3,(ulong*)&vertPos[1].vz);
        cpu_ldr(ur4,(ulong*)&vertPos[2].vx);
        cpu_ldr(ur5,(ulong*)&vertPos[2].vz);
        // Load the gte registers from the cpu registers (gte-cpu move 1 cycle) - mtc2 %0, $0;
        cpu_gted0(ur0);
        cpu_gted1(ur1);
        cpu_gted2(ur2);
        cpu_gted3(ur3);
        cpu_gted4(ur4);
        cpu_gted5(ur5);
        // Tri RotTransPers3
        // The two last cpu->gte copy will happen during the 2 nops in gte_rtpt() 
        gte_rtpt();
        // Fill the cpu registers with the Quad vertices 
        cpu_ldr(ur0,(ulong*)&vertPos[0].vx);
        cpu_ldr(ur1,(ulong*)&vertPos[0].vz);
        cpu_ldr(ur2,(ulong*)&vertPos[1].vx);
        cpu_ldr(ur3,(ulong*)&vertPos[1].vz);
        cpu_ldr(ur4,(ulong*)&vertPos[2].vx);
        cpu_ldr(ur5,(ulong*)&vertPos[2].vz);
        // Get nclip value, and win two cycles
        gte_nclip();
         // Copy Tri 's screen coordinates from gte registers to d-cache.
        gte_stsxy3c(&dc_sxytbl[0]);
        // Set matrices - Move to right of screen
        transVector.vx = CENTERX/2;
        // Increment rot on X/Y axis
        rotVector4.vy -= 1 ;
        rotVector4.vx -= 1 ;
        // Set matrices
        RotMatrix_gte(&rotVector4, &workMatrix);
        TransMatrix(&workMatrix, &transVector);
        gte_SetRotMatrix(&workMatrix);
        gte_SetTransMatrix(&workMatrix);
        // Load the gte registers from the cpu registers (gte-cpu move 1 cycle) - mtc2 %0, $0;
        cpu_gted0(ur0);
        cpu_gted1(ur1);
        cpu_gted2(ur2);
        cpu_gted3(ur3);
        cpu_gted4(ur4);
        cpu_gted5(ur5);
        // Quad RotTransPers3
        // Getting 2 cycles back thanks to nops
        gte_rtpt();
        // gte_nclip() has 2 nops, lets use them to load the remaining vertex data from ram->cpu register
        cpu_ldr(ur0,(ulong*)&vertPos[3].vx);
        cpu_ldr(ur1,(ulong*)&vertPos[3].vz);
        // Calculate nclip (outer product)
        gte_nclip();
        // Copy result to d-cache + 3
        gte_stsxy3c(&dc_sxytbl[3]);
        // Copy from cpu-gte
        cpu_gted0(ur0);
        cpu_gted1(ur1);
        // Quad last vertex RotTransPers
        // These two last cpu->gte load are free :p
        gte_rtps();
        gte_nclip();
        // Copy last vertex value to d-cache
        gte_stsxy(&dc_sxytbl[6]);
        // Get p, flag, OTz
        gte_stdp(&p);
        gte_stflg(&flag);
        gte_stszotz(&OTz);
        // That's 10 cycles we won back ?
        // Copy vertices data from d-cache to ram 
        // Tri 
        *(unsigned long long*)&poly->x0 = *(unsigned long long*)&dc_sxytbl[0];
        *(ulong*)&poly->x2 = *(ulong*)&dc_sxytbl[2];
        // Quad
        *(unsigned long long*)&poly4->x0 = *(unsigned long long*)&dc_sxytbl[3];
        *(unsigned long long*)&poly4->x2 = *(unsigned long long*)&dc_sxytbl[5];
        // Initialize polygons
        setPolyF3(poly);                
        setRGB0(poly, 255, 0, 255);                   
        setPolyF4(poly4);                
        setRGB0(poly4, 0, 255, 255);                   
        // Add to OT
        addPrim(ot[db], poly);
        addPrim(ot[db], poly4);
        // Display text
        FntPrint("Hello Free cycles !\n");                            
        FntFlush(-1);
        
        display();
        }
    return 0;
    }
