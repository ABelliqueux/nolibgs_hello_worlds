#include "mod.h"

long musicEvent;
typedef struct SpuVoiceVolume {
    short volL, volR;
} SpuVoiceVolume;

SpuVoiceVolume volumeState[24] = {0};

static void muteSPUvoices() {
  for (unsigned i = 0; i < 24; i++) {
    // Store current volume
    SpuGetVoiceVolume(i, &(volumeState[i].volL), &(volumeState[i].volR) );
    // Mute
    SpuSetVoiceVolume(i, 0, 0);
  }
}

static void restoreSPUvoices() {
  for (unsigned i = 0; i < 24; i++) {
    // Restore volume
    SpuSetVoiceVolume(i, volumeState[i].volL, volumeState[i].volR );
  }
}
// Playing a sound effect (aka mod note): https://discord.com/channels/642647820683444236/642848592754901033/898249196174458900 
// Code by NicolasNoble : https://discord.com/channels/642647820683444236/663664210525290507/902624952715452436
void loadMod() {
    printf("Loading MOD:\'%s\'\n", HITFILE);
    MOD_Load((struct MODFileFormat*)HITFILE);
    printf("%02d Channels, %02d Orders\n", MOD_Channels, MOD_SongLength);
}

static long processMusic() {
    uint32_t old_hblanks = MOD_hblanks;
    MOD_Poll();
    uint32_t new_hblanks = MOD_hblanks;
    if (old_hblanks != new_hblanks) SetRCnt(RCntCNT1, new_hblanks, RCntMdINTR);
    return MOD_hblanks;
}

void startMusic() {
  ResetRCnt(RCntCNT1);
  SetRCnt(RCntCNT1, MOD_hblanks, RCntMdINTR);
  StartRCnt(RCntCNT1);
  musicEvent = OpenEvent(RCntCNT1, EvSpINT, EvMdINTR, processMusic);
  EnableEvent(musicEvent);
  restoreSPUvoices();
}

void pauseMusic() {
  muteSPUvoices();
  DisableEvent(musicEvent);
}

void resumeMusic() {
  restoreSPUvoices();
  EnableEvent(musicEvent);
}

void stopMusic() {
  MOD_Silence();
  StopRCnt(RCntCNT1);
  DisableEvent(musicEvent);
  CloseEvent(musicEvent);
}
