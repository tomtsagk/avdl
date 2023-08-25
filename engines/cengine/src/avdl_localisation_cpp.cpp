#include "avdl_localisation.h"
#include "avdl_steam.h"
#include <string.h>

#if defined( AVDL_STEAM )
const char * avdl_locale_getSystemLocale() {
	if (strcmp(SteamApps()->GetCurrentGameLanguage(), "greek") == 0) {
		return "el";
	}
	else
	if (strcmp(SteamApps()->GetCurrentGameLanguage(), "english") == 0) {
		return "en";
	}
	else
	if (strcmp(SteamApps()->GetCurrentGameLanguage(), "japanese") == 0) {
		return "ja";
	}
	else
	if (strcmp(SteamApps()->GetCurrentGameLanguage(), "german") == 0) {
		return "de";
	}
	return "en";
}
#endif
