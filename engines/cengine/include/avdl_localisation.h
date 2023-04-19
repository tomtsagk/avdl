#ifndef AVDL_LOCALISATION_H
#define AVDL_LOCALISATION_H

#ifdef __cplusplus
extern "C" {
#endif

struct avdl_localisation {
	char keys[10][100];
	char values[10][100];
	int count;
	void (*set)(struct avdl_localisation *, const char *);
	char *(*getValue)(struct avdl_localisation *, const char *);
};

void avdl_localisation_create(struct avdl_localisation *o);
void avdl_localisation_clean(struct avdl_localisation *o);

void avdl_localisation_set(struct avdl_localisation *o, const char *keyGroupID);
char *avdl_localisation_getValue(struct avdl_localisation *o, const char *key);

void avdl_localisation_print(struct avdl_localisation *o);

#ifdef __cplusplus
}
#endif

#endif
