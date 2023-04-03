#ifndef AVDL_STEAM_H
#define AVDL_STEAM_H

#ifdef __cplusplus
#ifdef AVDL_STEAM
#include "steam_api.h"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

int avdl_steam_init();
void avdl_steam_update();
void avdl_steam_shutdown();

#ifdef __cplusplus
}
#endif

#endif
