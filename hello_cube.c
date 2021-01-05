
/*  primdrawG.c, by Schnappy, 12-2020
    
    - Draw a gouraud shaded mesh exported as a TMESH by the blender <= 2.79b plugin io_export_psx_tmesh.py
    
    based on primdraw.c by Lameguy64 (http://www.psxdev.net/forum/viewtopic.php?f=64&t=537)
	2014 Meido-Tek Productions.
	
	Demonstrates:
		- Using a primitive OT to draw triangles without libgs.
		- Using the GTE to rotate, translate, and project 3D primitives.
	
	Controls:
		Start							- Toggle interactive/non-interactive mode.
		Select							- Reset object's position and angles.
		L1/L2							- Move object closer/farther.
		L2/R2							- Rotate object (XY).
		Up/Down/Left/Right				- Rotate object (XZ/YZ).
		Triangle/Cross/Square/Circle	- Move object up/down/left/right.
		
*/
 /*		   PSX screen coordinate system 
 *
 *                           Z+
 *                          /
 *                         /
 *                        +------X+
 *                       /|
 *                      / |
 *                     /  Y+
 *                   eye		*/

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <stdio.h>

// Sample vector model
#include "cube.c"

#define VMODE       0

#define SCREENXRES 320
#define SCREENYRES 240

#define CENTERX		SCREENXRES/2
#define CENTERY		SCREENYRES/2

#define OTLEN	    2048	    // Maximum number of OT entries
#define PRIMBUFFLEN	32768	    // Maximum number of POLY_GT3 primitives

// Display and draw environments, double buffered
DISPENV disp[2];
DRAWENV draw[2];

u_long	    ot[2][OTLEN];   		        // Ordering table (contains addresses to primitives)
char	    primbuff[2][PRIMBUFFLEN] = {0};	// Primitive list // That's our prim buffer

char * nextpri = primbuff[0];			            // Primitive counter

short		    db	= 0;                        // Current buffer counter

// Prototypes
void init(void);
void display(void);
//~ void LoadTexture(u_long * tim, TIM_IMAGE * tparam);

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
	
    setRGB0(&draw[0], 0, 0, 255);
    setRGB0(&draw[1], 0, 0, 255);

    draw[0].isbg = 1;
    draw[1].isbg = 1;

    PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);
		
	// Init font system
	FntLoad(960, 0);
	FntOpen(16, 16, 196, 64, 0, 256);
	
    }

void display(void){
    
    DrawSync(0);
    VSync(0);

    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);

    SetDispMask(1);
    
    DrawOTag(ot[db] + OTLEN - 1);
    
    db = !db;

    nextpri = primbuff[db];
    
        
    }

int main() {
		
	int		i;
	int		PadStatus;
	int		TPressed=0;
	int		AutoRotate=1;
	
	long	t, p, OTz, Flag;                // t == vertex count, p == depth cueing interpolation value, OTz ==  value to create Z-ordered OT, Flag == see LibOver47.pdf, p.143
	
    POLY_G3 *poly = {0};                           // pointer to a POLY_G4 

    
	SVECTOR	Rotate={ 0 };					// Rotation coordinates
	VECTOR	Trans={ 0, 0, CENTERX, 0 };		// Translation coordinates
                                            // Scaling coordinates
	VECTOR	Scale={ ONE, ONE, ONE, 0 };     // ONE == 4096
	MATRIX	Matrix={0};						// Matrix data for the GTE
	
    // Texture window
    
    DR_MODE * dr_mode;                        // Pointer to dr_mode prim
    
    RECT tws = {0, 0, 32, 32};            // Texture window coordinates : x, y, w, h
                
	init();
    
	// Main loop
	while (1) {
	
		// Read pad status
		PadStatus = PadRead(0);
		
		if (AutoRotate == 0) {
		
			if (PadStatus & PADL1) Trans.vz -= 4;
			if (PadStatus & PADR1) Trans.vz += 4;
			if (PadStatus & PADL2) Rotate.vz -= 8;
			if (PadStatus & PADR2) Rotate.vz += 8;

			if (PadStatus & PADLup)		Rotate.vx -= 8;
			if (PadStatus & PADLdown)	Rotate.vx += 8;
			if (PadStatus & PADLleft)	Rotate.vy -= 8;
			if (PadStatus & PADLright)	Rotate.vy += 8;
			
			if (PadStatus & PADRup)		Trans.vy -= 2;
			if (PadStatus & PADRdown)	Trans.vy += 2;
			if (PadStatus & PADRleft)	Trans.vx -= 2;
			if (PadStatus & PADRright)	Trans.vx += 2;
					
			if (PadStatus & PADselect) {
				Rotate.vx = Rotate.vy = Rotate.vz = 0;
				Scale.vx = Scale.vy = Scale.vz = ONE;
				Trans.vx = Trans.vy = 0;
				Trans.vz = CENTERX;
			}
			
		}
		
		if (PadStatus & PADstart) {
			if (TPressed == 0) {
				AutoRotate = (AutoRotate + 1) & 1;
				Rotate.vx = Rotate.vy = Rotate.vz = 0;
				Scale.vx = Scale.vy = Scale.vz = ONE;
				Trans.vx = Trans.vy = 0;
				Trans.vz = CENTERX;
			}
			TPressed = 1;
		} else {
			TPressed = 0;
		}

		if (AutoRotate) {
			Rotate.vy += 8;	// Pan
			Rotate.vx += 8;	// Tilt
			//~ Rotate.vz += 8;	// Roll
		}
		
		
		// Clear the current OT
		ClearOTagR(ot[db], OTLEN);
        
		// Convert and set the matrixes
		RotMatrix(&Rotate, &Matrix);
		TransMatrix(&Matrix, &Trans);
		ScaleMatrix(&Matrix, &Scale);
		
		SetRotMatrix(&Matrix);
		SetTransMatrix(&Matrix);
		
		
		// Render the sample vector model
		t=0;
        
        // modelCube is a TMESH, len member == # vertices, but here it's # of triangle... So, for each tri * 3 vertices ...
		for (i = 0; i < (modelCube.len*3); i += 3) {               
			
            poly = (POLY_G3 *)nextpri;
			
            // Initialize the primitive and set its color values
			
            SetPolyG3(poly);

			setRGB0(poly, modelCube.c[i].r , modelCube.c[i].g   , modelCube.c[i].b);
			setRGB1(poly, modelCube.c[i+1].r, modelCube.c[i+1].g, modelCube.c[i+1].b);
			setRGB2(poly, modelCube.c[i+2].r, modelCube.c[i+2].g, modelCube.c[i+2].b);

            // Rotate, translate, and project the vectors and output the results into a primitive

            OTz  = RotTransPers(&modelCube_mesh[modelCube_index[t]]  , (long*)&poly->x0, &p, &Flag);
			OTz += RotTransPers(&modelCube_mesh[modelCube_index[t+1]], (long*)&poly->x1, &p, &Flag);
			OTz += RotTransPers(&modelCube_mesh[modelCube_index[t+2]], (long*)&poly->x2, &p, &Flag);
			
			// Sort the primitive into the OT
			OTz /= 3;
			if ((OTz > 0) && (OTz < OTLEN))
				AddPrim(&ot[db][OTz-2], poly);
			
			nextpri += sizeof(POLY_G3);
            
			t+=3;
			
		}
        
        FntFlush(-1);
		
		display();

	}
    return 0;
}
