#pragma once
#include <libapi.h>
#include <libspu.h>
#include "../../thirdparty/nugget/common/hardware/hwregs.h"
#include "../../thirdparty/nugget/common/hardware/irq.h"
#include "../../thirdparty/nugget/common/syscalls/syscalls.h"
#define printf ramsyscall_printf

// Mod Playback
#include "modplayer.h"
extern const uint8_t _binary_HIT_STAR_HIT_start[];
#define HITFILE _binary_HIT_STAR_HIT_start
extern long musicEvent;

void muteSPUvoices();
void restoreSPUvoices();
void loadMod();
long processMusic();
void startMusic();
void pauseMusic();
void resumeMusic();
void stopMusic();