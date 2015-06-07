#include <confuse.h>
#include <unistd.h>
#include <argp.h>
#include <stdlib.h>
#include "config.h"

#define OPT_USAGE ""
#define OPT_HELP "Control Raspberry Pi power with an external MCU connected over GPIO"
#define OPT_DEBUG -1
#define OPT_DAEMON 'd'
#define OPT_PIDFILE 'p'

#define CFG_GPIO_IN "GpioIn"
#define CFG_GPIO_OUT "GpioOut"
#define CFG_POLL_FREQ "PollFrequency"

typedef struct {
    uint8_t GpioIn;
    uint8_t GpioOut;
    uint32_t PollFrequency;
} ConfuseConfig;

typedef struct {
    bool RunAsDaemon;
    bool DebugEnabled;
    const char *PidFile;
} Arguments;

static error_t ParseOption(int key, char *arg, struct argp_state *state);
static struct argp_option options[] = {
        { "daemon", OPT_DAEMON, 0, 0, "Run as a daemon" },
        { "debug", OPT_DEBUG, 0, 0, "Run with debug options set in gpio library" },
        { "pidfile", OPT_PIDFILE, "FILE", 0, "Write PID to FILE" },
        { 0 }
};

static bool ValidateConfig(PiSwitchConfig *config);
static Arguments ParseArguments(int argc, char **argv);

bool TryGetPiSwitchConfig(const char *name, int argc, char **argv, PiSwitchConfig *piSwitchConfig) {
    Arguments arguments = ParseArguments(argc, argv);

    ConfuseConfig config;
    config.GpioIn = 0;
    config.GpioOut = 0;
    config.PollFrequency = 0;

    cfg_opt_t opts[] = {
            CFG_SIMPLE_INT(CFG_GPIO_IN, &config.GpioIn),
            CFG_SIMPLE_INT(CFG_GPIO_OUT, &config.GpioOut),
            CFG_SIMPLE_INT(CFG_POLL_FREQ, &config.PollFrequency),
            CFG_END()
    };

    cfg_t *cfg;
    cfg = cfg_init(opts, 0);

    if (access(name, F_OK) == -1 || cfg_parse(cfg, name) != CFG_SUCCESS) {
        return false;
    }

    piSwitchConfig->RunAsDaemon = !arguments.DebugEnabled && arguments.RunAsDaemon;
    piSwitchConfig->DebugEnabled = arguments.DebugEnabled;
    piSwitchConfig->PidFile = arguments.PidFile;
    piSwitchConfig->PidFilePointer = -1;
    piSwitchConfig->GpioIn = config.GpioIn;
    piSwitchConfig->GpioOut = config.GpioOut;
    piSwitchConfig->PollFrequency = config.PollFrequency;

    cfg_free(cfg);

    // Check config
    if(!ValidateConfig(piSwitchConfig)) {
        return false;
    }

    return true;
}

static bool ValidateConfig(PiSwitchConfig *config) {
    if(config->RunAsDaemon && config->PidFile == NULL) {
        fprintf(stderr, "PID file required when running as daemon\n");
        return false;
    }

    if(config->GpioIn <= 0) {
        fprintf(stderr, "%s Must be > 0\n", CFG_GPIO_IN);
        return false;
    }

    if(config->GpioOut <= 0) {
        fprintf(stderr, "%s Must be > 0\n", CFG_GPIO_OUT);
        return false;
    }

    if(config->PollFrequency <= 0) {
        fprintf(stderr, "%s Must be > 0\n", CFG_POLL_FREQ);
        return false;
    }

    return true;
}

static Arguments ParseArguments(int argc, char **argv) {
    Arguments arguments;
    arguments.RunAsDaemon = false;
    arguments.DebugEnabled = false;
    arguments.PidFile = NULL;

    struct argp argp = { options, ParseOption, OPT_USAGE, OPT_HELP };
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    return arguments;
}

static error_t ParseOption(int key, char *arg, struct argp_state *state) {
    Arguments *arguments = state->input;

    switch (key)
    {
        case OPT_DAEMON:
            arguments->RunAsDaemon = true;
            break;
        case OPT_PIDFILE:
            arguments->PidFile = arg;
            break;
        case OPT_DEBUG:
            arguments->DebugEnabled = true;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}