#include "avdl_settings.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <unistd.h>
#endif

void AvdlSettings_Create(struct AvdlSettings *o) {
	strncpy(o->src_dir, "src/", 99);
	o->src_dir[99] = '\0';
	strncpy(o->asset_dir, "assets/", 99);
	o->asset_dir[99] = '\0';
	strncpy(o->project_name, "avdl project", 99);
	o->project_name[99] = '\0';

	o->version_code = 1;
	strncpy(o->version_name, "0.0.0", 99);
	o->version_name[99] = '\0';
	o->revision = 0;

	strncpy(o->icon_path, "icon.png", 99);
	o->icon_path[99] = '\0';

	strncpy(o->package, "com.company.sample_project", 99);
	o->icon_path[99] = '\0';
}

int AvdlSettings_SetFromFile(struct AvdlSettings *o, char *filename) {
	FILE *f = fopen(filename, "r");
	if (!f) {
		printf("avdl error: reading settings file '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	char key[100];
	char value[100];
	while ( fscanf(f, "%*[ \t\r\n]") != EOF ) {

		// ignore comment lines
		if (fscanf(f, "%[#]", key)) {
			fscanf(f, "%*[^\r\n]");
			continue;
		}

		// find key
		fscanf(f, "%99[^: \t]", key);
		key[99] = '\0';
		//printf("Found key: %s.\n", key);

		// remove ':' and surrounding whitespace
		fscanf(f, "%*[ \t]");
		fscanf(f, "%*c");
		fscanf(f, "%*[ \t]");

		// find value
		fscanf(f, "%100[^\n\r \t]", value);
		//printf("Found value: %s.\n", value);

		// remove white space / new line
		fscanf(f, "%*[ \t\r\n]");

		/*
		 * It's possibly best not to move the source directory
		if (strcmp( key, "source" ) == 0) {
			strncpy(o->src_dir, value, 99);
			o->src_dir[99] = '\0';
		}
		else
		*/
		/*
		 * It's possibly best not to move the assets directory
		if (strcmp( key, "assets" ) == 0) {
			strncpy(o->asset_dir, value, 99);
			o->asset_dir[99] = '\0';
		}
		else
		*/
		if (strcmp( key, "name" ) == 0) {
			strncpy(o->project_name, value, 99);
			o->project_name[99] = '\0';
		}
		else
		if (strcmp( key, "version_code" ) == 0) {
			o->version_code = atoi(value);
		}
		else
		if (strcmp( key, "version_name" ) == 0) {
			strncpy(o->version_name, value, 99);
			o->version_name[99] = '\0';
		}
		else
		if (strcmp( key, "revision" ) == 0) {
			o->revision = atoi(value);
		}
		else
		if (strcmp( key, "icon" ) == 0) {
			strncpy(o->icon_path, value, 99);
			o->icon_path[99] = '\0';
		}
		else
		if (strcmp( key, "package" ) == 0) {
			strncpy(o->package, value, 99);
			o->icon_path[99] = '\0';
		}
		else {
			printf("avdl error: Unknown key: %s\n", key);
			return -1;
		}
	}

	return 0;
}
