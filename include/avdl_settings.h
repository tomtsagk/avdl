#ifndef AVDL_SETTINGS_H
#define AVDL_SETTINGS_H

/*
 * settings to aid compilation
 */

// all supported operating systems
#define AVDL_OS_LINUX   1
#define AVDL_OS_WINDOWS 2
#define AVDL_OS_UNKNOWN 3

// use this to determine OS at compile-time
#define AVDL_IS_OS(os) (AVDL_OS == os)

// if not explicitely given an OS, determine the current one
#ifndef AVDL_OS

	// windows
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define AVDL_OS AVDL_OS_WINDOWS

	// linux
	#elif defined(__unix__) || defined(linux)
	#define AVDL_OS AVDL_OS_LINUX

	// unknown
	#else
	#define AVDL_OS AVDL_OS_UNKNOWN

	// done figuring out OS
	#endif

#endif

/*
 * Compiler settings on how to compile a game
 */
struct AvdlSettings {

	// where to look for source files
	char src_dir[100];

	// where to look for assets
	char asset_dir[100];

	// name of the project
	char project_name[100];

	// versioning
	int version_code;
	char version_name[100];
	int revision;

	// icon
	char icon_path[100];

	char package[100];
};

void AvdlSettings_Create(struct AvdlSettings *);
void AvdlSettings_SetFromFile(struct AvdlSettings *, char *filename);

#endif
