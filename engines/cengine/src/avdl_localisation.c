#include "avdl_localisation.h"
#include "dd_json.h"
#include <string.h>
#include "dd_log.h"
#include "dd_game.h"
#include "avdl_assetManager.h"

#include <stddef.h>

enum avdl_locale AVDL_LOCALE_CURRENT = AVDL_LOCALE_DEFAULT;

void avdl_localisation_create(struct avdl_localisation *o) {
	o->count = 0;
	o->set = avdl_localisation_set;
	o->getValue = avdl_localisation_getValue;
}

void avdl_localisation_clean(struct avdl_localisation *o) {
}

static const char *localisation_to_filecode(enum avdl_locale locale) {
	switch (locale) {
	case AVDL_LOCALE_GERMAN  : return "_de"; break;
	case AVDL_LOCALE_JAPANESE: return "_ja"; break;
	case AVDL_LOCALE_GREEK: return "_el"; break;
	default:
	case AVDL_LOCALE_ENGLISH : return ""; break;
	}
}

extern const char* getAppLocation(void);

void avdl_localisation_set(struct avdl_localisation *o, const char *keyGroupID) {

	//dd_log("avdl: localisation '%s'", keyGroupID);
	struct dd_json_object obj;

	#if defined( AVDL_DIRECT3D11 )
	strcpy_s(obj.buffer, 1000, getAppLocation());
	strcat_s(obj.buffer, 1000, "/assets/");
	strcat_s(obj.buffer, 1000, keyGroupID);
	strcat_s(obj.buffer, 1000, localisation_to_filecode(AVDL_LOCALE_CURRENT));
	strcat_s(obj.buffer, 1000, ".json");
	#elif defined( AVDL_WINDOWS )
	strcpy(obj.buffer, "assets/");
	strcat(obj.buffer, keyGroupID);
	strcat(obj.buffer, localisation_to_filecode(AVDL_LOCALE_CURRENT));
	strcat(obj.buffer, ".json");
	#elif defined( AVDL_LINUX )
	strcpy(obj.buffer, avdl_getProjectLocation());
	strcat(obj.buffer, GAME_ASSET_PREFIX);
	strcat(obj.buffer, "assets/");
	strcat(obj.buffer, keyGroupID);
	strcat(obj.buffer, localisation_to_filecode(AVDL_LOCALE_CURRENT));
	strcat(obj.buffer, ".json");
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	strcpy(obj.buffer, keyGroupID);
	strcat(obj.buffer, localisation_to_filecode(AVDL_LOCALE_CURRENT));
	strcat(obj.buffer, ".json");
	#endif
	dd_json_initFile(&obj, obj.buffer);

	// expect start of object
	dd_json_next(&obj);
	if (dd_json_getToken(&obj) != DD_JSON_OBJECT_START) {
		dd_log("avdl: error reading json file '%s'", obj.buffer);
		return;
	}

	// expect series of key-string pairs until it's done
	o->count = 0;
	dd_json_next(&obj);
	while (dd_json_getToken(&obj) == DD_JSON_KEY) {

		if (o->count >= LOC_MAX_KEYS) {
			dd_log("avdl error: too many localisation keys while reading key '%s', maximum is %d", obj.buffer, LOC_MAX_KEYS);
			break;
		}
		const char *c = dd_json_getTokenString(&obj);
		if (strlen(c) >= LOC_MAX_CHARACTERS) {
			dd_log("avdl error: loc key '%s' has too many characters: %d / %d", c, strlen(c), LOC_MAX_CHARACTERS);
			break;
		}
		#if defined( AVDL_DIRECT3D11 )
		strcpy_s(o->keys[o->count], 400, c);
		#else
		strcpy(o->keys[o->count], c);
		#endif
		dd_json_next(&obj);
		const char *c2 = dd_json_getTokenString(&obj);
		if (strlen(c2) >= LOC_MAX_CHARACTERS) {
			dd_log("avdl error: loc value has too many characters: %d / %d", strlen(c2), LOC_MAX_CHARACTERS);
			dd_log("value: %s", c2);
			break;
		}
		#if defined( AVDL_DIRECT3D11 )
		strcpy_s(o->values[o->count], 400, c2);
		#else
		strcpy(o->values[o->count], c2);
		#endif
		dd_json_next(&obj);
		o->count++;
	}

	//avdl_localisation_print(o);
	dd_json_deinit(&obj);

}

char *avdl_localisation_getValue(struct avdl_localisation *o, const char *key) {
	for (int i = 0; i < o->count; i++) {
		if (strcmp(o->keys[i], key) == 0) {
			return o->values[i];
		}
	}
	return "empty";
}

void avdl_localisation_print(struct avdl_localisation *o) {
	for (int i = 0; i < o->count; i++) {
		dd_log("key: %s | value: %s", o->keys[i], o->values[i]);
	}
}

void avdl_locale_set(const char *locale_code) {

	// no locale given - go to system default
	if (!locale_code) {
		locale_code = avdl_locale_getSystemLocale();
	}

	if (strcmp(locale_code, "en") == 0) {
		AVDL_LOCALE_CURRENT = AVDL_LOCALE_ENGLISH;
	}
	else
	if (strcmp(locale_code, "de") == 0) {
		AVDL_LOCALE_CURRENT = AVDL_LOCALE_GERMAN;
	}
	else
	if (strcmp(locale_code, "ja") == 0) {
		AVDL_LOCALE_CURRENT = AVDL_LOCALE_JAPANESE;
	}
	else
	if (strcmp(locale_code, "el") == 0) {
		AVDL_LOCALE_CURRENT = AVDL_LOCALE_GREEK;
	}
}

#if !defined( AVDL_STEAM )
const char *avdl_locale_getSystemLocale() {
	return "en";
}
#endif
