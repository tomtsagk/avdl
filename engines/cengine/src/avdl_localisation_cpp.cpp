#include "avdl_localisation.h"
#include "avdl_steam.h"
#include <string.h>

#if defined( AVDL_STEAM )
const char * avdl_locale_getSystemLocale() {
	if (strcmp(SteamApps()->GetCurrentGameLanguage(), "greek") == 0) {
		return "el";
	}
	return "en";
}
#endif
