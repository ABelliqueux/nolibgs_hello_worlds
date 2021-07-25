// Draw a textured poly primitive with gouraud_shading, using a repeating texture pattern
//
// With help from Nicolas Noble, Jaby smoll Seamonstah, Lameguy64
// 
// From ../psyq/addons/graphics/MESH/RMESH/TUTO0.C :
// Schnappy 2021


#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>

#define VMODE 0                 // Video Mode : 0 : NTSC, 1: PAL

#define SCREENXRES 320          // Screen width
#define SCREENYRES 240          // Screen height

#define CENTERX SCREENXRES/2    // Center of screen on x 
#define CENTERY SCREENYRES/2    // Center of screen on y

#define MARGINX 32               // margins for text display
#define MARGINY 32

#define FONTSIZE 8 * 5             // Text Field Height

#define OTLEN 8                    // Ordering Table Length 

DISPENV disp[2];                   // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];

u_long ot[2][OTLEN];               // double ordering table of length 8 * 32 = 256 bits / 32 bytes

char primbuff[2][32768];     // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes

char *nextpri = primbuff[0];       // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]

short db = 0;                      // index of which buffer is used, values 0, 1

// 16bpp TIM
extern unsigned long _binary____TIM_bousai_tim_start[];
extern unsigned long _binary____TIM_bousai_tim_end[];
extern unsigned long _binary____TIM_bousai_tim_length;

TIM_IMAGE bousai;


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

void LoadTexture(u_long * tim, TIM_IMAGE * tparam){     // This part is from Lameguy64's tutorial series : lameguy64.net/svn/pstutorials/chapter1/3-textures.html login/pw: annoyingmous
        OpenTIM(tim);                                   // Open the tim binary data, feed it the address of the data in memory
        ReadTIM(tparam);                                // This read the header of the TIM data and sets the corresponding members of the TIM_IMAGE structure
        
        LoadImage(tparam->prect, tparam->paddr);        // Transfer the data from memory to VRAM at position prect.x, prect.y
        DrawSync(0);                                    // Wait for the drawing to end
        
        if (tparam->mode & 0x8){ // check 4th bit       // If 4th bit == 1, TIM has a CLUT
            LoadImage(tparam->crect, tparam->caddr);    // Load it to VRAM at position crect.x, crect.y
            DrawSync(0);                                // Wait for drawing to end
    }
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
    SetDispMask(1);                 // Display on screen    

    setRGB0(&draw[0], 128, 128, 128);
    setRGB0(&draw[1], 128, 128, 128);
    
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
    int i;
    
    MATRIX IDMATRIX = identity(3);                  // Generate 3x3 identity matrix
    
    POLY_GT4 *poly = {0};                           // pointer to a POLY_G4 
    SVECTOR RotVector = {0, 0, 0};                  // Initialize rotation vector {x, y, z}
    VECTOR  MovVector = {0, 0, 120, 0};                   // Initialize translation vector {x, y, z}
                                                    
    SVECTOR VertPos[4] = {                          // Set initial vertices position relative to 0,0 - see here : https://psx.arthus.net/docs/poly_f4.jpg
            {-32, -32, 0 },                         // Vert 1 
            {-32,  32, 0 },                         // Vert 2
            { 32, -32, 0 },                         // Vert 3
            { 32,  32, 0 }                          // Vert 4
        };                                          
    MATRIX PolyMatrix = IDMATRIX;                   
    
    DR_TPAGE * bousai_tpage;
    
    long polydepth;
    long polyflag;
    
    // Texture window
    
    DR_MODE * dr_mode;                              // Pointer to dr_mode prim
    
    RECT tws = {64, 32, 32, 32};                    // Texture window coordinates : x, y, w, h
                                                    // See libref47.pdf, p.242, 7-6, table 7-2 for possible values
    init();
    
    LoadTexture(_binary____TIM_bousai_tim_start, &bousai);
    
    while (1)
    {
        ClearOTagR(ot[db], OTLEN);


        poly = (POLY_GT4 *)nextpri;                    // Set poly to point to  the address of the next primitiv in the buffer
        
        // Set transform matrices for this polygon
                
        RotMatrix(&RotVector, &PolyMatrix);           // Apply rotation matrix
        TransMatrix(&PolyMatrix, &MovVector);         // Apply translation matrix   
        
        SetRotMatrix(&PolyMatrix);                    // Set default rotation matrix
        SetTransMatrix(&PolyMatrix);                  // Set default transformation matrix
        
        setPolyGT4(poly);                                      // Initialize poly as a POLY_F4 
        poly->tpage = getTPage(bousai.mode&0x3, 0, bousai.prect->x, bousai.prect->y);

        setRGB0(poly, 128, 128, 128);                       // Set vertice 1 color
        setRGB1(poly, 255, 0, 0);                           // Set vertice 2 color
        setRGB2(poly, 0, 255, 0);                           // Set vertice 3 color
        setRGB3(poly, 0, 0, 255);                           // Set vertice 4 color
                
        RotTransPers4(
                    &VertPos[0],      &VertPos[1],      &VertPos[2],      &VertPos[3],
                    (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2, (long*)&poly->x3,
                    &polydepth,
                    &polyflag
                    );                                // Perform coordinate and perspective transformation for 4 vertices
        
        setUV4(poly, 0, 0, 0, 144, 144, 0, 144, 144);  // Set UV coordinates in order Top Left, Bottom Left, Top Right, Bottom Right 
        
        RotVector.vy += 14;                              // Apply rotation on Z-axis. On PSX, the Z-axis is pointing away from the screen. 

        addPrim(ot[db], poly);                         // add poly to the Ordering table

        nextpri += sizeof(POLY_GT4);                    // increment nextpri address with size of a POLY_GT4 struct 
             
        // drawing mode primitive
           
        dr_mode = (DR_MODE *)nextpri;                   // initialize drawing mode primitive
        
        setDrawMode(dr_mode, 1, 0, getTPage(bousai.mode&0x3, 0, bousai.prect->x, bousai.prect->y), &tws);  //set texture window
        
        addPrim(ot[db], dr_mode);                      

        nextpri += sizeof(DR_MODE);                    // increment nextpri address with size of a  DR_MODE struct 

        
    
        FntPrint("Hello textured shaded !");                   
            
        FntFlush(-1);
        
        display();
        }
    return 0;
    }
