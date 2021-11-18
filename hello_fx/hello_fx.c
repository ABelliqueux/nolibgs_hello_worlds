// Controls :
//             SELECT : Switch semi-transparency on/off on primitives
//             START : Cycle semi-transparency rates on/off on primitives
//             LEFT/RIGHT: Move forward cube
//             X : Reset Cube position
// Schnappy 11-2021
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <libapi.h>
#include <inline_n.h>
#include <gtemac.h>
// Sample vector model
#include "cubetex.c"
#define VMODE       0
// Number of primitives to draw
#define NUM_PRIM 2
// Number of textures to load
#define NUM_TEX 3
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
// Get included tim files address
extern TIM_FILE _binary_TIM_cube_tim_start;
extern TIM_FILE _binary_TIM_sky_tim_start;
extern TIM_FILE _binary_TIM_bg_tim_start;
// Light
CVECTOR BGc = {130, 200, 255, 0};  
// Back color  
VECTOR  BKc = {128, 128, 128, 0};
// Light rotation angle
SVECTOR lgtang = {0, 0, 0}; 
// These will be used to store the light rotation matrix, cube rotation matrix, and composite light matrix.
MATRIX  rotlgt, rotcube, light;
// Local Light Matrix : Direction and reach of each light source. 
MATRIX lgtmat = {
//      X      Y      Z
        0,  -ONE,   0, // Lightsource 1 : here, the light source is at the Bottom-Left of the screen, and points into the screen.
        0,     0,     0, // Lightsource 2
        0,     0,     0, // Lightsource 3
    };
// Local Color Matrix
MATRIX cmat = {
//   L1    L2   L3
    4096,   0,   0, // R
    4096,   0,   0, // G
    4096,   0,   0  // B
    };

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
    // Set far color
    SetFarColor( BGc.r, BGc.g, BGc.b );
    // Set Ambient color
    SetBackColor( BKc.vx, BKc.vy, BKc.vz );
    // Set Color matrix
    SetColorMatrix(&cmat);
    // Set Fog settings
    SetFogNearFar( 128, 1024, CENTERX );
    setRGB0(&draw[0], 0, 0, 0);
    setRGB0(&draw[1], 0, 0, 0);
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
    timFiles[0] = &_binary_TIM_cube_tim_start;
    timFiles[1] = &_binary_TIM_sky_tim_start;
    timFiles[2] = &_binary_TIM_bg_tim_start;
    // Pad values
    int pad, oldPad;
    // Set semi-transparency on (1) and off (0)
    int stpFlag = 1;
    // Set primitive semi-transparency rate - See LibOver47.pdf, p.107
    int stpRate = 0;
    // If set, rotate cube
    int rotateCube = 1;
    int offsetCube = 0;
    // Array of pointers to a POLY_G4 we iterate over
    POLY_GT3 * poly[NUM_PRIM];                           
     // Rotation vector
    SVECTOR rotVector={ 384, 0, 128, 0 };
    // Translation vector
    VECTOR transVector= { 0, 0, 256, 0};
    // BG sprt
    POLY_FT4 * bg;
    // Normalized UV coordinates for the X axis
    long normH = ((255 << 12) / SCREENXRES);
    
    // Init Disp/Drawenv, Font, etc.
    init();
    // Load textures to VRAM
    for (char tex = 0; tex < NUM_TEX; tex++){
        LoadTexture(timFiles[tex], &timImages[tex]);
    }
    
    // Main loop
    while (1) {
        // Work matrix
        MATRIX  Work= {0} ;
        // Triangle counters array - one for each cube
        long curTriangle[3] = {0,0,0}; 
        // Clear the current OT
        ClearOTagR(ot[db], OTLEN);
        // Draw BG
        bg = (POLY_FT4 * )nextpri;
        SetPolyFT4(bg);
        bg->tpage = getTPage( timImages[2].mode&0x3, 0,
                                    timImages[2].prect->x,
                                    timImages[2].prect->y
                                   );
        if ( (timImages[2].mode & 0x3) < 2 ) {
            setClut( bg,          
                     timImages[2].crect->x,
                     timImages[2].crect->y
            );
        }
        setRGB0(bg, 127,127,127);
        setUV4(bg, 0, 0, 
                   SCREENYRES, 0,
                   0, SCREENYRES,
                   SCREENYRES, SCREENYRES
              );
        setXY4(bg, 0 , 0,
                   SCREENXRES, 0,
                   0 , SCREENYRES,
                   SCREENXRES, SCREENYRES);
        addPrim(ot[db][OTLEN-1], bg);
        nextpri += sizeof(POLY_FT4);        
        // Rotate cube
        if(rotateCube) rotVector.vy += 10;
        // Find and apply light rotation matrix
        // Find rotmat from light angles
        RotMatrix_gte(&lgtang, &rotlgt);
        // Find rotmat from cube angles
        RotMatrix_gte(&rotVector, &rotcube);  
        // RotMatrix cube * RotMatrix light
        MulMatrix0(&rotcube, &rotlgt, &rotlgt);
        // Light Matrix * RotMatrix light 
        MulMatrix0(&lgtmat, &rotlgt, &light);
        // Set new light matrix 
        SetLightMatrix(&light);
        // Apply Transl, Rot, then matrix
        RotMatrix(&rotVector, &Work);
        TransMatrix(&Work, &transVector);
        SetRotMatrix(&Work);
        SetTransMatrix(&Work);
        long p, OTz, Flag;
        // Draw NUM_PRIM primitives
        for (int i = 0; i < (modelCube.len * 3); i += 3) {
            // Set projection matrices
            transVector.vx = 0;
            TransMatrix(&Work, &transVector);
            SetRotMatrix(&Work);
            SetTransMatrix(&Work);
            // Cast nextpri as POLY_GT3
            poly[0] = (POLY_GT3 *)nextpri;
            poly[1] = (POLY_GT3 *)nextpri+sizeof(POLY_GT3);
            // Initialize the primitives
            SetPolyGT3(poly[0]);
            SetPolyGT3(poly[1]);

            // Reflection Cube
            // This cube has its UVs mapped directly to VRAM coordinates
            // We're using the framebuffers as a texture (0,0 and 0,256)
            // Get 256x256 texture page that's at x0, y0
            poly[1]->tpage = getTPage( 2, stpRate,
                                      0,
                                      !(db) << 8 // Here, we're using db's value that can be either 0 or 1 to determine the texture page Y coordinate.
                                      );
            // Set STP
            SetSemiTrans(poly[1], stpFlag); 
            // Map coordinates from drawarea (320x240) to texture size (128x128) in fixed point math
            // x = x * (256 / 320) => ( x * ( 128 * 4096 ) / 320 ) / 4096
            // y = y * (240 / 240) => ( y * ( 240 * 4096 ) / 240 ) / 4096 => y * 2184 >> 12 -> y
            setUV3( poly[1],  
                (poly[1]->x0 * normH) >> 12,
                poly[1]->y0 - (!(db) << 4) , // We're using db's value again to add a 16 pixels offset to the Y's coordinates of the UVs
                (poly[1]->x1 * normH) >> 12,
                poly[1]->y1 - (!(db) << 4),  // We have to do that because the buffer is 240 high, whereas our texture page is 256, hence 256 - 240 == 16
                (poly[1]->x2 * normH) >> 12,
                poly[1]->y2 - (!(db) << 4)
            );
            
            // Draw "container" cube
            // This cube has a texture with transparent areas. 
            // STP bit is set on PNG's alpha channel : img2tim -usealpha -org 320 0 -o cube.tim cube.png
            poly[0]->tpage = getTPage( timImages[0].mode&0x3, stpRate,
                                          timImages[0].prect->x,
                                          timImages[0].prect->y
                                          );
            // If 8/4bpp, load CLUT to vram
            if ( (timImages[0].mode & 0x3) < 2 ) {
                setClut( poly[0],          
                         timImages[0].crect->x,
                         timImages[0].crect->y
                );
            }
            // Set UV coordinates
            setUV3(poly[0], modelCube.u[i].vx, modelCube.u[i].vy,
                            modelCube.u[i+2].vx, modelCube.u[i+2].vy,
                            modelCube.u[i+1].vx, modelCube.u[i+1].vy
                    );
            // Rotate, translate, and project the vectors and output the results into a primitive
            // curTriangle, +1, +2 point to the vertices index of the triangle we're drawing.
            OTz  = RotTransPers(&modelCube_mesh[ modelCube_index[ curTriangle[0] ] ]  , ( long * ) &poly[1]->x0, &p, &Flag);
            OTz += RotTransPers(&modelCube_mesh[ modelCube_index[ curTriangle[0] + 2] ], ( long*) &poly[1]->x1, &p, &Flag);
            OTz += RotTransPers(&modelCube_mesh[ modelCube_index[ curTriangle[0] + 1] ], ( long * ) &poly[1]->x2, &p, &Flag);
            
            // Here we're only messing with the matrices so that the foreground cube can be moved independantly from the backgound one.
            // In real code, you don't want to do the same calculation twice !            
            transVector.vx = offsetCube;
            TransMatrix(&Work, &transVector);
            SetRotMatrix(&Work);
            SetTransMatrix(&Work);
            
            OTz  = RotTransPers(&modelCube_mesh[ modelCube_index[ curTriangle[0] ] ]  , ( long * ) &poly[0]->x0, &p, &Flag);
            OTz += RotTransPers(&modelCube_mesh[ modelCube_index[ curTriangle[0] + 2] ], ( long*)  &poly[0]->x1, &p, &Flag);
            OTz += RotTransPers(&modelCube_mesh[ modelCube_index[ curTriangle[0] + 1] ], ( long * ) &poly[0]->x2, &p, &Flag);
            
            // The right way to do it is re-using the results from the first RotTransPer() batch
            // i.e commenting lines 273 to 280 and uncommenting lines 284 to 289
            //~ poly[0]->x0 = poly[1]->x0;
            //~ poly[0]->y0 = poly[1]->y0;
            //~ poly[0]->x1 = poly[1]->x1;
            //~ poly[0]->y1 = poly[1]->y1;
            //~ poly[0]->x2 = poly[1]->x2;
            //~ poly[0]->y2 = poly[1]->y2;
            
            // Average OTz value for 3 vertices
            // OTz is 1/4 of screen to vertex length
            OTz /= 3;
            // Work color vectors
            // This is the hue of the transparent cube
            CVECTOR prismCol = {0xff,0xff,0x0,0x0};
            // This will store the result of the depth cueing.
            CVECTOR outCol, outCol1, outCol2  = { 0,0,0,0 };
            // Find local color from three normal vectors and perform depth cueing.
            gte_NormalColorDpq3( &modelCube.n[i+0],
                                 &modelCube.n[i+2],
                                 &modelCube.n[i+3],
                                 &prismCol, p, &outCol, &outCol1, &outCol2);
            // Set vertex colors on transparent/background cube                 
            setRGB0(poly[1], outCol.r, outCol.g  , outCol.b);
            setRGB1(poly[1], outCol1.r, outCol1.g, outCol1.b);
            setRGB2(poly[1], outCol2.r, outCol2.g, outCol2.b);
            // Non-transparent/foreground cube color
            // Find local color from three normal vectors and perform depth cueing.
            gte_NormalColorDpq( &modelCube.n[i+0], &modelCube.c[i+0], p, &outCol);
            gte_NormalColorDpq( &modelCube.n[i+2], &modelCube.c[i+2], p, &outCol2);
            gte_NormalColorDpq( &modelCube.n[i+1], &modelCube.c[i+1], p, &outCol1);
            // Set vertex colors                 
            setRGB0(poly[0], outCol.r, outCol.g  , outCol.b);
            setRGB1(poly[0], outCol1.r, outCol1.g, outCol1.b);
            setRGB2(poly[0], outCol2.r, outCol2.g, outCol2.b);
            // If OTz is in range (not too close) 
            if ((OTz > 0) && (OTz < OTLEN))
                // Add to ordering table, at index OTz-2 
                AddPrim(&ot[ db ][ OTz-2 ], poly[0]);
                AddPrim(&ot[ db ][ OTz-2 ], poly[1]);
            // Increment next primitive address
            nextpri += sizeof(POLY_GT3)*2;
            // Increment to next triangle
            curTriangle[0] += 3;
            curTriangle[1] += 3;
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
        if ( pad & PADRdown && !( oldPad & PADRdown ) ){
            // Switch STP rates 
            offsetCube = 0;
            // Set flag to avoir misfire
            oldPad = pad;
        } 
        // Reset flag when button released
        if (!(pad & PADRdown) && oldPad & PADRdown) {
            oldPad = pad;
        }
        if ( pad & PADLright && !( oldPad & PADLright ) ){
            offsetCube += 6;
        } 
        if ( pad & PADLleft && !( oldPad & PADLleft ) ){
            offsetCube -= 6;
        } 
        FntPrint("Hello fx  !\n");
        FntPrint("Select: STP on/off\nStart: Cycle STP rates\nLeft/Right: Move FG cube.\nX: Reset cube pos\n");
        FntPrint("STP : %d\n", stpFlag);
        FntPrint("STP rate : %d\n", stpRate);
        FntFlush(-1);
        display();
    }
    return 0;
}
