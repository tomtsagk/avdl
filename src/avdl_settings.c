#include "avdl_settings.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "avdl_pkg.h"
#include "avdl_log.h"

#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <unistd.h>
#endif

int AvdlSettings_Create(struct AvdlSettings *o) {
	strncpy(o->src_dir, "src/", 99);
	o->src_dir[99] = '\0';
	strncpy(o->asset_dir, "assets/", 99);
	o->asset_dir[99] = '\0';
	strncpy(o->project_name, "Avdl Project", 99);
	o->project_name[99] = '\0';

	strncpy(o->project_name_code, "avdl_project", 99);
	o->project_name_code[99] = '\0';

	o->version_code = 1;
	strncpy(o->version_code_str, "1", 99);
	o->version_code_str[99] = '\0';
	strncpy(o->version_name, "0.0.0", 99);
	o->version_name[99] = '\0';
	o->revision = 0;

	strncpy(o->icon_path, "icon.png", 99);
	o->icon_path[99] = '\0';

	strncpy(o->icon_ico_path, "icon.ico", 99);
	o->icon_ico_path[99] = '\0';

	strncpy(o->icon_foreground_path, "icon_foreground.png", 99);
	o->icon_foreground_path[99] = '\0';

	strncpy(o->icon_background_path, "icon_background.png", 99);
	o->icon_background_path[99] = '\0';

	strncpy(o->package, "com.company.sample_project", 99);
	o->icon_path[99] = '\0';

	// get avdl path
	const char *avdl_project_path = avdl_pkg_GetProjectPath();
	if (!avdl_project_path) {
		avdl_log_error("cannot get package path");
		return -1;
	}
	if (strlen(avdl_project_path) > 1023) {
		avdl_log_error("package path is too long: %s", avdl_project_path);
		return -1;
	}
	strncpy(o->pkg_path, avdl_project_path, 1023);
	o->pkg_path[1023] = '\0';

	// get cengine path
	const char *cengine_path = avdl_pkg_GetCenginePath();
	if (!cengine_path) {
		avdl_log_error("cannot get cengine path");
		return -1;
	}
	if (strlen(cengine_path) > 1023) {
		avdl_log_error("cengine path is too long: %s", cengine_path);
		return -1;
	}
	strncpy(o->cengine_path, cengine_path, 1023);
	o->cengine_path[1023] = '\0';

	o->save_path[0] = '\0';

	o->steam_mode = 0;
	o->standalone = 0;
	o->quiet_mode = 0;
	o->asset_prefix[0] = '\0';

	o->translate_only = 0;

	o->total_include_directories = 0;
	o->total_lib_directories = 0;

	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	o->target_platform = AVDL_PLATFORM_WINDOWS;
	#elif AVDL_IS_OS(AVDL_OS_LINUX)
	o->target_platform = AVDL_PLATFORM_LINUX;
	#else
	o->target_platform = AVDL_PLATFORM_UNKNOWN;
	#endif

	return 0;
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
		if (fscanf(f, "%1[#]", key)) {
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
		fscanf(f, "%99[^\n\r \t]", value);
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

			// convert project name to a code-friendly version
			strncpy(o->project_name_code, o->project_name, 99);
			o->project_name_code[99] = '\0';
			for (int i = 0; i < strlen(o->project_name_code); i++) {
				if (o->project_name_code[i] >= 'a'
				&&  o->project_name_code[i] <= 'z') {
					continue;
				}

				if (o->project_name_code[i] >= 'A'
				&&  o->project_name_code[i] <= 'Z') {
					o->project_name_code[i] -= ('A' - 'a');
					continue;
				}

				if (o->project_name_code[i] >= '0'
				&&  o->project_name_code[i] <= '9') {
					continue;
				}

				o->project_name_code[i] = '_';
			}

		}
		else
		if (strcmp( key, "version_code" ) == 0) {
			o->version_code = atoi(value);
			strncpy(o->version_code_str, value, 99);
			o->version_code_str[99] = '\0';
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
			fclose(f);
			return -1;
		}
	}

	fclose(f);

	return 0;
}
