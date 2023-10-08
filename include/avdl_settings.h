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

#if AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <windows.h>
#endif

#ifndef PKG_NAME
#define PKG_NAME "avdl"
#endif

#ifndef PKG_VERSION
#define PKG_VERSION "0.0.0-1"
#endif

#ifndef PKG_LOCATION
#define PKG_LOCATION ""
#endif

/*
 * compiling platform
 *
 * used when compiling a game to decide
 * which platform to compile for
 *
 * by default, it will try to compile for the same
 * platform as the host device
 *
 */
enum AVDL_PLATFORM {
	AVDL_PLATFORM_LINUX,
	AVDL_PLATFORM_WINDOWS,
	AVDL_PLATFORM_ANDROID,
	AVDL_PLATFORM_QUEST2,
	AVDL_PLATFORM_D3D11,

	AVDL_PLATFORM_UNKNOWN,
};

#define AVDL_GOOGLE_PLAY_ACHIEVEMENTS_MAX 20
struct avdl_google_play_achievement {
	char id[100];
	char api[100];
};

/*
 * Compiler settings on how to compile a game
 * contains all data that could be changed
 * every time this program is run
 */
struct AvdlSettings {

	// where to look for source files
	char src_dir[100];

	// where to look for assets
	char asset_dir[100];

	// name of the project
	char project_name[100];
	char project_name_code[100];

	// versioning
	int version_code;
	char version_code_str[100];
	char version_name[100];
	int revision;

	// icon
	char icon_path[100];
	char icon_ico_path[100];
	char icon_foreground_path[100];
	char icon_background_path[100];

	// package (like com.company.app)
	char package[100];

	/* avdl details */
	char pkg_path[1024];
	char cengine_path[1024];
	char save_path[1024];

	// add steam
	int steam_mode;

	// include c++ functionality
	int cpp_mode;

	int standalone;
	int quiet_mode;
	int makefile_mode;
	int cmake_mode;

	// where to look for assets
	// For exampe "share/avdl/" would search for assets in "share/avdl/assets/"
	char asset_prefix[1024];

	int translate_only;

	char *additional_include_directory[10];
	int total_include_directories;
	char *additional_lib_directory[10];
	int total_lib_directories;

	// what platform to compile for
	enum AVDL_PLATFORM target_platform;

	// google play
	int googleplay_mode;
	char googleplay_id[100];
	struct avdl_google_play_achievement googleplay_achievement[AVDL_GOOGLE_PLAY_ACHIEVEMENTS_MAX];
	int googleplay_achievement_count;

	// admob ads
	int admob_ads;
	char admob_ads_id[100];
	int admob_ads_fullscreen;
	int admob_ads_rewarded;
	char admob_ads_fullscreen_id[100];
	char admob_ads_rewarded_id[100];

	// oculus build
	int oculus_mode;
	char oculus_project_id[100];

	int use_cache;

};

int AvdlSettings_Create(struct AvdlSettings *);
int AvdlSettings_SetFromFile(struct AvdlSettings *, char *filename);

#endif
