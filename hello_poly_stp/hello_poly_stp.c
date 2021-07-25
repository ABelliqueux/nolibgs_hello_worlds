// Demo the different settings for pixel and primitive semi-transparency
//
// Schnappy 07-2021
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
#define MARGINX 16               // margins for text display
#define MARGINY 16
#define OTLEN 8                    // Ordering Table Length 
DISPENV disp[2];                   // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
u_long ot[2][OTLEN];               // double ordering table of length 8 * 32 = 256 bits / 32 bytes
char primbuff[2][32768];     // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes
char *nextpri = primbuff[0];       // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]
short db = 0;                      // index of which buffer is used, values 0, 1


// RGB pixels are 16bpp, 5b Red, 5b Green, 5b Blue, 1b STP (semi-transparency)
// See http://psx.arthus.net/sdk/Psy-Q/DOCS/FileFormat47.pdf, p.183
typedef struct RGB_PIX {
    u_int  R:5, G:5, B:5, STP:1;
    } RGB_PIX;
    
// TIM's pixel data
// See http://psx.arthus.net/sdk/Psy-Q/DOCS/FileFormat47.pdf, p.182
typedef struct PIXEL {
    u_long bnum;
    u_short DX, DY;
    u_short W, H;
    RGB_PIX data[]; 
} PIXEL;

// TIM's CLUT section - exists only in 4/8bpp TIMs 
// See See http://psx.arthus.net/sdk/Psy-Q/DOCS/FileFormat47.pdf, p.181
typedef struct CLUT {
    u_long bnum;
    u_short DX, DY;
    u_short W, H;
    u_short clut[]; 
} CLUT;

// 4/8bpp TIM files have CLUT
typedef struct TIM_FILE_CLUT{
    u_long ID;
    u_long flag;
    u_long clut;
    PIXEL pixel[];
} TIM_FILE_CLUT;

// 16/24bpp TIM files have not CLUT member
// See See http://psx.arthus.net/sdk/Psy-Q/DOCS/FileFormat47.pdf, p.179
typedef struct TIM_FILE{
    u_long ID;
    u_long flag;
    PIXEL pixel[];
} TIM_FILE;

// If we were using C++, we could use templates 
//~ struct EmbeddedClut { u_long clut; };
//~ struct NoEmbeddedClut { };
//~ template<has_clut>
//~ struct TIM_FILE {
    //~ u_long ID;
    //~ u_long flag;
    //~ std::conditional<has_clut, EmbeddedClut, NoEmbeddedClut> clut;
    //~ PIXEL pixel[];
//~ };

// 16bpp TIM
// STP set on black pixels ( STP, B, R, G == 1, 0, 0 ,0)
extern TIM_FILE _binary_TIM_stpOnBlack_tim_start;
// STP set on non black pixels ( STP, B, R, G == 1, !0, !0 ,!0)
extern TIM_FILE _binary_TIM_stpOnNonBlack_tim_start;
// STP set on image's alpha channnel ( STP, B, R, G == 1, a, a ,a)
extern TIM_FILE _binary_TIM_stpOnAlphaI_tim_start;
// STP set on 8bpp TIM's CLUT index 0 ( STP, B, R, G == 1, i, i, i)
extern TIM_FILE _binary_TIM_stpOnColIndex_tim_start;
// Store in an array so we can iterate over it
TIM_FILE * timFiles[4];
TIM_IMAGE timImages[4];
// Number of primitives to draw
#define NUM_PRIM 4
// Primitive stp flag : 0 == off, 1 == on
char stpFlag = 0;

void LoadTexture(TIM_FILE * tim, TIM_IMAGE * tparam){     // This part is from Lameguy64's tutorial series : lameguy64.net/svn/pstutorials/chapter1/3-textures.html login/pw: annoyingmous
        OpenTIM( ( u_long * ) tim);                                   // Open the tim binary data, feed it the address of the data in memory
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
    SetGeomOffset( 0 , 0 );
    SetGeomScreen( CENTERX );
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
    setRGB0(&draw[0], 255, 0, 128);
    setRGB0(&draw[1], 255, 0, 128);
    draw[0].isbg = 1;
    draw[1].isbg = 1;
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    FntLoad(960, 0);
    FntOpen(MARGINX, MARGINY, SCREENXRES - MARGINX * 2, SCREENXRES - MARGINY * 2, 0, 512 );
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
    // Populate array with pointers to TIM data
    timFiles[0] = &_binary_TIM_stpOnBlack_tim_start;
    timFiles[1] = &_binary_TIM_stpOnNonBlack_tim_start;
    timFiles[2] = &_binary_TIM_stpOnAlphaI_tim_start;
    timFiles[3] = &_binary_TIM_stpOnColIndex_tim_start;
    // Init Disp/Draw, double buffer, font
    init();
    // Init proto pad
    PadInit(0);
    int pad, oldPad;
    // Pointer to a POLY_G4 
    POLY_FT4 * poly[4] = {0};                       
    SVECTOR VertPos[4] = {                          // Set initial vertices position relative to 0,0 - see here : https://psx.arthus.net/docs/poly_f4.jpg
            {-32, -32, 1 },                         // Vert 1 
            {-32,  32, 1 },                         // Vert 2
            { 32, -32, 1 },                         // Vert 3
            { 32,  32, 1  }                         // Vert 4
        };                                          
    VECTOR  transVector = { SCREENXRES/3, SCREENYRES/4, 128, 0};               // Initialize translation vector {x, y, z, pad}
    SVECTOR rotVector = {0};                        // Initialize rotation vector {x, y, z} 
    
    // Load textures to VRAM
    for (char tim = 0; tim < NUM_PRIM; tim++){
        LoadTexture(timFiles[tim], &timImages[tim]);
    }
    
    while (1)
    {
        MATRIX Work;
        // Clear OT
        ClearOTagR(ot[db], OTLEN);
        // Use a temporary work matrix
        // Set Trans/Rot vectors to work matrix
        RotMatrix(&rotVector, &Work);           // Apply rotation matrix
        TransMatrix(&Work, &transVector);         // Apply translation matrix   
        SetRotMatrix(&Work);                    // Set default rotation matrix
        SetTransMatrix(&Work);                  // Set default transformation matrix
        // Draw NUM_PRIM primitives
        for (int i = 0; i < NUM_PRIM; i++){
            long p, flag;
            // Draw prims with an offset base on iteration number
            transVector.vx = SCREENXRES/NUM_PRIM + (i * (SCREENXRES/NUM_PRIM + 32) ) ;
            transVector.vy = SCREENYRES/NUM_PRIM;
            if ( i >= 2) { 
                transVector.vx = SCREENXRES/NUM_PRIM + ((i - 2) * (SCREENXRES/NUM_PRIM + 32) ) ;
                transVector.vy = SCREENYRES/2 + 24;
            } 
            TransMatrix(&Work, &transVector);         
            SetTransMatrix(&Work);
            // Set poly 
            poly[i] = (POLY_FT4 *)nextpri;                    // Set poly to point to  the address of the next primitiv in the buffer
            setPolyFT4(poly[i]);                              // Initialize poly as a POLY_F4 
            // Get texture page
            poly[i]->tpage = getTPage( timImages[i].mode & 0x3,
                                       0,
                                       // Get Tpage coordinates from the TIM_IMAGE mode and prect members.
                                       timImages[i].prect->x,
                                       timImages[i].prect->y); 
            // If 8/4bpp, get CLUT
            if ( (timImages[i].mode & 0x3) < 2 ) {
                setClut(poly[i],             
                        timImages[i].crect->x,
                        timImages[i].crect->y
                );
            }
            setRGB0(poly[i], 128, 128, 128);                   // Set poly color (neutra here)
            SetSemiTrans(poly[i], stpFlag);
            RotTransPers4(
                        &VertPos[0],      &VertPos[1],      &VertPos[2],      &VertPos[3],
                        (long*)&poly[i]->x0, (long*)&poly[i]->x1, (long*)&poly[i]->x2, (long*)&poly[i]->x3,
                        &p,
                        &flag
                        );                                 // Perform coordinate and perspective transformation for 4 vertices
            setUV4(poly[i], 0, 0, 0, 144, 144, 0, 144, 144);  // Set UV coordinates in order Top Left, Bottom Left, Top Right, Bottom Right 
            // Add poly to the Ordering table
            addPrim(ot[db], poly[i]);                        
            // Increment nextpri address with size of a POLY_F4 struct 
            nextpri += sizeof(POLY_FT4);                    
        }
        
        // Get pad input
        pad = PadRead(0); 
        // If select button is used
        if ( pad & PADselect && !( pad & oldPad ) ){
            // Flip STP flag
            stpFlag = !stpFlag;
            // Set flag to avoir misfire
            oldPad = pad;
        } 
        // Reset flag when button released
        if (!(pad & PADselect)) {
            oldPad = 0;
        }
        FntPrint("Hello semi-transparency  !\nPrim STP (push Select) : %d\n\n\n\n\n\n\n\n\n\n\n\n", stpFlag);                   
        FntPrint("    stp on black    stp on non-black\n\n\n\n\n\n\n\n\n\n\n\n");                   
        FntPrint("  stp on non-black  stp on col index");                   
        FntFlush(-1);
        display();
        }
    return 0;
    }
