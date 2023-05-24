#include "avdl_webapi.h"
#include "avdl_steam.h"

#if defined(AVDL_STEAM)
void avdl_webapi_openurl(const char *url) {
	SteamFriends()->ActivateGameOverlayToWebPage(url);
}
#endif
