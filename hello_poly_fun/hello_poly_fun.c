// Having fun with polygons, matrices and vectors
// Credits : Schnappy
//With great help from Jaby smoll Seamonstah, Nicolas Noble, NDR008, paul, sickle on https://discord.com/invite/Zd82yXvs
// 11/2020

#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <kernel.h>

#define VMODE 0         // Video Mode : 0 : NTSC, 1: PAL

#define SPIN 16         // Rotation speed increment

#define SCREENXRES 320
#define SCREENYRES 240

#define CENTERX SCREENXRES/2
#define CENTERY SCREENYRES/2

#define MARGINX 10    // margins for text display
#define MARGINY 4

#define FONTSIZE 8 * 7           // Text Field Height

#define OTLEN 16              // Ordering Table Length 

DISPENV disp[2];             // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];

u_long ot[2][OTLEN];     // double ordering table of length 8 * 32 = 256 bits / 32 bytes

char primbuff[2][32768] = {};     // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes

char *nextpri = primbuff[0]; // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]

short db = 0;                // index of which buffer is used, values 0, 1

CVECTOR BgColor[3] = {20, 20, 20}; 

struct polygon
    { 
    POLY_F4 * poly_f4;
    CVECTOR   color;
    short     width;
    short     height;
    //~ VECTOR    PosV_L; // Not used anymore
    SVECTOR   RotV_L;
    VECTOR    TransV_L;
    VECTOR    ScaleV_L;
    SVECTOR   PivotV_L;
    SVECTOR   Verts[4];
    MATRIX    Matrix;
    long      depth;
    long      flag;
    short     rotSpeed;
    int       otz;
    };

void init(void)
{
    ResetGraph(0);
    
    // Initialize and setup the GTE : Not needed ?
    
    InitGeom();
    SetGeomOffset(CENTERX,CENTERY);
    SetGeomScreen(CENTERX);
    
    PadInit(0);
    
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

    setRGB0(&draw[0], BgColor->r, BgColor->g, BgColor->b);
    setRGB0(&draw[1], BgColor->r, BgColor->g, BgColor->b);
    
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
    
    DrawOTag(ot[db] + OTLEN - 1);
    
    db = !db;
    
    nextpri = primbuff[db];
}


void pivotPoint(SVECTOR VertPos[3],short width,short height, SVECTOR pivot){

        // Not very efficient I think

        VertPos[0].vx = -pivot.vx;
        VertPos[0].vy = -pivot.vy;
        VertPos[0].vz = 1;

        VertPos[1].vx = width - pivot.vx;
        VertPos[1].vy = -pivot.vy;
        VertPos[1].vz = 1;

        VertPos[2].vx = -pivot.vx;
        VertPos[2].vy = height-pivot.vy;
        VertPos[2].vz = 1;

        VertPos[3].vx = width  - pivot.vx;
        VertPos[3].vy = height - pivot.vy;
        VertPos[3].vz = 1;
}


MATRIX identity(int num)
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

int main(void)
{
    
    MATRIX IDMATRIX = identity(3);
    
    u_short BtnTimer = 0;           // Timer to limit pad input rate
    u_short polyCount = 1;          // current polygon index
    
    int otz;                        // z-index
    
    struct polygon *CurrentPoly;    // points to the address of selected polygon
    
    
    // White cursor : shows which polygon is selected
    
    struct polygon cursorS = {
        cursorS.poly_f4,
        {255, 255, 255},        // color
        30, 30,                 // width, height
        {0,0,0},                // RotV_L
        {0,0,0, 0},             // TransV_L
        {4096,4096,4096},       // ScaleV_L
        {1,1,1},                // PivotV
        {                       // Verts[4]
            {-1, -1, 1},
            { 1, -1, 1},
            {-1,  1, 1},
            { 1,  1, 1}
        },
        IDMATRIX              // Matrix
    };
    
    //Red
    
    struct polygon polyS = {
    polyS.poly_f4,
    {255, 0, 0},                      // color
    30, 30,                           // width, height
    {0,0,0},                          // RotV_L
    {-48, -30, 0, 0},                    // TransV_L
    {4096,4096,4096},                 // ScaleV_L
    {15,15,1},                        // PivotV
    {                                 // Verts[4]
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    },
    IDMATRIX,                       // Matrix
    0,0,                              // depth, flag
    8,                                // rotSpeed
    1                                 // z-index
    };
    

    //Yellow

    struct polygon poly1S = {
    poly1S.poly_f4,
    {255, 187, 0},                    // color
    28, 28,                           // width, height
    {0,0,0},                          // RotV_L
    {-20, 10, 0, 0},                  // TransV_L
    {4096,4096,4096},                 // ScaleV_L
    {4,4,1},                          // PivotV
    {                                 // Verts[4]
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    },
    IDMATRIX,                       // Matrix
    0,0,                              // depth, flag
    -12,                              // rotSpeed
    2                                 // z-index
    };

        
    //Green
    
    struct polygon poly2S = {
    poly2S.poly_f4,
    {0, 255, 153},                    // color
    24, 24,                           // width, height
    {0,0,0},                          // RotV_L
    {36, -10, 0, 0},                  // TransV_L
    {4096,4096,4096},                 // ScaleV_L
    {12,12,1},                        // PivotV
    {                                 // Verts[4]
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    },
    IDMATRIX,                       // Matrix
    0,0,                              // depth, flag
    -6,                               // rotSpeed
    3                                 // z-index
    };
 

    //Blue
    
    struct polygon poly3S = {
    poly3S.poly_f4,
    {112, 254, 254},                  // color
    26, 26,                           // width, height
    {0,0,0},                          // RotV_L
    {20, 20, 0, 0},                   // TransV_L
    {4096,4096,4096},                 // ScaleV_L
    {13,13,1},                        // PivotV
    {                                 // Verts[4]
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    },
    IDMATRIX,                         // Matrix
    0,0,                              //depth, flag
    256,                              //rotSpeed
    4                                 // z-index
    };
    
    
    /////

    CurrentPoly = &polyS;
    
    pivotPoint(polyS.Verts, polyS.width, polyS.height, polyS.PivotV_L);
    pivotPoint(poly1S.Verts, poly1S.width, poly1S.height, poly1S.PivotV_L);
    pivotPoint(poly2S.Verts, poly2S.width, poly2S.height, poly2S.PivotV_L);
    pivotPoint(poly3S.Verts, poly3S.width, poly3S.height, poly3S.PivotV_L);
 
    init();
    
    while (1)
    {
        ClearOTagR(ot[db], OTLEN);
        
        cursorS.poly_f4 = (POLY_F4 *)nextpri;
        
        RotMatrix(&cursorS.RotV_L , &cursorS.Matrix);
        TransMatrix(&cursorS.Matrix, &CurrentPoly->TransV_L);
        
        SetRotMatrix(&cursorS.Matrix);
        SetTransMatrix(&cursorS.Matrix);
        
        setPolyF4(cursorS.poly_f4);
        setRGB0(cursorS.poly_f4,cursorS.color.r,cursorS.color.g,cursorS.color.b);
        //~ setXY4(cursorS, MovVector.vx-1, MovVector.vy-1 ,MovVector.vx + 1, MovVector.vy -1,MovVector.vx-1, MovVector.vy+1,MovVector.vx+1, MovVector.vy+1);
        
        RotTransPers4(
                    &cursorS.Verts[0],      &cursorS.Verts[1],      &cursorS.Verts[2],      &cursorS.Verts[3],
                    (long*)&cursorS.poly_f4->x0, (long*)&cursorS.poly_f4->x1, (long*)&cursorS.poly_f4->x2, (long*)&cursorS.poly_f4->x3,
                    &cursorS.depth,
                    &cursorS.flag
                    );
        
        addPrim(ot[db], cursorS.poly_f4);
        
        nextpri += sizeof(POLY_F4);
        
        ///// Red
        
        polyS.poly_f4 = (POLY_F4 *)nextpri;
        
        polyS.RotV_L.vz += polyS.rotSpeed;
        
        RotMatrix(&polyS.RotV_L, &polyS.Matrix);
        TransMatrix(&polyS.Matrix, &polyS.TransV_L);
        ScaleMatrix(&polyS.Matrix, &polyS.ScaleV_L);
        
        SetRotMatrix(&polyS.Matrix);
        SetTransMatrix(&polyS.Matrix);
        
        setPolyF4(polyS.poly_f4);
        setRGB0(polyS.poly_f4, polyS.color.r,polyS.color.g,polyS.color.b);        
        RotTransPers4(
                    &polyS.Verts[0],      &polyS.Verts[1],      &polyS.Verts[2],      &polyS.Verts[3],
                    (long*)&polyS.poly_f4->x0, (long*)&polyS.poly_f4->x1, (long*)&polyS.poly_f4->x2, (long*)&polyS.poly_f4->x3,
                    &polyS.depth,
                    &polyS.flag
                    );
        
        addPrim(ot[db]+polyS.otz, polyS.poly_f4);
        
        nextpri += sizeof(POLY_F4);
        
        ///// Yellow
        
        poly1S.poly_f4 = (POLY_F4 *)nextpri;
        
        poly1S.RotV_L.vz += poly1S.rotSpeed;
        
        RotMatrix(&poly1S.RotV_L, &poly1S.Matrix);
        TransMatrix(&poly1S.Matrix, &poly1S.TransV_L);
        ScaleMatrix(&poly1S.Matrix, &poly1S.ScaleV_L);

        
        SetRotMatrix(&poly1S.Matrix);
        SetTransMatrix(&poly1S.Matrix);
        
        setPolyF4(poly1S.poly_f4);
        setRGB0(poly1S.poly_f4, poly1S.color.r,poly1S.color.g,poly1S.color.b);        
        RotTransPers4(
                    &poly1S.Verts[0],      &poly1S.Verts[1],      &poly1S.Verts[2],      &poly1S.Verts[3],
                    (long*)&poly1S.poly_f4->x0, (long*)&poly1S.poly_f4->x1, (long*)&poly1S.poly_f4->x2, (long*)&poly1S.poly_f4->x3,
                    &poly1S.depth,
                    &poly1S.flag
                    );
        
        addPrim(ot[db]+poly1S.otz, poly1S.poly_f4);
        
        nextpri += sizeof(POLY_F4);
        
        
        ///// Green
        
        poly2S.poly_f4 = (POLY_F4 *)nextpri;
                
        poly2S.RotV_L.vz += poly2S.rotSpeed;
        
        RotMatrix(&poly2S.RotV_L, &poly2S.Matrix);
        TransMatrix(&poly2S.Matrix, &poly2S.TransV_L);
        ScaleMatrix(&poly2S.Matrix, &poly2S.ScaleV_L);

        
        SetRotMatrix(&poly2S.Matrix);
        SetTransMatrix(&poly2S.Matrix);
        
        setPolyF4(poly2S.poly_f4);
        setRGB0(poly2S.poly_f4, poly2S.color.r,poly2S.color.g,poly2S.color.b);        
        RotTransPers4(
                    &poly2S.Verts[0],      &poly2S.Verts[1],      &poly2S.Verts[2],      &poly2S.Verts[3],
                    (long*)&poly2S.poly_f4->x0, (long*)&poly2S.poly_f4->x1, (long*)&poly2S.poly_f4->x2, (long*)&poly2S.poly_f4->x3,
                    &poly2S.depth,
                    &poly2S.flag
                    );
        
        addPrim(ot[db]+poly2S.otz, poly2S.poly_f4);
        
        nextpri += sizeof(POLY_F4);
        
        ///// Blue
        
        poly3S.poly_f4 = (POLY_F4 *)nextpri;
                
        poly3S.RotV_L.vz += poly3S.rotSpeed;
        
        RotMatrix(&poly3S.RotV_L, &poly3S.Matrix);
        TransMatrix(&poly3S.Matrix, &poly3S.TransV_L);
        ScaleMatrix(&poly3S.Matrix, &poly3S.ScaleV_L);
        
        SetRotMatrix(&poly3S.Matrix);
        SetTransMatrix(&poly3S.Matrix);
        
        setPolyF4(poly3S.poly_f4);
        setRGB0(poly3S.poly_f4, poly3S.color.r,poly3S.color.g,poly3S.color.b);        
        RotTransPers4(
                    &poly3S.Verts[0],      &poly3S.Verts[1],      &poly3S.Verts[2],      &poly3S.Verts[3],
                    (long*)&poly3S.poly_f4->x0, (long*)&poly3S.poly_f4->x1, (long*)&poly3S.poly_f4->x2, (long*)&poly3S.poly_f4->x3,
                    &poly3S.depth,
                    &poly3S.flag
                    );
        
        addPrim(ot[db]+poly3S.otz, poly3S.poly_f4);
        
        nextpri += sizeof(POLY_F4);
        
        
        // Pad stuff
        
        
        int pad = PadRead(0); // init pad
        
        // Right D-pad
        
        if(pad & PADRup){
            if (CurrentPoly->PivotV_L.vy >= 0){
                CurrentPoly->PivotV_L.vy -= 1;
                pivotPoint(CurrentPoly->Verts, CurrentPoly->width, CurrentPoly->height, CurrentPoly->PivotV_L);
            }
            else {
                CurrentPoly->PivotV_L.vy = CurrentPoly->PivotV_L.vy;
            }
        };
        
        if(pad & PADRdown){
            if (CurrentPoly->PivotV_L.vy <= CurrentPoly->height ){
                CurrentPoly->PivotV_L.vy += 1;
                pivotPoint(CurrentPoly->Verts, CurrentPoly->width, CurrentPoly->height, CurrentPoly->PivotV_L);
            }
            else {
                CurrentPoly->PivotV_L.vy = CurrentPoly->PivotV_L.vy;
            }
        };
        
        if(pad & PADRleft){
            if (CurrentPoly->PivotV_L.vx >= 0){
                CurrentPoly->PivotV_L.vx -= 1;
                pivotPoint(CurrentPoly->Verts, CurrentPoly->width, CurrentPoly->height, CurrentPoly->PivotV_L);
            }
            else {
                CurrentPoly->PivotV_L.vx = CurrentPoly->PivotV_L.vx;
            }
        };
        
        if(pad & PADRright){
            if (CurrentPoly->PivotV_L.vx <= CurrentPoly->width ){
                CurrentPoly->PivotV_L.vx += 1;
                pivotPoint(CurrentPoly->Verts, CurrentPoly->width, CurrentPoly->height, CurrentPoly->PivotV_L);
            }
            else {
                CurrentPoly->PivotV_L.vx = CurrentPoly->PivotV_L.vx;
            }
        };
        
        // R1, R2, L2, L2
        
        if(pad & PADR1){

            if(BtnTimer == 0){
                
                if (polyCount < 4){ 
                    CurrentPoly -= 1;
                    BtnTimer = 10;
                    polyCount++;
                }
                else {
                    CurrentPoly = &polyS + 1;
                    polyCount = 0;
                    }
            }
        }
        
         if(pad & PADR2){
             
            if(BtnTimer == 0){
                if(CurrentPoly->otz < 5 ){ 
                    CurrentPoly->otz += 1;
                    BtnTimer = 10;
                } else {
                    CurrentPoly->otz = 1;
                    BtnTimer = 10;
                    }
            } 
        }
        
         if(pad & PADL1){
             
            if(BtnTimer == 0){
                if (CurrentPoly->rotSpeed <= 320){
                CurrentPoly->rotSpeed += 8;
            }
                BtnTimer = 10;
            } 
        }
        
        if(pad & PADL2){
             
            if(BtnTimer == 0){
                if (CurrentPoly->rotSpeed >= -320){
                CurrentPoly->rotSpeed -= 8;
            }
                BtnTimer = 10;
            } 
        }
        
        // Left D-Pad
        
        if(pad & PADLup){
             
            if(BtnTimer == 0){
                CurrentPoly->TransV_L.vy -= 1;
                //~ BtnTimer = 2;
            } 
        }
        if(pad & PADLdown){
             
            if(BtnTimer == 0){
                CurrentPoly->TransV_L.vy += 1;
                //~ BtnTimer = 2;
            } 
        }
        if(pad & PADLleft){
             
            if(BtnTimer == 0){
                CurrentPoly->TransV_L.vx -= 1;
                //~ BtnTimer = 2;
            } 
        }
        if(pad & PADLright){
             
            if(BtnTimer == 0){
                CurrentPoly->TransV_L.vx += 1;
                //~ BtnTimer = 2;
            } 
        }
        if(pad & PADstart){
             
            if(BtnTimer == 0){
                CurrentPoly->ScaleV_L.vx += 100;
                CurrentPoly->ScaleV_L.vy += 100;
                //~ CurrentPoly->TransV_L.vz += 1;

            } 
        }
        if(pad & PADselect){
             
            if(BtnTimer == 0){
               CurrentPoly->ScaleV_L.vx -= 100;
               CurrentPoly->ScaleV_L.vy -= 100;
               //~ CurrentPoly->TransV_L.vz -= 1;

            } 
        }
        
        // Btn_timer decrement
        
        if(BtnTimer > 0){
            BtnTimer -= 1;
            }

        // Debug stuff

        // Display Rotation matrix
        
        //~ FntPrint("Rotmatrix:\n%d %d %d\n%d %d %d\n%d %d %d \n",
                //~ Poly1Matrix.m[0][0], Poly1Matrix.m[0][1], Poly1Matrix.m[0][2],
                //~ Poly1Matrix.m[1][0], Poly1Matrix.m[1][1], Poly1Matrix.m[1][2],
                //~ Poly1Matrix.m[2][0], Poly1Matrix.m[2][1], Poly1Matrix.m[2][2]);
      
        // Display Mem adress and values of verticess
        //~ FntPrint("cur:%x\n  0:%x\n  1:%x\n  2:%x\n  3:%x\n", CurrentPoly, &polyS, &poly1S, &poly2S, &poly3S);
        //~ FntPrint("timer:%d polyCount:%d speed:%d", BtnTimer, polyCount, CurrentPoly->rotSpeed );

        //~ FntPrint("&poly->x0 Addr:%x Value:%d \n&poly->y0 Addr:%x Value:%d \n&poly->x1 Addr:%x Value:%d",
                //~ (long)&poly->x0, poly->x0,
                //~ (long)&poly->y0, poly->y0,
                //~ (long)&poly->x1, poly->x1);

        //~ FntPrint("otz : %d\n" , CurrentPoly->rotSpeed);

        // On-screen instructions 

        FntPrint("\
D-Pad:move polygon.\n\
[],X,O,\/\\ : Move pivot point.\n\
L1,L2 : Rotations speed +/-\n\
R1 : select polygon\n\
R2 : change z-index\n\
Start,Select : Scale polygon +/-\
");

        FntFlush(-1);
        
        display();
        }
    return 0;
    }
