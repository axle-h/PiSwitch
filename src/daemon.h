#pragma once

typedef void (*TermSignalHandler) (int);

void StartDaemon(const char *pidFile, TermSignalHandler termSignalHandler);