#include "../common.h"

int ovl_main_tile(void)
{
    setRGB(&BGcolor, 150, 0, 50);
    init();
    uint16_t timer = 0;
    uint16_t timeout = 100;
    TILE * blue_tile;
    TILE * pink_tile;
    // This one is added at a different OT index
    TILE * yellow_tile;
    while(1)
    {
        // Initialize the reversed ordering table. This means the elements at index OTLEN - 1 is drawn first.
        ClearOTagR(ot[db], OTLEN);

        // yellow_tile
        
        yellow_tile = (TILE * ) nextpri;                   // yellow_tile is a pointer to primbuf content at adress nextpri, that's cast (type converted) to a TILE struc.   
        
        setTile(yellow_tile);                              // initialize the TILE structure ( fill the length and tag(?) value )
        setXY0(yellow_tile, CENTERX - 32 , CENTERY - 48);  // Set X,Y
        setWH(yellow_tile, 128, 40);                       // Set Width, Height
        setRGB0(yellow_tile, 255, 255, 0);                 // Set color
        addPrim(ot[db][OTLEN - 1], yellow_tile);                    // Add primitive to ordering table
            
        nextpri += sizeof(TILE);     
        
        // blue_tile 
        
        blue_tile = (TILE * ) nextpri;                  // blue_tile is a pointer to primbuf content at adress nextpri, that's cast (type converted) to a blue_tile struc.   
        
        setTile(blue_tile);                              // initialize the blue_tile structure ( fill the length and tag(?) value )
        setXY0(blue_tile, CENTERX - 16, CENTERY - 32);   // Set X,Y
        setWH(blue_tile, 32, 64);                        // Set Width, Height
        setRGB0(blue_tile, 60, 180, 255);                // Set color
        addPrim(ot[db][OTLEN - 2], blue_tile);                      // Add primitive to ordering table

        nextpri += sizeof(TILE);                    // Increment the adress nextpri points to by the size of TILE struct
        
        // pink_tile
        
        pink_tile = (TILE * ) nextpri;                  // pink_tile is a pointer to primbuf content at adress nextpri, that's cast (type converted) to a TILE struc.   
        
        setTile(pink_tile);                              // initialize the TILE structure ( fill the length and tag(?) value )
        setXY0(pink_tile, CENTERX, CENTERY - 64);   // Set X,Y
        setWH(pink_tile, 64, 64);                        // Set Width, Height
        setRGB0(pink_tile, 255, 32, 255);                // Set color
        addPrim(ot[db][OTLEN - 2], pink_tile);                      // Add primitive to ordering table
            
        nextpri += sizeof(TILE);     
        
        timer ++;
        
        FntPrint("Hello tile ! %d\n", timer);
        #ifndef STANDALONE
            if (timer == timeout){
                next_overlay = OVERLAY_POLY;
                // Empty ordering table
                //~ EmptyOTag(&ot[db], &primbuff[db], nextpri);
                break;
            }
        #endif
        FntFlush(-1);
        display();
    }
    return next_overlay;
};
