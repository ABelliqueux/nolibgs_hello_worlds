// Demo the different settings and rates for pixel and primitive semi-transparency on a cube
// Controls :
//             SELECT : Switch semi-transparency on/off on primitives
//             START : Cycle semi-transparency rates on/off on primitives
// Schnappy 07-2021
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libapi.h>
//~ #include <stdio.h>
// Sample vector model
#include "cubetex.c"
#define VMODE       0
// Number of primitives to draw
#define NUM_PRIM 3
#define SCREENXRES 320
#define SCREENYRES 240
#define CENTERX     SCREENXRES/2
#define CENTERY     SCREENYRES/2
#define MARGINX 16               // margins for text display
#define MARGINY 16
#define OTLEN       2048        // Maximum number of OT entries
#define PRIMBUFFLEN 32768       // Maximum number of POLY_GT3 primitives
// Display and draw environments, double buffered
DISPENV disp[2];
DRAWENV draw[2];
u_long ot[2][OTLEN];                   // Ordering table (contains addresses to primitives)
char primbuff[2][PRIMBUFFLEN]; // Primitive list // That's our prim buffer
char * nextpri = primbuff[0];                       // Primitive counter
short db  = 0;                        // Current buffer counter
// Store TIM files in an array so we can iterate over them - see 'cubetex.c' for TIM_FILE struct and declaration
TIM_FILE * timFiles[3];
TIM_IMAGE timImages[3];
// Prototypes
void init(void);
void display(void);
void LoadTexture(TIM_FILE * tim, TIM_IMAGE * tparam);
void init(){
    // Reset the GPU before doing anything and the controller
    PadInit(0);
    ResetGraph(0);
    // Initialize and setup the GTE
    InitGeom();
    SetGeomOffset(CENTERX, CENTERY);        // x, y offset
    SetGeomScreen(CENTERX);                 // Distance between eye and screen  
    // Set the display and draw environments
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[1], 0, 0, SCREENXRES, SCREENYRES);
    if (VMODE)
    {
        SetVideoMode(MODE_PAL);
        disp[0].screen.y += 8;
        disp[1].screen.y += 8;
    }
    SetDispMask(1);
    setRGB0(&draw[0], 0, 0, 255);
    setRGB0(&draw[1], 0, 0, 255);
    draw[0].isbg = 1;
    draw[1].isbg = 1;
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    // Init font system
    FntLoad(960, 0);
    FntOpen(MARGINX, MARGINY, SCREENXRES - MARGINX * 2, SCREENXRES - MARGINY * 2, 0, 512 );
}
void display(void){
    DrawSync(0);
    VSync(0);
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    DrawOTag(&ot[db][OTLEN - 1]);
    db = !db;
    nextpri = primbuff[db];
    }
void LoadTexture(TIM_FILE * tim, TIM_IMAGE * tparam){     // This part is from Lameguy64's tutorial series : lameguy64.net/svn/pstutorials/chapter1/3-textures.html login/pw: annoyingmous
        OpenTIM((u_long*)tim);                                   // Open the tim binary data, feed it the address of the data in memory
        ReadTIM(tparam);                                // This read the header of the TIM data and sets the corresponding members of the TIM_IMAGE structure
        LoadImage(tparam->prect, tparam->paddr);        // Transfer the data from memory to VRAM at position prect.x, prect.y
        DrawSync(0);                                    // Wait for the drawing to end
        if (tparam->mode & 0x8){ // check 4th bit       // If 4th bit == 1, TIM has a CLUT
            LoadImage(tparam->crect, tparam->caddr);    // Load it to VRAM at position crect.x, crect.y
            DrawSync(0);                                // Wait for drawing to end
    }
}
int main() {
    // Populate array with pointers to TIM data
    timFiles[0] = &_binary_TIM_stpOnBlack_tim_start;
    timFiles[1] = &_binary_TIM_stpOnNonBlack_tim_start;
    timFiles[2] = &_binary_TIM_stpOn8bpp_tim_start;
    // Pad values
    int pad, oldPad;
    // Set semi-transparency on (1) and off (0)
    int stpFlag = 0;
    // Set primitive semi-transparency rate - See LibOver47.pdf, p.107
    int stpRate = 0;
    // If set, rotate cube
    int rotateCube = 1;
    // Array of pointers to a POLY_G4 we iterate over
    POLY_GT3 * poly[NUM_PRIM];                           
     // Rotation vector
    SVECTOR rotVector={ 496, 0, 0, 0 };
    // Translation vector
    VECTOR transVector= { -SCREENXRES/NUM_PRIM, -SCREENYRES/NUM_PRIM, SCREENXRES, 0};
    // Init Disp/Drawenv, Font, etc.
    init();
    // Load textures to VRAM
    for (char tim = 0; tim < NUM_PRIM; tim++){
        LoadTexture(timFiles[tim], &timImages[tim]);
    }
    // Main loop
    while (1) {
        // Work matrix
        MATRIX  Work= {0} ;
        // Triangle counters array - one for each cube
        long curTriangle[3] = {0,0,0}; 
        // Clear the current OT
        ClearOTagR(ot[db], OTLEN);
        // Rotate cube
        if(rotateCube) rotVector.vy += 10;
        // Apply Transl, Rot, then matrix
        RotMatrix(&rotVector, &Work);
        TransMatrix(&Work, &transVector);
        SetRotMatrix(&Work);
        SetTransMatrix(&Work);
        
        // Draw NUM_PRIM primitives
        for (int prim = 0; prim < NUM_PRIM; prim++){
            // Draw prims with an offset based on iteration number
            // A bit messy but the cubes are drawn where we want them :)
            transVector.vx = ( SCREENXRES/NUM_PRIM + ( prim * (SCREENXRES/NUM_PRIM + 48) )) - SCREENXRES + 56 ;
            transVector.vy = SCREENYRES/NUM_PRIM - SCREENYRES + 86;
            // Add vertical offset to second cube
            if ( prim == 1) { 
                transVector.vy = SCREENYRES/NUM_PRIM + 48;
            } 
            // Apply transl matrix
            TransMatrix(&Work, &transVector);         
            SetTransMatrix(&Work);
            
            // modelCube is a TMESH, len member == # vertices, but here it's # of triangle... So, for each tri * 3 vertices ...
            for (int i = 0; i < (modelCube.len * 3); i += 3) {
                // t == vertex count, p == depth cueing interpolation value, OTz ==  value to create Z-ordered OT, Flag == see LibOver47.pdf, p.143 
                long p, OTz, Flag;
                // cast nextpri as POLY_GT3
                poly[prim] = (POLY_GT3 *)nextpri;
                // Initialize the primitive and set its color values
                SetPolyGT3(poly[prim]);
                // Get TPAGE
                poly[prim]->tpage = getTPage( timImages[prim].mode&0x3, stpRate,
                                              timImages[prim].prect->x,
                                              timImages[prim].prect->y
                                              );
                // Set RGB colors for each vertex
                setRGB0(poly[prim] , modelCube.c[i].r , modelCube.c[i].g   , modelCube.c[i].b);
                setRGB1(poly[prim], modelCube.c[i+2].r, modelCube.c[i+2].g, modelCube.c[i+2].b);
                setRGB2(poly[prim], modelCube.c[i+1].r, modelCube.c[i+1].g, modelCube.c[i+1].b);
                // If 8/4bpp, load CLUT to vram
                if ( (timImages[prim].mode & 0x3) < 2 ) {
                    setClut( poly[prim],          
                             timImages[prim].crect->x,
                             timImages[prim].crect->y
                    );
                }
                // Set stpFlag
                SetSemiTrans(poly[prim], stpFlag);
                // Set UV coordinates
                setUV3(poly[prim], modelCube.u[i].vx, modelCube.u[i].vy,
                                   modelCube.u[i+2].vx, modelCube.u[i+2].vy,
                                   modelCube.u[i+1].vx, modelCube.u[i+1].vy);
                // Rotate, translate, and project the vectors and output the results into a primitive
                // curTriangle, +1, +2 point to the vertices index of the triangle we're drawing.
                OTz  = RotTransPers(&modelCube_mesh[ modelCube_index[ curTriangle[prim] ] ]  , ( long * ) &poly[prim]->x0, &p, &Flag);
                OTz += RotTransPers(&modelCube_mesh[ modelCube_index[ curTriangle[prim] + 2] ], ( long*) &poly[prim]->x1, &p, &Flag);
                OTz += RotTransPers(&modelCube_mesh[ modelCube_index[ curTriangle[prim] + 1] ], ( long * ) &poly[prim]->x2, &p, &Flag);
                // Average OTz value for 3 vertices
                // OTz is 1/4 of screen to vertex length
                OTz /= 3;
                // If OTz is in range (not to close) 
                if ((OTz > 0) && (OTz < OTLEN))
                    // Add to ordering table, at index OTz-2 
                    AddPrim(&ot[ db ][ OTz-2 ], poly[prim]);
                // Increment next primitive address
                nextpri += sizeof(POLY_GT3);
                // Increment to next triangle
                curTriangle[prim] += 3;
            }
        }
        // Get pad input
        pad = PadRead(0); 
        // If select button is used
        if ( pad & PADselect && !(oldPad & PADselect) ){
            // Flip STP flag
            stpFlag = !stpFlag;
            // Set flag to avoir misfire
            oldPad = pad;
        } 
        // Reset flag when button released
        if ( !(pad & PADselect) && oldPad & PADselect) {
            oldPad = pad;
        }
        // If start button is used
        if ( pad & PADstart && !( oldPad & PADstart ) ){
            // Switch STP rates 
            stpRate > 2 ? stpRate = 0 : stpRate++;
            // Set flag to avoir misfire
            oldPad = pad;
        } 
        // Reset flag when button released
        if (!(pad & PADstart) && oldPad & PADstart) {
            oldPad = pad;
        }
        FntPrint("Hello semi-transparency  !\nPrim STP (push Select) : %d\nSTP rate (push start): %d\n\n\n\n\n\n\n\n\n\n\n\n", stpFlag, stpRate);                   
        FntPrint("    stp on black    stp on col index\n\n\n\n\n\n\n\n\n\n\n\n");                   
        FntPrint("          stp on non-black");   
        FntFlush(-1);
        display();
    }
    return 0;
}
