// Play a MOD file converted to HIT 
// MOD Wiki page : https://github.com/ABelliqueux/nolibgs_hello_worlds/wiki/MOD
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
// Mod playback
#include "src/mod.h"

#define VMODE 0                 // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320          // Screen width
#define SCREENYRES 240          // Screen height : If VMODE is 0 = 240, if VMODE is 1 = 256 
#define CENTERX SCREENXRES/2    // Center of screen on x 
#define CENTERY SCREENYRES/2    // Center of screen on y
#define FONTX   960
#define FONTY   0
#define OTLEN 8 
DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
short db = 1;                    // index of which buffer is used, values 0, 1

// Font color
CVECTOR fntColor = { 128, 255, 0 };
CVECTOR fntColorBG = { 0, 0, 0 };

// Playback state
enum PLAYBACK {
    STOP  = 0,
    PLAY  = 1,
    PAUSE = 2,
};

enum PLAYBACK state = PLAY;

void init(void);
void FntColor(CVECTOR fgcol, CVECTOR bgcol );
void display(void);
void drawBG(void);
void checkPad(void);

void FntColor(CVECTOR fgcol, CVECTOR bgcol )
{
    // The debug font clut is at tx, ty + 128
    // tx = bg color
    // tx + 1 = fg color
    // We can override the color by drawing a rect at these coordinates
    // 
    RECT fg = { FONTX+1, FONTY + 128, 1, 1 };
    RECT bg = { FONTX, FONTY + 128, 1, 1 };
    ClearImage(&fg, fgcol.r, fgcol.g, fgcol.b);
    ClearImage(&bg, bgcol.r, bgcol.g, bgcol.b);
}

void init(void)
{
    ResetCallback();
    ResetGraph(0);                 // Initialize drawing engine with a complete reset (0)
    InitGeom();
    SetGeomOffset(CENTERX,CENTERY);
    SetGeomScreen(CENTERX);
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);     // Set display area for both &disp[0] and &disp[1]
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // &disp[0] is on top  of &disp[1]
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // Set draw for both &draw[0] and &draw[1]
    SetDefDrawEnv(&draw[1], 0, 0         , SCREENXRES, SCREENYRES);     // &draw[0] is below &draw[1]
    // Set video mode
    #if VMODE
        SetVideoMode(MODE_PAL);
        disp[0].disp.y = 8;
        disp[1].disp.y = 8;
    #endif
    SetDispMask(1);                 // Display on screen    
    setRGB0(&draw[0], 40, 40, 40); // set color for first draw area
    setRGB0(&draw[1], 40, 40, 40); // set color for second draw area
    draw[0].isbg = 1;               // set mask for draw areas. 1 means repainting the area with the RGB color each frame 
    draw[1].isbg = 1;
    PutDispEnv(&disp[db]);          // set the disp and draw environnments
    PutDrawEnv(&draw[db]);
    FntLoad(FONTX, FONTY);                // Load font to vram at 960,0(+128)
    FntOpen(32, 64, 260, 120, 0, 120 ); // FntOpen(x, y, width, height,  black_bg, max. nbr. chars
    FntColor(fntColor, fntColorBG);
}
void display(void)
{
    DrawSync(0);                    // Wait for all drawing to terminate
    VSync(0);                       // Wait for the next vertical blank
    PutDispEnv(&disp[db]);          // set alternate disp and draw environnments
    PutDrawEnv(&draw[db]);  
    db = !db;
}

void checkPad(void)
{
    u_short pad = 0;
    static u_short oldPad;    
    pad = PadRead(0);

    // Up
    if ( pad & PADLup && !(oldPad & PADLup) )
    {
        MOD_PlayNote(11, 25, 15, 63);
        oldPad = pad;
    }
    if ( !(pad & PADLup) && oldPad & PADLup )
    {
        oldPad = pad;
    }
    // Down
    if ( pad & PADLdown && !(oldPad & PADLdown) )
    {
        MOD_PlayNote(12, 26, 15, 63);
        oldPad = pad;
    }
    if ( !(pad & PADLdown) && oldPad & PADLdown )
    {
        oldPad = pad;
    }
    // Left
    if ( pad & PADLleft && !(oldPad & PADLleft) )
    {
        MOD_PlayNote(13, 27, 15, 63);
        oldPad = pad;
    }
    if ( !(pad & PADLleft) && oldPad & PADLleft )
    {
        oldPad = pad;
    }
    // Right
    if ( pad & PADLright && !(oldPad & PADLright) )
    {
        // Channel 1 is transition anim, only take input when !transition
        MOD_PlayNote(6, 21, 15, 63);
        oldPad = pad;
    }
    if ( !(pad & PADLright) && oldPad & PADLright )
    {
        oldPad = pad;
    }
    // Cross button
    if ( pad & PADRdown && !(oldPad & PADRdown) )
    {
        // Select sound
        MOD_PlayNote(7, 22, 15, 63);
        oldPad = pad;
    }
    if ( !(pad & PADRdown) && oldPad & PADRdown )
    {
        oldPad = pad;
    }
    // Square button
    if ( pad & PADRleft && !(oldPad & PADRleft) )
    {
        // Select sound
        MOD_PlayNote(8, 23, 15, 63);
        oldPad = pad;
    }
    if ( !(pad & PADRleft) && oldPad & PADRleft )
    {
        oldPad = pad;
    }
    // Circle button
    if ( pad & PADRright && !(oldPad & PADRright) )
    {
        // Select sound
        MOD_PlayNote(9, 28, 15, 63);
        oldPad = pad;
    }
    if ( !(pad & PADRright) && oldPad & PADRright )
    {
        oldPad = pad;
    }
    // Circle button
    if ( pad & PADRup && !(oldPad & PADRup) )
    {
        // Select sound
        MOD_PlayNote(9, 24, 15, 63);
        oldPad = pad;
    }
    if ( !(pad & PADRup) && oldPad & PADRup )
    {
        oldPad = pad;
    }
    // Select button
    if ( pad & PADselect && !(oldPad & PADselect) )
    {
        if ( state == PLAY ) { stopMusic(); state = STOP; }
        else if ( state == STOP ) { loadMod();startMusic(); state = PLAY; }
        oldPad = pad;
    }
    if ( !(pad & PADselect) && oldPad & PADselect )
    {
        oldPad = pad;
    }
    // Start button
    if ( pad & PADstart && !(oldPad & PADstart) )
    {
        if ( state == PLAY ) { pauseMusic(); state = PAUSE; }
        else if ( state == PAUSE ) { resumeMusic(); state = PLAY; }
        oldPad = pad;
    }
    if ( !(pad & PADstart) && oldPad & PADstart )
    {
        oldPad = pad;
    }
    
}
int main() {
    u_int t = 0;
    init();
    PadInit(0);
    VSyncCallback(checkPad);
    // Mod Playback
    loadMod();
    startMusic();
    // Main loop
    while (1) 
    {
        // TODO: change volume
        t++;
        FntPrint("Hello mod ! %d\nUse pad buttons to play sounds.\n", t);
        FntPrint("State: %d\n", state);
        FntPrint("Start : play/pause music.\n");
        FntFlush(-1);
        display();
    }
    return 0;
}

