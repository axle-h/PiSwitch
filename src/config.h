#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool DebugEnabled;
    const char *PidFile;
    uint8_t GpioIn;
    uint8_t GpioOut;
    uint32_t PollFrequency;
} PiSwitchConfig;


bool TryGetPiSwitchConfig(const char *name, PiSwitchConfig *piSwitchConfig);