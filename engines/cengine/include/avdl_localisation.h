#ifndef AVDL_LOCALISATION_H
#define AVDL_LOCALISATION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum avdl_locale {
	AVDL_LOCALE_ENGLISH,
	AVDL_LOCALE_GERMAN,
	AVDL_LOCALE_JAPANESE,
	AVDL_LOCALE_GREEK,

	AVDL_LOCALE_DEFAULT = AVDL_LOCALE_ENGLISH,
};

#define LOC_MAX_KEYS 50
#define LOC_MAX_CHARACTERS 400

struct avdl_localisation {
	char keys[LOC_MAX_KEYS][LOC_MAX_CHARACTERS];
	char values[LOC_MAX_KEYS][LOC_MAX_CHARACTERS];
	int count;
	void (*set)(struct avdl_localisation *, const char *);
	char *(*getValue)(struct avdl_localisation *, const char *);
};

void avdl_localisation_create(struct avdl_localisation *o);
void avdl_localisation_clean(struct avdl_localisation *o);

void avdl_localisation_set(struct avdl_localisation *o, const char *keyGroupID);
char *avdl_localisation_getValue(struct avdl_localisation *o, const char *key);

void avdl_localisation_print(struct avdl_localisation *o);

void avdl_locale_set(const char *locale_code);

const char *avdl_locale_getSystemLocale();

#ifdef __cplusplus
}
#endif

#endif
