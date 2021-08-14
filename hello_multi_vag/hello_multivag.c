// VAGDEMO2020 by Schnappy
// December 2020
// Based on VAGDEMO_FIXED by Yagotzirck
// Based on VAGDEMO by Shadow
// based on psyq/addons/sound/TUTO3.C
//
//
// Load two VAG file to SPU sound buffer and play them back alternatively or simultaneously.
//
// WAV creation: use ffmpeg to create a 16-bit ADPCM mono WAV file - change -ar to reduce filesize (and quality)
// $ ffmpeg -i input.ext -f s16le -ac 1 -ar 44100 tmp.dat
//
// WAV to VAG convertion using WAV2VAG : https://github.com/ColdSauce/psxsdk/blob/master/tools/wav2vag.c
// change -freq according to the -ar setting above 
// $ wav2vag tmp.dat output.vag -sraw16 -freq=44100 (-L) 
//
// Alternatively, you can use PsyQ VAGEDIT.EXE to change the sampling frequency of an existing VAG file.
//
// Docs : see libformat47.pdf p.209
//            libover47.pdf,  p.271
//            libref47.pdf,   p.980
// URLS : http://psx.arthus.net/code/VAG/
//        https://github.com/ABelliqueux/nolibgs_hello_worlds/tree/main/VAG
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
// Sound stuff
#define MALLOC_MAX 3            // Max number of time we can call SpuMalloc
//~ // convert Little endian to Big endian
#define SWAP_ENDIAN32(x) (((x)>>24) | (((x)>>8) & 0xFF00) | (((x)<<8) & 0x00FF0000) | ((x)<<24))
typedef struct VAGheader{       // All the values in this header must be big endian
        char id[4];             // VAGp         4 bytes -> 1 char * 4
        unsigned int version;          // 4 bytes
        unsigned int reserved;         // 4 bytes
        unsigned int dataSize;         // (in bytes) 4 bytes
        unsigned int samplingFrequency;// 4 bytes
        char  reserved2[12];    // 12 bytes -> 1 char * 12
        char  name[16];         // 16 bytes -> 1 char * 16
        // Waveform data after that
} VAGhdr;
SpuCommonAttr commonAttributes;          // structure for changing common voice attributes
SpuVoiceAttr  voiceAttributes ;          // structure for changing individual voice attributes
u_long hello_spu_address;                  // address allocated in memory for first sound file
u_long poly_spu_address;                 // address allocated in memory for second sound file
// DEBUG : these allow printing values for debugging
u_long hello_spu_start_address;                
u_long hello_get_start_addr;
u_long hello_transSize;                            
u_long poly_spu_start_address;                
u_long poly_get_start_addr;
u_long poly_transSize;                            
#define HELLO SPU_0CH                   // Play first vag on channel 0
#define POLY SPU_2CH                    // Play second vag on channel 2
// Memory management table ; allow MALLOC_MAX calls to SpuMalloc() - ibref47.pdf p.1044
char spu_malloc_rec[SPU_MALLOC_RECSIZ * (2 + MALLOC_MAX+1)]; 
// VAG files
// We're using GrumpyCoder's Nugget wrapper to compile the code with a modern GCC : https://github.com/grumpycoders/pcsx-redux/tree/main/src/mips/psyq
// To include binary files in the exe, add your VAG files to the SRCS variable in Makefile
// and in common.mk, add this rule to include *.vag files :
//
//~ %.o: %.vag
    //~ $(PREFIX)-objcopy -I binary --set-section-alignment .data=4 --rename-section .data=.rodata,alloc,load,readonly,data,contents -O elf32-tradlittlemips -B mips $< $@
// hello.vag - 44100 Khz
extern unsigned char _binary____VAG_hello_vag_start[]; // filename must begin with _binary____ followed by the full path, with . and / replaced, and then suffixed with _ and end with _start[]; or end[];
extern unsigned char _binary____VAG_hello_vag_end[];   // https://discord.com/channels/642647820683444236/663664210525290507/780866265077383189
// poly.vag - 44100 Khz
extern unsigned char _binary____VAG_poly_vag_start[];
extern unsigned char _binary____VAG_poly_vag_end[];
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
}
u_long sendVAGtoRAM(unsigned int VAG_data_size, unsigned char *VAG_data){
    u_long size;
    SpuSetTransferMode(SpuTransByDMA);                              // DMA transfer; can do other processing during transfer
    size = SpuWrite (VAG_data + sizeof(VAGhdr), VAG_data_size);     // transfer VAG_data_size bytes from VAG_data  address to sound buffer
    SpuIsTransferCompleted (SPU_TRANSFER_WAIT);                     // Checks whether transfer is completed and waits for completion
    return size;
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
    voiceAttributes.volume.left  = 0x1000;                  //~ Volume 
    voiceAttributes.volume.right = 0x1000;                  //~ Volume
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
void playSFX(unsigned long fx){
    SpuSetKey(SpuOn, fx); 
}
int main(void)
{
    short counter = 0;
    const VAGhdr * HellofileHeader = (VAGhdr *) _binary____VAG_hello_vag_start;   // get header of first VAG file
    const VAGhdr * PolyfileHeader = (VAGhdr *) _binary____VAG_poly_vag_start;   // get header of second VAG file
    // From libover47.pdf :
    // The sampling frequency of the original audio file can be used to determine the pitch
    // at which to play the VAG. pitch = (sampling frequency << 12)/44100L 
    // Ex: 44.1kHz=0x1000 22.05kHz=0x800 etc
    unsigned int Hellopitch =   (SWAP_ENDIAN32(HellofileHeader->samplingFrequency) << 12) / 44100L; 
    unsigned int Polypitch =   (SWAP_ENDIAN32(PolyfileHeader->samplingFrequency) << 12) / 44100L; 
    SpuInit();                                                                            // Initialize SPU. Called only once.
    initSnd();
    // First VAG
    hello_spu_address   = SpuMalloc(SWAP_ENDIAN32(HellofileHeader->dataSize));                // Allocate an area of dataSize bytes in the sound buffer. 
    hello_spu_start_address = SpuSetTransferStartAddr(hello_spu_address);                         // Sets a starting address in the sound buffer
    hello_get_start_addr    = SpuGetTransferStartAddr();                                        // SpuGetTransferStartAddr() returns current sound buffer transfer start address.
    hello_transSize         = sendVAGtoRAM(SWAP_ENDIAN32(HellofileHeader->dataSize), _binary____VAG_hello_vag_start);
     // First VAG
    poly_spu_address   = SpuMalloc(SWAP_ENDIAN32(PolyfileHeader->dataSize));                // Allocate an area of dataSize bytes in the sound buffer. 
    poly_spu_start_address = SpuSetTransferStartAddr(poly_spu_address);                         // Sets a starting address in the sound buffer
    poly_get_start_addr    = SpuGetTransferStartAddr();                                        // SpuGetTransferStartAddr() returns current sound buffer transfer start address.
    poly_transSize         = sendVAGtoRAM(SWAP_ENDIAN32(PolyfileHeader->dataSize), _binary____VAG_poly_vag_start);
    // set VAG to channel 
    setVoiceAttr(Hellopitch, HELLO, hello_spu_address); // SPU_0CH == hello
    setVoiceAttr(Polypitch, POLY, poly_spu_address);  // SPU_2CH == poly
    initGraph();
    while (1)
    {
        if(!counter){
            playSFX(HELLO);     // Play first VAG
            counter = 240;
        }
        if(counter == 160){      
            playSFX(POLY);      // Play second VAG
        }
        if(counter == 80){
            playSFX(HELLO|POLY); // Play both VAGs simultaneously
        }
        FntPrint("First VAG:");
        FntPrint("\nPitch             : %08x-%dKhz", Hellopitch, (SWAP_ENDIAN32(HellofileHeader->samplingFrequency)) );
        FntPrint("\nSet Start addr    : %08x", hello_spu_address);
        FntPrint("\nReturn start addr : %08x", hello_spu_start_address);      
        FntPrint("\nGet Start  addr   : %08x", hello_get_start_addr);  
        FntPrint("\nSend size         : %08x", SWAP_ENDIAN32(HellofileHeader->dataSize));  
        FntPrint("\nReturn size       : %08x\n", hello_transSize);  
        FntPrint("\nSecond VAG:");
        FntPrint("\nPitch             : %08x-%dKhz", Polypitch, (SWAP_ENDIAN32(HellofileHeader->samplingFrequency)) );
        FntPrint("\nSet Start addr    : %08x", poly_spu_address);
        FntPrint("\nReturn start addr : %08x", poly_spu_start_address);      
        FntPrint("\nGet Start  addr   : %08x", poly_get_start_addr);  
        FntPrint("\nSend size         : %08x", SWAP_ENDIAN32(PolyfileHeader->dataSize));  
        FntPrint("\nReturn size       : %08x\n", poly_transSize);  
        FntPrint("\nCounter       : %d\n", counter);  
        FntFlush(-1);
        counter --;
        display();
    }
    return 0;
    }
