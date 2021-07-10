// With help from Nicolas Noble, Jaby smoll Seamonstah
// Based on Lameguy64's tutorial series  : http://lameguy64.net/svn/pstutorials/chapter1/2-graphics.html
//
// From ../psyq/addons/graphics/MESH/RMESH/TUTO0.C :
// 
 /*        PSX screen coordinate system 
 *
 *                           Z+
 *                          /
 *                         /
 *                        +------X+
 *                       /|
 *                      / |
 *                     /  Y+
 *                   eye        */

#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <libapi.h>


#define VMODE 0                 // Video Mode : 0 : NTSC, 1: PAL

#define SCREENXRES 320          // Screen width
#define SCREENYRES 240          // Screen height

#define CENTERX SCREENXRES/2    // Center of screen on x 
#define CENTERY SCREENYRES/2    // Center of screen on y

#define MARGINX 0                // margins for text display
#define MARGINY 32

#define FONTSIZE 8 * 7             // Text Field Height

#define OTLEN 8                    // Ordering Table Length 

DISPENV disp[2];                   // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];

u_long ot[2][OTLEN];               // double ordering table of length 8 * 32 = 256 bits / 32 bytes

char primbuff[2][32768] = {1};     // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes

char *nextpri = primbuff[0];       // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]

short db = 0;                      // index of which buffer is used, values 0, 1


MATRIX identity(int num)           // generate num x num matrix 
{
   int row, col;
   MATRIX matrix;
   
   for (row = 0; row < num; row++)
   {
      for (col = 0; col < num; col++)
      {
         if (row == col)
            matrix.m[row][col] = 4096;
         else
            matrix.m[row][col] = 0;
      }
   }
   return matrix;
}

void init(void)
{
    ResetGraph(0);
    
    // Initialize and setup the GTE
    InitGeom();
    SetGeomOffset(CENTERX,CENTERY);
    SetGeomScreen(CENTERX);
    
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
    
    MATRIX IDMATRIX = identity(3);                  // Generate 3x3 identity matrix
    
    POLY_F4 *poly = {0};                            // pointer to a POLY_F4 
    SVECTOR RotVector = {0, 0, 0};                  // Initialize rotation vector {x, y, z}
    VECTOR  MovVector = {0, 0, CENTERX, 0};         // Initialize translation vector {x, y, z}
    VECTOR  ScaleVector ={ONE, ONE, ONE};           // ONE is define as 4096 in libgte.h
    
    SVECTOR VertPos[4] = {                          // Set initial vertices position relative to 0,0 - see here : https://psx.arthus.net/docs/poly_f4.jpg
            {-32, -32, 1 },                         // Vert 1 
            {-32,  32, 1 },                         // Vert 2
            { 32, -32, 1 },                         // Vert 3
            { 32,  32, 1  }                         // Vert 4
        };                                          
    MATRIX PolyMatrix = IDMATRIX;                   
    
    long polydepth;
    long polyflag;
    long OTz;
    
    init();
    
    while (1)
    {
        ClearOTagR(ot[db], OTLEN);
        
        poly = (POLY_F4 *)nextpri;                    // Set poly to point to  the address of the next primitiv in the buffer
        
        // Set transform matrices for this polygon
                
        RotMatrix(&RotVector, &PolyMatrix);           // Apply rotation matrix
        TransMatrix(&PolyMatrix, &MovVector);
        ScaleMatrix(&PolyMatrix, &ScaleVector);         // Apply translation matrix   
        
        SetRotMatrix(&PolyMatrix);                    // Set default rotation matrix
        SetTransMatrix(&PolyMatrix);                  // Set default transformation matrix
        
        setPolyF4(poly);                              // Initialize poly as a POLY_F4 
        setRGB0(poly, 255, 0, 255);                   // Set poly color

        
        // RotTransPers
        
        //~ OTz = RotTransPers(&VertPos[0], (long*)&poly->x0, &polydepth, &polyflag);
        //~ RotTransPers(&VertPos[1], (long*)&poly->x1, &polydepth, &polyflag);
        //~ RotTransPers(&VertPos[2], (long*)&poly->x2, &polydepth, &polyflag);
        //~ RotTransPers(&VertPos[3], (long*)&poly->x3, &polydepth, &polyflag);
        
        // RotTransPers4 equivalent 
        
        OTz = RotTransPers4(
                    &VertPos[0],      &VertPos[1],      &VertPos[2],      &VertPos[3],
                    (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2, (long*)&poly->x3,
                    &polydepth,
                    &polyflag
                    );                                // Perform coordinate and perspective transformation for 4 vertices
        
        RotVector.vy += 4;
        RotVector.vz += 4;                              // Apply rotation on Z-axis. On PSX, the Z-axis is pointing away from the screen.  

        addPrim(ot[db], poly);                         // add poly to the Ordering table
        
        nextpri += sizeof(POLY_F4);                    // increment nextpri address with size of a POLY_F4 struct 
    
        FntPrint("Hello Poly !");                   
    
        FntFlush(-1);
        
        display();
        }
    return 0;
    }
