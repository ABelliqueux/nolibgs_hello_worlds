// Load a BS file from CD, decompress and display it.
// Schnappy 07-2021
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
// CD library
#include <libcd.h>
// CODEC library
#include <libpress.h>
#include <malloc.h>


#define VMODE 0                 // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320          // Screen width
#define SCREENYRES 240 + (VMODE << 4)          // Screen height : If VMODE is 0 = 240, if VMODE is 1 = 256 
#define CENTERX SCREENXRES/2    // Center of screen on x 
#define CENTERY SCREENYRES/2    // Center of screen on y
#define MARGINX 8                // margins for text display
#define MARGINY 16
#define FONTSIZE 8 * 7           // Text Field Height
DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
short db = 0;                      // index of which buffer is used, values 0, 1

// CD specifics
#define CD_SECTOR_SIZE 2048
// Converting bytes to sectors SECTOR_SIZE is defined in words, aka int
#define BtoS(len) ( ( len + CD_SECTOR_SIZE - 1 ) / CD_SECTOR_SIZE ) 
// Name of file to load
static char * loadFile;
// libcd's CD file structure contains size, location and filename
CdlFILE filePos = {0};
//~ struct EXEC * exeStruct;
// Define start address of allocated memory
// Let's use an array so we don't have to worry about using a memory segment that's already in use.
static unsigned char ramAddr[0x40000]; // https://discord.com/channels/642647820683444236/663664210525290507/864936962199781387
// We could also set a memory address manually, but we have to make sure this won't get in the way of other routines.
// void * ramAddr = (void *)0x80030D40; 
// Load data to this buffer
u_long * dataBuffer;              
// Those are not strictly needed, but we'll use them to see the commands results.
// They could be replaced by a 0 in the various functions they're used with.
u_char CtrlResult[8];
// Value returned by CDread() - 1 is good, 0 is bad
int CDreadOK = 0;
// Value returned by CDsync() - Returns remaining sectors to load. 0 is good.
int CDreadResult = 0;
// BS decompression
// Store size of uncompressed data
long bsBufferSize;
// Allocated memory address
void * bsWorkBuffer;
// Define image draw area
RECT bsDrawArea = { 0, 0, 16, SCREENYRES};
// Used to store a 16x240 image strip
u_long bsStrip[16*240];

void init(void)
{
    ResetGraph(0);                 // Initialize drawing engine with a complete reset (0)
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);     // Set display area for both &disp[0] and &disp[1]
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // &disp[0] is on top  of &disp[1]
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // Set draw for both &draw[0] and &draw[1]
    SetDefDrawEnv(&draw[1], 0, 0         , SCREENXRES, SCREENYRES);     // &draw[0] is below &draw[1]
    // Set video mode
    if (VMODE){ SetVideoMode(MODE_PAL);}
    SetDispMask(1);                 // Display on screen    
    setRGB0(&draw[0], 155, 0, 150); // set color for first draw area
    setRGB0(&draw[1], 155, 0, 150); // set color for second draw area
    draw[0].isbg = 0;               // set mask for draw areas. 1 means repainting the area with the RGB color each frame 
    draw[1].isbg = 0;
    PutDispEnv(&disp[db]);          // set the disp and draw environnments
    PutDrawEnv(&draw[db]);
    FntLoad(960, 0);                // Load font to vram at 960,0(+128)
    FntOpen(MARGINX, MARGINY, SCREENXRES - MARGINX * 2, FONTSIZE, 0, 280 ); // FntOpen(x, y, width, height,  black_bg, max. nbr. chars
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
    // Init display
    init();
    // Init CD system
    CdInit();
    // Init heap
    InitHeap((u_long *)ramAddr, sizeof(ramAddr));
    // If the other method was chosen at l.39
    // InitHeap((void *)0x80030D40, 0x40000);
    // Set name of file to load
    loadFile = "\\BACE.BS;1";
    // Get file position from filename
    CdSearchFile( &filePos, loadFile);
    // Allocate memory
    dataBuffer = malloc( BtoS(filePos.size) * CD_SECTOR_SIZE );
    // Issue  CdlSetloc CDROM command : Set the seek target position
    // Beware of a misnomed 'sector' member in the CdlLOC struct that should really be named 'frame'.
    // https://discord.com/channels/642647820683444236/663664210525290507/864912470996942910
    CdControl(CdlSetloc, (u_char *)&filePos.pos, CtrlResult);
    // Read data and load it to dataBuffer
    CDreadOK = CdRead( (int)BtoS(filePos.size), (u_long *)dataBuffer, CdlModeSpeed );
    // Wait for operation to complete
    CDreadResult = CdReadSync(0, 0);

    // Image Decompression
    // Initialize image processing subsystem
    DecDCTReset(0);
    // Find the needed buffer size
    bsBufferSize = DecDCTBufSize(dataBuffer);
    // Allocate buffer size at &bsWorkBuffer
    bsWorkBuffer = malloc( bsBufferSize );
    // Decode Huffman (also called VLC: variable length coding ) compressed image 
    DecDCTvlc( dataBuffer, (u_long *) bsWorkBuffer );
    // Send decoded data to MDEC for RLE decoding.
    DecDCTin( (u_long*) bsWorkBuffer, 0);
    // Fetch decoded image in 16x240 strips
    for( bsDrawArea.x = 0; bsDrawArea.x < SCREENXRES; bsDrawArea.x += 16 ){
        // Request decoded data from MDEC
        // Request 16 * 240 pixel high lines.
        // But size is in long words (4B), so divide by 2 to get words (2B) ?
        DecDCTout( bsStrip,  (16*SCREENYRES)/2);
        // Wait for transfer to complete 
        DecDCToutSync(0);
        // Load image data to fb
        LoadImage( &bsDrawArea, bsStrip );
    }
    free( bsWorkBuffer );
    while (1)  // infinite loop
    {   
        // Copy BS image to the other buffer
        MoveImage2(&disp[db].disp, 0, disp[!db].disp.y );
        DrawSync(0);
        FntPrint("Hello BS! \n");
        // Print filesize in bytes/sectors
        FntPrint("Bs Size: %dB sectors: %d\n", filePos.size, BtoS(filePos.size));
        // Print heap and buffer addresses
        FntPrint("Heap: %x - Buf: %x\n", ramAddr, dataBuffer);
        FntPrint("bsWork: %x\nbsBufSize: %dB\n", bsWorkBuffer, bsBufferSize);        
        
        FntFlush(-1);               // Draw print stream
        display();                  // Execute display()
    }
    return 0;
    }
