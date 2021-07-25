// hello_sio example
// This example will display the RX data in a 64 char rolling buffer.
// Use minicom or any other serial comm program and a serial/USB cable.
//
// Relevant doc is libref47.pdf, l.1120-1127
// Schnappy - 04/2021
//
// Based on :  ../psyq/psx/sample/serial/SIO
// sio echo back 
// 1.00     Jan.28.1997 shino
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <stdio.h>
// Needed for SIO operations
#include <libsio.h>
// Needed for manipulating strings
#include <string.h>
// Display stuff (see hello_tile for the basics)
#define VMODE       0
#define SCREENXRES 320
#define SCREENYRES 240
#define CENTERX     SCREENXRES/2
#define CENTERY     SCREENYRES/2
#define OTLEN       2048        // Maximum number of OT entries
#define PRIMBUFFLEN 32768       // Maximum number of POLY_GT3 primitives
// Display and draw environments, double buffered
DISPENV disp[2];
DRAWENV draw[2];
u_long      ot[2][OTLEN];                   // Ordering table (contains addresses to primitives)
char    primbuff[2][PRIMBUFFLEN] = {0};     // Primitive list // That's our prim buffer
char * nextpri = primbuff[0];               // Primitive counter
short           db  = 0;                    // Current buffer counter
// SIO
#define MAX_CHARS 64
u_char SIO = 1;                             // Is SIO enabled ?
u_char SIOinit = 0;                         // Is SIO initialized ?
// Prototypes
void init(void);
void display(void);
void init(){
    // Reset the GPU before doing anything and the controller
    ResetGraph(0);
    // Set the display and draw environments
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[1], 0, 0, SCREENXRES, SCREENYRES);
    // If in PAL mode, add vertical offset
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
    FntOpen(16, 16, 196, 64, 0, 256);
    }
void display(void){
    DrawSync(0);
    VSync(0);
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    db = !db;
    }
int main() {
    init();
    // Main loop
    while (1) {
        // Buffer for the RX data of size MAX_CHARS
        static char buffer[ MAX_CHARS ] = {0};
        // If SIO flag is set, initialize and get data
        if( SIO ){
            // Is SIO is not initialized, dot it
            if( ! SIOinit ){
                ResetCallback();
                // Load SIO driver at 115200bps
                AddSIO(115200);
                ResetGraph(0);
                // Use _sio_control to clear driver status error-related bits
                // See psyq's libref47.pdf, p.1125 for the commands and status tables
                _sio_control(2,1,0);
                SIOinit = 1;
            }
            // Limit input buffer to MAX_CHARS chars, making it a rolling buffer
            if( strlen(buffer) > MAX_CHARS ){
               // If that limit is reached, remove first char in string
               memmove(buffer, buffer + 1, strlen(buffer));
            }
            // Check if sio driver is able to write communications data
            // If so, this means reading is not occuring
            if( _sio_control(0,0,0) & SR_RXRDY ){ // SR_RXRDY == 0x2
                // Read byte
                char c = _sio_control(0,4,0);
                // Add to buffer
                strncat(buffer, &c, 1);
            }
        }           
      // END SIO FUN
        FntPrint("Hello Serial!\n\n");
        if( buffer ){
            FntPrint("%s", buffer);
        }
        FntFlush(-1);
        display();
    }
    return 0;
}
