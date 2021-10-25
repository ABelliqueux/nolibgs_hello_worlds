// Change the debug font face and colors !
// Schnappy 2020
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
#define MARGINX 96                // margins for text display
#define MARGINY 64
#define FONTSIZE 8 * 7           // Text Field Height
DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
short db = 0;                      // index of which buffer is used, values 0, 1
#define FONTX   960
#define FONTY   0
// Two color vectors R,G,B
CVECTOR fntColor = { 255, 0, 0 };
CVECTOR fntColorBG = { 0, 0, 0 };

extern unsigned long _binary_fnt_tim_start[];
extern unsigned long _binary_fnt_tim_end[];
extern unsigned long _binary_fnt_tim_length;

// Loading an image to vram. See https://github.com/ABelliqueux/nolibgs_hello_worlds/blob/main/hello_sprt/hello_sprt.c#L42
TIM_IMAGE fontface;
void LoadTexture(u_long * tim, TIM_IMAGE * tparam){     
        OpenTIM(tim);                                   
        ReadTIM(tparam);                                
        LoadImage(tparam->prect, tparam->paddr);        
        DrawSync(0);                                    
        if (tparam->mode & 0x8){ // check 4th bit       
            LoadImage(tparam->crect, tparam->caddr);    
            DrawSync(0);                                
    }
}

void FntColor(CVECTOR fgcol, CVECTOR bgcol )
{
    // The debug font clut is at tx, ty + 128
    // tx = bg color
    // tx + 1 = fg color
    // We can override the color by drawing a rect at these coordinates
    // 
    // Define 1 pixel at 960,128 (background color) and 1 pixel at 961, 128 (foreground color)
    RECT fg = { FONTX+1, FONTY + 128, 1, 1 };
    RECT bg = { FONTX, FONTY + 128, 1, 1 };
    // Set colors
    ClearImage(&fg, fgcol.r, fgcol.g, fgcol.b);
    ClearImage(&bg, bgcol.r, bgcol.g, bgcol.b);
}

void init(void)
{
    ResetGraph(0);                 // Initialize drawing engine with a complete reset (0)
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);     // Set display area for both &disp[0] and &disp[1]
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // &disp[0] is on top  of &disp[1]
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // Set draw for both &draw[0] and &draw[1]
    SetDefDrawEnv(&draw[1], 0, 0         , SCREENXRES, SCREENYRES);     // &draw[0] is below &draw[1]
    if (VMODE)                  // PAL
    {
        SetVideoMode(MODE_PAL);
        disp[0].screen.y += 8;  // add offset : 240 + 8 + 8 = 256
        disp[1].screen.y += 8;
        }
    SetDispMask(1);                 // Display on screen    
    setRGB0(&draw[0], 50, 50, 50); // set color for first draw area
    setRGB0(&draw[1], 50, 50, 50); // set color for second draw area
    draw[0].isbg = 1;               // set mask for draw areas. 1 means repainting the area with the RGB color each frame 
    draw[1].isbg = 1;
    PutDispEnv(&disp[db]);          // set the disp and draw environnments
    PutDrawEnv(&draw[db]);
    FntLoad(960, 0);                // Load font to vram at 960,0(+128)
    FntOpen(MARGINX, MARGINY, SCREENXRES, SCREENXRES, 0, 280 ); // FntOpen(x, y, width, height,  black_bg, max. nbr. chars

}
void display(void)
{
    DrawSync(0);                    // Wait for all drawing to terminate
    VSync(0);                       // Wait for the next vertical blank
    PutDispEnv(&disp[db]);          // set alternate disp and draw environnments
    PutDrawEnv(&draw[db]);  
    db = !db;                       // flip db value (0 or 1)
}
int main(void)
{
    u_int t = 0;
    u_short step = 10;
    u_char * channel, * prevChannel;
    channel = &fntColor.r;
    prevChannel = &fntColor.b;
    init();                         // execute init()
    LoadTexture(_binary_fnt_tim_start, &fontface);
    FntColor(fntColor, fntColorBG);
    while (1)                       // infinite loop
    {   
        
        t++;
                
        if (fntColor.r >= 254 - step ){channel = &fntColor.g; prevChannel = &fntColor.r;}
        if (fntColor.g >= 254 - step ){channel = &fntColor.b; prevChannel = &fntColor.g;}
        if (fntColor.b >= 254 - step ){channel = &fntColor.r; prevChannel = &fntColor.b;}
            
        *channel += step;
        
        if (*prevChannel){
            *prevChannel -= step;
        }
        //~ fntColorBG.g = 255 - color ;
        FntColor(fntColor, fntColorBG);
        // 4lines, 16 glyphs on each line
        // Glyphs are 5x7 pixels wide in 16bpp, 
        FntPrint("!\"#$%%&'()*+,-./\n");
        FntPrint("0123456789:;<=^?\n");
        FntPrint("@ABCDEFGHIJKLMNO\n");
        FntPrint("PQRSTUVWXYZ[\\]>");

        FntPrint("\n\nHELLO DEBUG FONT! %d", t);
                                             
        FntFlush(-1);               // Draw print stream
        display();                  // Execute display()
    }
    return 0;
    }
