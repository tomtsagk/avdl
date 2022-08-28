#include "avdl_steam.h"
#include <stdio.h>
#include "steam_api.h"

int avdl_steam_init() {
	return SteamAPI_Init();
}

void avdl_steam_update() {
	SteamAPI_RunCallbacks();
}

void avdl_steam_shutdown() {
	SteamAPI_Shutdown();
}
