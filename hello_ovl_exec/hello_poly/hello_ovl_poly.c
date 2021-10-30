#include "../common.h"

int ovl_main_poly(void)
{   
    setRGB(&BGcolor, 250, 0, 250);
    init();
    uint16_t timer = 0;
    uint16_t timeout = 100;
    MATRIX IDMATRIX = {0};                 
    POLY_F4 *poly = {0};                   
    SVECTOR RotVector = {0, 0, 0};         
    VECTOR  MovVector = {0, 0, CENTERX, 0};
    SVECTOR VertPos[4] = {                 
            {-32, -32, 1 },                
            {-32,  32, 1 },                
            { 32, -32, 1 },                
            { 32,  32, 1  }                
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
        SetRotMatrix(&PolyMatrix);                    // Set default rotation matrix
        SetTransMatrix(&PolyMatrix);                  // Set default transformation matrix
        setPolyF4(poly);                              // Initialize poly as a POLY_F4 
        setRGB0(poly, 255, 255, 0);                   // Set poly color

        // RotTransPers
        OTz = RotTransPers4(
                    &VertPos[0],      &VertPos[1],      &VertPos[2],      &VertPos[3],
                    (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2, (long*)&poly->x3,
                    &polydepth,
                    &polyflag
                    );                                // Perform coordinate and perspective transformation for 4 vertices
        
        RotVector.vy += 4;
        RotVector.vz += 4;                              // Apply rotation on Z-axis. On PSX, the Z-axis is pointing away from the screen.  

        //~ addPrim(ot[db], poly);                         // add poly to the Ordering table
        addPrim(ot[db][OTLEN-1], poly);                         // add poly to the Ordering table
        
        nextpri += sizeof(POLY_F4);                    // increment nextpri address with size of a POLY_F4 struct 
        
        timer++;
    
        FntPrint("Hello Poly ! %d", timer);                   
        #ifndef STANDALONE
            if (timer == timeout){
                next_overlay = MOTHERSHIP;
                break;
            }
        #endif
        FntFlush(-1);
        display();
    }
    return next_overlay;
};
