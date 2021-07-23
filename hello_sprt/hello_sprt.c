#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>

#define VMODE 0         // Video Mode : 0 : NTSC, 1: PAL

#define SCREENXRES 320
#define SCREENYRES 240

#define CENTERX SCREENXRES/2
#define CENTERY SCREENYRES/2

#define MARGINX 32      // margins for text display
#define MARGINY 44

#define FONTSIZE 8 * 3          // Text Field Height

#define OTLEN 8              // Ordering Table Length 

DISPENV disp[2];             // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];

u_long ot[2][OTLEN];     // double ordering table of length 8 * 32 = 256 bits / 32 bytes

char primbuff[2][32768] = {1};     // double primitive buffer of length 32768 * 8 =  262.144 bits / 32,768 Kbytes

char *nextpri = primbuff[0]; // pointer to the next primitive in primbuff. Initially, points to the first bit of primbuff[0]

short db = 0;                // index of which buffer is used, values 0, 1

// Embed TIM files

// See https://github.com/ABelliqueux/nolibgs_hello_worlds#embedding-binary-data-in-a-ps-exe

// 16bpp TIM
extern unsigned long _binary____TIM_TIM16_tim_start[];
extern unsigned long _binary____TIM_TIM16_tim_end[];
extern unsigned long _binary____TIM_TIM16_tim_length;

// 8bpp TIM
extern unsigned long _binary____TIM_TIM8_tim_start[];
extern unsigned long _binary____TIM_TIM8_tim_end[];
extern unsigned long _binary____TIM_TIM8_TIM_length;

// 4bpp TIM
extern unsigned long _binary____TIM_TIM4_tim_start[];
extern unsigned long _binary____TIM_TIM4_tim_end[];
extern unsigned long _binary____TIM_TIM4_tim_length;


TIM_IMAGE TIM_16;
TIM_IMAGE TIM_8;
TIM_IMAGE TIM_4;


void LoadTexture(u_long * tim, TIM_IMAGE * tparam){     // This part is from Lameguy64's tutorial series : lameguy64.net/svn/pstutorials/chapter1/3-textures.html login/pw: annoyingmous
        OpenTIM(tim);                                   // Open the tim binary data, feed it the address of the data in memory
        ReadTIM(tparam);                                // This read the header of the TIM data and sets the corresponding members of the TIM_IMAGE structure
        
        LoadImage(tparam->prect, tparam->paddr);        // Transfer the data from memory to VRAM at position prect.x, prect.y
        DrawSync(0);                                    // Wait for the drawing to end
        
        if (tparam->mode & 0x8){ // check 4th bit       // If 4th bit == 1, TIM has a CLUT
            LoadImage(tparam->crect, tparam->caddr);    // Load it to VRAM at position crect.x, crect.y
            DrawSync(0);                                // Wait for drawing to end
    }
}

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


int main(void)
{
    
    SPRT * sprt_16b;                // Define 3 pointers to SPRT struct
    SPRT * sprt_8b;
    SPRT * sprt_4b;
    
    DR_TPAGE * tpage_16b;           // Define 3 pointers to DR_TPAGE struct. We need three because our images are on three 
    DR_TPAGE * tpage_8b;            // different texture pages.
    DR_TPAGE * tpage_4b;
    
    init();
    
    LoadTexture(_binary____TIM_TIM16_tim_start, &TIM_16); // Load everything to vram
    LoadTexture(_binary____TIM_TIM8_tim_start, &TIM_8);
    LoadTexture(_binary____TIM_TIM4_tim_start, &TIM_4);
    
    while (1)
    {
        ClearOTagR(ot[db], OTLEN);
        
        // Loading a 16 bit TIM 
        
        sprt_16b = (SPRT *)nextpri;                 // Cast whats at nexpri as a SPRT named sprt_16b
                
        setSprt(sprt_16b);                          // Initialize the SPRT struct
        setRGB0(sprt_16b, 128, 128, 128);           // Set RGB color. 128,128,128 is neutral. You can color the image by adjusting these values
        setXY0(sprt_16b, 28, MARGINY);              // Set sprite position
        setWH(sprt_16b, 64, 128 );                  // Set sprite width and height

        addPrim(ot[db], sprt_16b);                  // add the sprite primitive to the ordering table
        
        nextpri += sizeof(SPRT);                    // increment nextpri so that it points just after sprt_16b in the primitive buffer
        
        // Set Texture page for the 16bit tim : 768, 0 - No CLUT
        
        // Note :  You need to use setDrawTPage each time you want to use a texture that's on a different texture page
        
        tpage_16b = (DR_TPAGE*)nextpri;
        
        setDrawTPage(tpage_16b, 0, 1,               // Set the Texture Page the texture we want resides on.
            getTPage(TIM_16.mode&0x3, 0,            // Here we are using bitmasking to deduce the picture mode : &0x3
            TIM_16.prect->x, TIM_16.prect->y));     // In binary, 3 is 11, so we only keep the first two bits
                                                    // Values can be 00 (0), 01 (1), 10(2), respectively, 4bpp, 8bpp, 15bpp, 24bpp. See Fileformat47.pdf, p.180
                                                    // Similarly, we could use bitmasking to deduce if there is a CLUT by bitmasking the 4th bit : if(TIM_IMAGE.mode & 0x8) LoadImage... :  
                                                    
        addPrim(ot[db], tpage_16b);                 // add the sprite primitive to the ordering table

        nextpri += sizeof(DR_TPAGE);                // Advance next primitive address
        
        // Loading a 8 bit TIM
        
        sprt_8b = (SPRT *)nextpri;
                
        setSprt(sprt_8b);
        setRGB0(sprt_8b, 128, 128, 128);        
        setXY0(sprt_8b, sprt_16b->x0 + sprt_16b->w + 32, MARGINY);
        setWH(sprt_8b, 64, 128 );
        setClut(sprt_8b, TIM_8.crect->x, TIM_8.crect->y);       // Only difference here is we set the CLUT to the position of the VRAM we loaded the palette earlier (see LoadTexture())
        
        addPrim(ot[db], sprt_8b);
        
        nextpri += sizeof(SPRT);
        
        // Set Texture page for the 8bit tim : 512, 256 - CLUT is at 0, 480
        
        tpage_8b = (DR_TPAGE*)nextpri;
        
        setDrawTPage(tpage_8b, 0, 1,               
            getTPage(TIM_8.mode&0x3, 0,            
            TIM_8.prect->x, TIM_8.prect->y));

        addPrim(ot[db], tpage_8b);                 
        nextpri += sizeof(DR_TPAGE);               
    
        // Loading a 4 bit TIM
        
        sprt_4b = (SPRT *)nextpri;
                
        setSprt(sprt_4b);
        setRGB0(sprt_4b, 128, 128, 128);        
        setXY0(sprt_4b, sprt_8b->x0 + sprt_8b->w + 32, MARGINY);
        setWH(sprt_4b, 64, 128 );
        setClut(sprt_4b, TIM_4.crect->x, TIM_4.crect->y);
        
        addPrim(ot[db], sprt_4b);
        
        nextpri += sizeof(SPRT);
        
        // Set Texture page for the 4bit tim : 512, 256 - CLUT is at 0, 480
        
        tpage_4b = (DR_TPAGE*)nextpri;
        
        setDrawTPage(tpage_4b, 0, 1,                
            getTPage(TIM_4.mode&0x3, 0,             
            TIM_4.prect->x, TIM_4.prect->y));

        addPrim(ot[db], tpage_4b);                  

        nextpri += sizeof(DR_TPAGE);                
    
        FntPrint("16 Bit!     ");
        FntPrint("8 Bit!      ");
        FntPrint("4 Bit!\n\n");
        FntPrint("Check VRAM in emu to see the dif");
    
        FntFlush(-1);
        
        display();
        }
    return 0;
    }
