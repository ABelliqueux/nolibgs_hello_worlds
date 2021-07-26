// hello_libpad example
//
// We're using libpad this time.
// You can use the classic controller, analog, wheel, gun buttons or mouse
//
// Schnappy - 12/2020
//
// Based on :  ../psyq/addons/scea/CNTRL/PAD.C
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <libapi.h>
#define VMODE 0                  // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320
#define SCREENYRES 240
#define CENTERX SCREENXRES/2
#define CENTERY SCREENYRES/2
#define MARGINX 32               // margins for text display
#define MARGINY 32
#define FONTSIZE 8 * 7           // Text Field Height
#define OTLEN 8                  // Ordering Table Length 
DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
u_long ot[2][OTLEN];             // double ordering table of length 8 * 32 = 256 bits / 32 bytes
char primbuff[2][32768];   // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes
char *nextpri = primbuff[0];     // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]
short db = 0;                    // index of which buffer is used, values 0, 1
// Pad stuff
// Structure for RAW hardware-based light gun position values
typedef struct
{
    unsigned short    v_count;      // Y-axis (vertical scan counter)
    unsigned short    h_count;      // H-axis (horizontal pixel clock value)
} Gun_Position;
// Structure for storing processed controller data
typedef struct
{
    int             xpos, ypos;     // Stored position for sprite(s)
    int             xpos2, ypos2;   // controlled by this controller.
    unsigned char   status;         // These 8 values are obtained
    unsigned char   type;           // directly from the controller
    unsigned char   button1;        // buffer we installed with InitPAD.
    unsigned char   button2;
    unsigned char   analog0;
    unsigned char   analog1;
    unsigned char   analog2;
    unsigned char   analog3;
} Controller_Data;
// All-purpose controller data buffer
typedef struct
{
    unsigned char pad[34];          // 8-bytes w/o Multi-Tap, 34-bytes w/Multi-Tap
} Controller_Buffer;
Controller_Buffer controllers[2];   // Buffers for reading controllers
Controller_Data theControllers[8];  // Processed controller data
void init(void)
{
    ResetGraph(0);
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
    setRGB0(&draw[0], 50, 50, 50);
    setRGB0(&draw[1], 50, 50, 50);
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
    DrawOTag(&ot[db][OTLEN - 1]);
    db = !db;
    nextpri = primbuff[db];
}
void get_digital_direction( Controller_Data *c, int buttondata ) // get analog stick values
{
int i;
    i = ~(buttondata);
    if( i & 0x80 )
        c->xpos -= 1;
    if( i & 0x20 )
        c->xpos += 1;
    if( i & 0x40 )
        c->ypos += 1;
    if( i & 0x10 )
        c->ypos -= 1;
}
void read_controller( Controller_Data *c, unsigned char *buf, int port )  // get the raw values from controller
{
    register int mouse_x, mouse_y, x;
    register Gun_Position *g;
    c->status =  buf[0];    // Copy over raw controller data
    c->type =    buf[1];
    c->button1 = buf[2];
    c->button2 = buf[3];
    c->analog0 = buf[4];
    c->analog1 = buf[5];
    c->analog2 = buf[6];
    c->analog3 = buf[7];
    if( buf[0] == 0xff )    // If controller returns BAD status then bail on it.
    {
        c->type = 0;
        return;
    }
    // Look at the controller type code & process controller data as indicated
    switch( c->type )
    {
        case 0x12:      // Sony Mouse
            mouse_x = buf[4];
            mouse_y = buf[5];
            if( mouse_x & 0x80 )
                mouse_x |= 0xffffff80;
            if( mouse_y & 0x80 )
                mouse_y |= 0xffffff80;
            c->xpos += mouse_x;
            c->ypos += mouse_y;
            break;
        case 0x23:      // Namco negCon
                        // Steering wheel
                        // Sankyo Pachinko controler
            get_digital_direction( c, buf[2] );
            break;
        case 0x53:      // Analog 2-stick
            get_digital_direction( c, buf[2] );
            break;
        case 0x41:      // Standard Sony PAD controller
            get_digital_direction( c, buf[2] );
            break;
        default:        // If don't know what it is, treat it like standard controller
            get_digital_direction( c, buf[2] );
            break;
    }
}
int main(void)
{
    TILE * PADL;                    // Tile primitives
    TILE * TRIGGERL;
    TILE * PADR;
    TILE * TRIGGERR;
    TILE * START, * SELECT;
    init();
    InitPAD(controllers[0].pad, 34, controllers[1].pad, 34);
    StartPAD();
    while (1)
    {
        read_controller( &theControllers[0], &controllers[0].pad[0], 0 );  // Read controllers
        read_controller( &theControllers[1], &controllers[1].pad[0], 1 );
        ClearOTagR(ot[db], OTLEN);
        // D-cross
        PADL = (TILE *)nextpri;
        setTile(PADL);
        setRGB0(PADL, 80, 180, 255);        
        setXY0(PADL, CENTERX - 80, CENTERY);
        setWH(PADL, 24, 24);
        addPrim(ot[db], PADL);
        nextpri += sizeof(TILE);
        // L1+L2
        TRIGGERL = (TILE *)nextpri;
        setTile(TRIGGERL);
        setRGB0(TRIGGERL, 255, 0, 0);        
        setXY0(TRIGGERL, CENTERX - 80, CENTERY - 80);
        setWH(TRIGGERL, 24, 24);
        addPrim(ot[db], TRIGGERL);
        nextpri += sizeof(TILE);
        // /\, X, O, [] 
        PADR = (TILE *)nextpri;
        setTile(PADR);
        setRGB0(PADR, 0, 255, 0);        
        setXY0(PADR, CENTERX + 50, CENTERY);
        setWH(PADR, 24, 24);
        addPrim(ot[db], PADR);
        nextpri += sizeof(TILE);
        // R1+R2
        TRIGGERR = (TILE *)nextpri;
        setTile(TRIGGERR);
        setRGB0(TRIGGERR, 255, 0, 255);        
        setXY0(TRIGGERR, CENTERX + 50, CENTERY -80);
        setWH(TRIGGERR, 24, 24);
        addPrim(ot[db], TRIGGERR);
        nextpri += sizeof(TILE);
        // START + SELECT
        START = (TILE *)nextpri;
        setTile(START);
        setRGB0(START, 240, 240, 240);        
        setXY0(START, CENTERX - 16, CENTERY - 36);
        setWH(START, 24, 24);
        addPrim(ot[db], START);
        nextpri += sizeof(TILE);
        // D-pad
        switch(theControllers[0].button1){
            case 0xDF:                      // Right 
                PADL->x0 = CENTERX - 64;
                break;
            case 0x7F:                      // Left  
                PADL->x0 = CENTERX - 96;
                break;
            case 0xEF:                      // Up    
                PADL->y0 = CENTERY - 16;
                break;
            case 0xBF:                      // Down  
                PADL->y0 = CENTERY + 16;
                break;
            // Start & Select
            case 0xF7:
                START->w = 32; START->h = 32;START->x0 -= 4;START->y0 -= 4; // START
                break;
            case 0xFE:                                                      // SELECT
                START->r0 = 0;
                break;
            // Dualshock L3 + R3
            case 0xFD:                      // L3
                TRIGGERL->w += 10;
                TRIGGERL->h += 10;
                break;
            case 0xFB:                      //R3
                TRIGGERR->w += 10;
                TRIGGERR->h += 10;
                break;
        }
        // Buttons
        switch(theControllers[0].button2){
            case 0xDF:                      // ⭘
                PADR->x0 = CENTERX + 66;
                break;
            case 0x7F:                      // ⬜
                PADR->x0 = CENTERX + 34;
                break;
            case 0xEF:                      // △
                PADR->y0 = CENTERY - 16;
                break;
            case 0xBF:                      // ╳
                PADR->y0 = CENTERY + 16;
                break;
        // Shoulder buttons             
            case 0xFB:                       // L1
                TRIGGERL->y0 = CENTERY - 64;
                break;
            case 0xFE:                       // L2
                TRIGGERL->y0 = CENTERY - 96;
                break;
            case 0xF7:                       // R1
                TRIGGERR->y0 = CENTERY - 96;
                break;
            case 0xFD:                       // R2
                TRIGGERR->y0 = CENTERY - 64;
                break;
        // Mouse buttons 
            case 0xF4:                      // Mouse Left click
                PADL->w += 10;
                PADL->h += 10;
                break;
            case 0xF8:                      // Mouse Right click
                PADL->w -= 10; 
                PADL->h -= 10; 
                break;
        }
        FntPrint("Hello 2 pads!\n\n");
        FntPrint( "Pad 1 : %02x\nButtons:%02x %02x, Stick:%02x %02x %02x %02x\n",
                    theControllers[0].type,             // Controller type : 00 == none,  41 == standard, 73 == analog/dualshock, 12 == mouse, 23 == steering wheel, 63 == gun, 53 == analog joystick
                    theControllers[0].button1,          // 
                    theControllers[0].button2,
                    theControllers[0].analog0,
                    theControllers[0].analog1,
                    theControllers[0].analog2,
                    theControllers[0].analog3 );
        FntPrint( "Pad 2 : %02x\nButtons:%02x %02x, Stick:%02x %02x %02x %02x\n",
                    theControllers[1].type,             // Controller type : 00 == none,  41 == standard, 73 == analog/dualshock, 12 == mouse, 23 == steering wheel, 63 == gun, 53 == analog joystick
                    theControllers[1].button1,          // 
                    theControllers[1].button2,
                    theControllers[1].analog0,          // R3 horizontal
                    theControllers[1].analog1,          // R3 vertical
                    theControllers[1].analog2,          // L3 horizontal
                    theControllers[1].analog3 );        // L3 vertical
        FntFlush(-1);
        display();
        }
    return 0;
    }
