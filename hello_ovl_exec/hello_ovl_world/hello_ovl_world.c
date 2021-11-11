#include "../common.h"

int ovl_main_hello(void)
{   
    init();
    int i = 0;
    while(1)
    {
        i++;
        #ifndef STANDALONE
        if (i==100){
            next_overlay = OVERLAY_TILE;
            break;
        }
            FntPrint("Hello world ! %d\n", i);
        #else
            FntPrint("Hello standalone ! %d\n", i);
        #endif
        FntFlush(-1);
        display();
    }
    return next_overlay;
};

