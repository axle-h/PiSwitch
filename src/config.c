#include <confuse.h>
#include <unistd.h>
#include "config.h"

typedef struct {
    cfg_bool_t DebugEnabled;
    const char *PidFile;
    uint8_t GpioIn;
    uint8_t GpioOut;
    uint32_t PollFrequency;
} ConfuseConfig;

bool ValidateConfig(ConfuseConfig *config);

bool TryGetPiSwitchConfig(const char *name, PiSwitchConfig *piSwitchConfig) {
    ConfuseConfig config;
    config.DebugEnabled = cfg_false;
    config.PidFile = NULL;
    config.GpioIn = 0;
    config.GpioOut = 0;
    config.PollFrequency = 0;

    cfg_opt_t opts[] = {
            CFG_SIMPLE_BOOL("DebugEnabled", &config.DebugEnabled),
            CFG_SIMPLE_STR("PidFile", &config.PidFile),
            CFG_SIMPLE_INT("GpioIn", &config.GpioIn),
            CFG_SIMPLE_INT("GpioOut", &config.GpioOut),
            CFG_SIMPLE_INT("PollFrequency", &config.PollFrequency),
            CFG_END()
        };

    cfg_t *cfg;
    cfg = cfg_init(opts, 0);

    if (access(name, F_OK) == -1 || cfg_parse(cfg, name) == CFG_PARSE_ERROR) {
        return false;
    }

    // Check config
    if(!ValidateConfig(&config)) {
        return false;
    }

    piSwitchConfig->PidFile = config.PidFile;
    piSwitchConfig->GpioIn = config.GpioIn;
    piSwitchConfig->GpioOut = config.GpioOut;
    piSwitchConfig->DebugEnabled = config.DebugEnabled ? true : false;
    piSwitchConfig->PollFrequency = config.PollFrequency;

    cfg_free(cfg);

    return true;
}

bool ValidateConfig(ConfuseConfig *config) {
    return config->PidFile != NULL && config->GpioIn > 0 && config->GpioOut > 0 && config->PollFrequency > 0;
}