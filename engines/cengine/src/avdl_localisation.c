#include "avdl_localisation.h"
#include "dd_json.h"
#include <string.h>
#include "dd_log.h"
#include "dd_game.h"

#ifndef PKG_LOCATION
#define PKG_LOCATION
#endif

void avdl_localisation_create(struct avdl_localisation *o) {
	o->count = 0;
	o->set = avdl_localisation_set;
	o->getValue = avdl_localisation_getValue;
}

void avdl_localisation_clean(struct avdl_localisation *o) {
}

void avdl_localisation_set(struct avdl_localisation *o, const char *keyGroupID) {

	//dd_log("avdl: localisation '%s'", keyGroupID);
	struct dd_json_object obj;
	/*
	strcpy(obj.buffer, avdl_getProjectLocation());
	strcat(obj.buffer, "assets/");
	strcat(obj.buffer, keyGroupID);
	strcat(obj.buffer, ".asset");
	*/

	wcscpy(obj.bufferW, avdl_getProjectLocation());
	wcscat(obj.bufferW, L"assets/");
	//wcscat(obj.bufferW, keyGroupID);
	//wcscat(obj.bufferW, L"keyGroupID");
	mbstowcs((obj.bufferW +wcslen(obj.bufferW)), keyGroupID, 1000 -wcslen(obj.bufferW));
	wcscat(obj.bufferW, L".asset");
	//wprintf(L"localisation: %lS\n", obj.bufferW);
	dd_json_initFile(&obj, obj.bufferW);

	// expect start of object
	dd_json_next(&obj);
	if (dd_json_getToken(&obj) != DD_JSON_OBJECT_START) {
		//wprintf(L"avdl: error reading json file '%lS'\n", keyGroupID);
		dd_log("avdl: error reading json file '%s'", keyGroupID);
		return;
	}

	// expect series of key-string pairs until it's done
	o->count = 0;
	dd_json_next(&obj);
	while (dd_json_getToken(&obj) == DD_JSON_KEY && o->count < 10) {
		strcpy(o->keys[o->count], dd_json_getTokenString(&obj));
		dd_json_next(&obj);
		strcpy(o->values[o->count], dd_json_getTokenString(&obj));
		dd_json_next(&obj);
		o->count++;
	}

	//avdl_localisation_print(o);
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
