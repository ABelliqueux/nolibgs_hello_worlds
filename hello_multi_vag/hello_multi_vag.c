// Hello multi vag by Schnappy
// August 2021
//
//
// Load VAG samples to the SPU sound buffer and play them back with pad input
//
// See here for more details, file format infos, tools, conversion :
//        https://github.com/ABelliqueux/nolibgs_hello_worlds/wiki/VAG
// Docs : see libformat47.pdf p.209
//            libover47.pdf,  p.271
//            libref47.pdf,   p.980

#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
// Sound system
#include <libsnd.h>
#include <libspu.h>
#define VMODE 0                 // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320
#define SCREENYRES 240
#define CENTERX SCREENXRES/2
#define CENTERY SCREENYRES/2
#define MARGINX 0               // margins for text display
#define MARGINY 32
#define FONTSIZE 8 * 7          // Text Field Height
DISPENV disp[2];                // Double buffered DISPENV and DRAWENV
DRAWENV draw[2];
short db = 0;                   // index of which buffer is used, values 0, 1
// Number of VAG files to load
#define VAG_NBR 8
#define MALLOC_MAX VAG_NBR            // Max number of time we can call SpuMalloc
// convert Little endian to Big endian
#define SWAP_ENDIAN32(x) (((x)>>24) | (((x)>>8) & 0xFF00) | (((x)<<8) & 0x00FF0000) | ((x)<<24))
// Memory management table ; allow MALLOC_MAX calls to SpuMalloc() - libref47.pdf p.1044
char spu_malloc_rec[SPU_MALLOC_RECSIZ * (MALLOC_MAX + 1)]; 
// Custom struct to handle VAG files
typedef struct VAGsound {
    u_char * VAGfile;        // Pointer to VAG data address
    u_long spu_channel;      // SPU voice to playback to
    u_long spu_address;      // SPU address for memory freeing spu mem
    } VAGsound;
// VAG header struct (see fileformat47.pdf, p.209)
typedef struct VAGhdr {                // All the values in this header must be big endian
        char id[4];                    // VAGp         4 bytes -> 1 char * 4
        unsigned int version;          // 4 bytes
        unsigned int reserved;         // 4 bytes
        unsigned int dataSize;         // (in bytes) 4 bytes
        unsigned int samplingFrequency;// 4 bytes
        char  reserved2[12];           // 12 bytes -> 1 char * 12
        char  name[16];                // 16 bytes -> 1 char * 16
        // Waveform data after that
} VAGhdr;
// SPU settings
SpuCommonAttr commonAttributes;          // structure for changing common voice attributes
SpuVoiceAttr  voiceAttributes ;          // structure for changing individual voice attributes                       
// extern VAG files
extern u_char _binary____VAG_0_come_vag_start;
extern u_char _binary____VAG_1_cuek_vag_start;
extern u_char _binary____VAG_2_erro_vag_start;
extern u_char _binary____VAG_3_hehe_vag_start;
extern u_char _binary____VAG_4_m4a1_vag_start;
extern u_char _binary____VAG_5_punc_vag_start;
extern u_char _binary____VAG_7_wron_vag_start;
extern u_char _binary____VAG_8_yooo_vag_start;
// soundBank
VAGsound soundBank[VAG_NBR] = {
      { &_binary____VAG_0_come_vag_start,
        SPU_00CH, 0 },  
      { &_binary____VAG_1_cuek_vag_start,
        SPU_01CH, 0 },   
      { &_binary____VAG_2_erro_vag_start,
        SPU_02CH, 0 },   
      { &_binary____VAG_3_hehe_vag_start,
        SPU_03CH, 0 },   
      { &_binary____VAG_4_m4a1_vag_start,
        SPU_04CH, 0 },  
      { &_binary____VAG_5_punc_vag_start,
        SPU_05CH, 0 },   
      { &_binary____VAG_7_wron_vag_start,
        SPU_06CH, 0 },   
      { &_binary____VAG_8_yooo_vag_start,
        SPU_07CH, 0 }
};
// Prototypes
void initGraph(void);
void display(void);
void initSnd(void);
u_long sendVAGtoSPU(unsigned int VAG_data_size, u_char *VAG_data);
void setVoiceAttr(unsigned int pitch, long channel, unsigned long soundAddr );
u_long setSPUtransfer(VAGsound * sound);
void playSFX(VAGsound *  sound);

void initGraph(void)
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
    FntOpen(8, 60, 304, 200, 0, 500 );
}
void display(void)
{
    DrawSync(0);
    VSync(0);
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    db = !db;
}
// Audio initialisation & functions
void initSnd(void){
    SpuInitMalloc(MALLOC_MAX, spu_malloc_rec);                      // Maximum number of blocks, mem. management table address.
    commonAttributes.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR);  // Mask which attributes to set
    commonAttributes.mvol.left  = 0x3fff;                           // Master volume left
    commonAttributes.mvol.right = 0x3fff;                           // see libref47.pdf, p.1058
    SpuSetCommonAttr(&commonAttributes);                            // set attributes
    SpuSetIRQ(SPU_OFF);
    SpuSetKey(SpuOff, SPU_ALLCH);
}
u_long sendVAGtoSPU(unsigned int VAG_data_size, u_char *VAG_data){
    u_long transferred;
    SpuSetTransferMode(SpuTransByDMA);                              // DMA transfer; can do other processing during transfer
    transferred = SpuWrite (VAG_data + sizeof(VAGhdr), VAG_data_size);     // transfer VAG_data_size bytes from VAG_data  address to sound buffer
    SpuIsTransferCompleted (SPU_TRANSFER_WAIT);                     // Checks whether transfer is completed and waits for completion
    return transferred;
}
void setVoiceAttr(unsigned int pitch, long channel, unsigned long soundAddr ){
    voiceAttributes.mask=                                   //~ Attributes (bit string, 1 bit per attribute)
    (
      SPU_VOICE_VOLL |
      SPU_VOICE_VOLR |
      SPU_VOICE_PITCH |
      SPU_VOICE_WDSA |
      SPU_VOICE_ADSR_AMODE |
      SPU_VOICE_ADSR_SMODE |
      SPU_VOICE_ADSR_RMODE |
      SPU_VOICE_ADSR_AR |
      SPU_VOICE_ADSR_DR |
      SPU_VOICE_ADSR_SR |
      SPU_VOICE_ADSR_RR |
      SPU_VOICE_ADSR_SL
    );
    voiceAttributes.voice        = channel;                 //~ Voice (low 24 bits are a bit string, 1 bit per voice )
    voiceAttributes.volume.left  = 0x0;                  //~ Volume 
    voiceAttributes.volume.right = 0x0;                  //~ Volume
    voiceAttributes.pitch        = pitch;                   //~ Interval (set pitch)
    voiceAttributes.addr         = soundAddr;               //~ Waveform data start address
    voiceAttributes.a_mode       = SPU_VOICE_LINEARIncN;    //~ Attack rate mode  = Linear Increase - see libref47.pdf p.1091
    voiceAttributes.s_mode       = SPU_VOICE_LINEARIncN;    //~ Sustain rate mode = Linear Increase
    voiceAttributes.r_mode       = SPU_VOICE_LINEARDecN;    //~ Release rate mode = Linear Decrease
    voiceAttributes.ar           = 0x0;                     //~ Attack rate
    voiceAttributes.dr           = 0x0;                     //~ Decay rate
    voiceAttributes.rr           = 0x0;                     //~ Release rate
    voiceAttributes.sr           = 0x0;                     //~ Sustain rate
    voiceAttributes.sl           = 0xf;                     //~ Sustain level
    SpuSetVoiceAttr(&voiceAttributes);                      // set attributes
}
u_long setSPUtransfer(VAGsound * sound){
    // Return spu_address
    u_long transferred, spu_address;
    u_int pitch;
    const VAGhdr * VAGheader = (VAGhdr *) sound->VAGfile;
    pitch = (SWAP_ENDIAN32(VAGheader->samplingFrequency) << 12) / 44100L; 
    spu_address = SpuMalloc(SWAP_ENDIAN32(VAGheader->dataSize));                // Allocate an area of dataSize bytes in the sound buffer. 
    SpuSetTransferStartAddr(spu_address);                                       // Sets a starting address in the sound buffer
    transferred = sendVAGtoSPU(SWAP_ENDIAN32(VAGheader->dataSize), sound->VAGfile);
    setVoiceAttr(pitch, sound->spu_channel, spu_address); 
    // Return 1 if ok, size transferred else.
    //~ if (transferred == SWAP_ENDIAN32(VAGheader->dataSize)){
        //~ return 1;
        //~ }
    //~ return transferred;
    return spu_address;
}
void playSFX(VAGsound *  sound){
    // Set voice volume to max
    voiceAttributes.mask= ( SPU_VOICE_VOLL | SPU_VOICE_VOLR );
    voiceAttributes.voice        = sound->spu_channel;
    voiceAttributes.volume.left  = 0x1000;
    voiceAttributes.volume.right = 0x1000;
    SpuSetVoiceAttr(&voiceAttributes);
    // Play voice
    SpuSetKey(SpuOn, sound->spu_channel);
}

int main(void)
{
    // Store input values
    int pad, oldpad;
    u_long spu_address;
    // Init all the things !
    initGraph();
    // Init Spu
    SpuInit();
    // Init sound settings
    initSnd();
    // Init Pad
    PadInit(0);   
    // Transfer all VAGs to SPU RAM
    // Beware that the SPU only has 512KB of RAM so a 'soundbank' should stay within that limit.
    // If more is needed, a second soundbank could be loaded on demand.
    for (u_short vag = 0; vag < VAG_NBR; vag++ ){
        soundBank[vag].spu_address = setSPUtransfer(&soundBank[vag]);
    }
    while (1)
    {
        // Pad 
        // Use up, down, left, right, triangle, cross, circle, square to play various samples
        // Read pad values
        pad = PadRead(0);
        
        if (pad & PADLleft && !(oldpad & PADLleft)){
            playSFX(&soundBank[0]);
            oldpad = pad;
            }
        if (pad & PADLright && !(oldpad & PADLright)){
            playSFX(&soundBank[1]);
            oldpad = pad;
            }
        if (pad & PADLup && !(oldpad & PADLup)){
            playSFX(&soundBank[2]);
            oldpad = pad;
            }
        if (pad & PADLdown && !(oldpad & PADLdown)){
            playSFX(&soundBank[3]);
            oldpad = pad;
            }
        if (pad & PADRleft && !(oldpad & PADRleft)){
            playSFX(&soundBank[4]);
            oldpad = pad;
            }
            
        if (pad & PADRright && !(oldpad & PADRright)){
            playSFX(&soundBank[5]);
            oldpad = pad;
            }
        if (pad & PADRup && !(oldpad & PADRup)){
            playSFX(&soundBank[6]);
            oldpad = pad;
            }
        if (pad & PADRdown && !(oldpad & PADRdown)){
            playSFX(&soundBank[7]);
            oldpad = pad;
            }
        // Reset oldpad    
        if (!(pad & PADRdown) ||
            !(pad & PADRup)   ||
            !(pad & PADRleft) ||
            !(pad & PADRright) ||
            !(pad & PADLdown) ||
            !(pad & PADLup)   ||
            !(pad & PADLleft) ||
            !(pad & PADLright)
            ){ oldpad = pad; }
        
        FntPrint("Hello multi vag ! %d\n", VSync(-1));
        FntPrint("Use the pad to play various samples.\n");
        FntFlush(-1);
        display();
    }
    return 0;
}
