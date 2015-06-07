#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool RunAsDaemon;
    bool DebugEnabled;
    const char *PidFile;
    int PidFilePointer;
    uint8_t GpioIn;
    uint8_t GpioOut;
    uint32_t PollFrequency;
} PiSwitchConfig;

typedef struct {

} PiSwitchArgs;


bool TryGetPiSwitchConfig(const char *name, int argc, char **argv, PiSwitchConfig *piSwitchConfig);
