#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "avdl_struct_table.h"
#include "avdl_commands.h"
#include "avdl_file_op.h"
#include "avdl_lexer.h"
#include "avdl_semantic_analyser.h"
#include "avdl_ast_node.h"
#include "avdl_parser.h"
#include "avdl_platform.h"
#include "avdl_settings.h"
#include "avdl_pkg.h"
#include "avdl_log.h"
#include "avdl_string.h"
#include "avdl_arguments.h"

#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <unistd.h>
#endif

const char cache_dir[] = ".avdl_cache/";

extern float parsing_float;

char included_files[10][100];
int included_files_num = 0;

// buffer for general use
#define DD_BUFFER_SIZE 2048
char buffer[DD_BUFFER_SIZE];

// game node, parent of all nodes
struct ast_node *game_node;

char *additionalIncludeDirectory[10];
int totalIncludeDirectories = 0;
char *additionalLibDirectory[10];
int totalLibDirectories = 0;

int create_android_directory(const char *androidDirName);

struct cengine_file_structure {
	const char *main;
	const char *steam;
};

struct cengine_file_structure cengine_files[] = {
	{"avdl_assetManager.c", 0},
	{"avdl_data.c", 0},
	{"avdl_localisation.c", 0},
	{"avdl_particle_system.c", 0},
	{"avdl_shaders.c", 0},
	{"dd_dynamic_array.c", 0},
	{"dd_filetomesh.c", 0},
	{"dd_fov.c", 0},
	{"dd_game.c", 0},
	{"dd_gamejolt.c", 0},
	{"dd_image.c", 0},
	{"dd_json.c", 0},
	{"dd_log.c", 0},
	{"dd_math.c", 0},
	{"dd_matrix.c", 0},
	{"dd_mesh.c", 0},
	{"dd_meshColour.c", 0},
	{"dd_meshTexture.c", 0},
	{"dd_mouse.c", 0},
	{"dd_opengl.c", 0},
	{"dd_sound.c", 0},
	{"dd_music.c", 0},
	{"whereami.c", 0},
	{"dd_string3d.c", 0},
	{"dd_vec3.c", 0},
	{"dd_vec4.c", 0},
	{"dd_world.c", 0},
	{"avdl_input.c", 0},
	{"main.c", 0},
	{"avdl_steam.c", "avdl_steam_cpp.cpp"},
	{0, "main_cpp.cpp"},
	{"avdl_achievements.c", "avdl_achievements_steam.cpp"},
	{"avdl_multiplayer.c", "avdl_multiplayer_steam.cpp"},
};
unsigned int cengine_files_total = sizeof(cengine_files) /sizeof(struct cengine_file_structure);

char *cengine_headers[] = {
	"avdl_steam.h",
	"avdl_assetManager.h",
	"avdl_cengine.h",
	"avdl_data.h",
	"avdl_localisation.h",
	"avdl_particle_system.h",
	"avdl_shaders.h",
	"dd_async_call.h",
	"dd_dynamic_array.h",
	"dd_filetomesh.h",
	"dd_fov.h",
	"dd_game.h",
	"dd_gamejolt.h",
	"dd_image.h",
	"dd_json.h",
	"dd_log.h",
	"dd_math.h",
	"dd_matrix.h",
	"dd_mesh.h",
	"dd_meshColour.h",
	"dd_meshTexture.h",
	"dd_mouse.h",
	"dd_opengl.h",
	"dd_sound.h",
	"dd_music.h",
	"whereami.h",
	"dd_string3d.h",
	"dd_vec2.h",
	"dd_vec3.h",
	"dd_vec4.h",
	"dd_world.h",
	"avdl_input.h",
	"avdl_achievements.h",
	"avdl_multiplayer.h",
};
unsigned int cengine_headers_total = sizeof(cengine_headers) /sizeof(char *);

// compiler steps
int avdl_transpile(struct AvdlSettings *);
int avdl_compile(struct AvdlSettings *);
int avdl_compile_cengine(struct AvdlSettings *);
int avdl_link(struct AvdlSettings *);
int avdl_assets(struct AvdlSettings *);
int avdl_android_object(struct AvdlSettings *);

const char *avdl_project_path;

// hide main when doing unit tests - temporary solution
#ifdef AVDL_UNIT_TEST
#define AVDL_MAIN avdl_main
#else
#define AVDL_MAIN main
#endif

// init data, parse, exit
int AVDL_MAIN(int argc, char *argv[]) {

	// project settings
	struct AvdlSettings avdl_settings;
	if (AvdlSettings_Create(&avdl_settings) != 0) {
		avdl_log_error("unable to initialise avdl settings");
		return -1;
	}

	// temp solutions
	avdl_project_path = avdl_settings.pkg_path;

	int handle_return = avdl_arguments_handle(&avdl_settings, argc, argv);

	totalIncludeDirectories = avdl_settings.total_include_directories;
	for (int i = 0; i < totalIncludeDirectories; i++) {
		additionalIncludeDirectory[i] = avdl_settings.additional_include_directory[i];
	}
	totalLibDirectories = avdl_settings.total_lib_directories;
	for (int i = 0; i < totalLibDirectories; i++) {
		additionalLibDirectory[i] = avdl_settings.additional_lib_directory[i];
	}

	// the arguments require the program to stop executing
	if (handle_return > 0) {
		return 0;
	}
	else
	// error while parsing arguments
	if (handle_return < 0) {
		avdl_log_error("failed to parse arguments");
		return -1;
	}

	// load settings from the current project
	if (AvdlSettings_SetFromFile(&avdl_settings, "app.avdl") != 0) {
		avdl_log_error("failed to get project settings from '%s'", "app.avdl");
		return -1;
	}

	avdl_log("~ Project Details ~");
	avdl_log("Project Name: " BLU "%s" RESET, avdl_settings.project_name);
	avdl_log("Version: " BLU "%d" RESET " (" BLU "%s" RESET ")-" BLU "%d" RESET, avdl_settings.version_code, avdl_settings.version_name, avdl_settings.revision);
	avdl_log("Icon: " BLU "%s" RESET, avdl_settings.icon_path);
	avdl_log("Package: " BLU "%s" RESET, avdl_settings.package);
	avdl_log("CEngine location in: " BLU "%s" RESET, avdl_settings.cengine_path);
	avdl_log("~ Project Details end ~");
	avdl_log("");

	if (!is_dir("avdl_build")) {
		dir_create("avdl_build");
	}

	if (!is_dir(".avdl_cache")) {
		dir_create(".avdl_cache");
	}

	// from `.dd` to `.c`
	if ( avdl_transpile(&avdl_settings) != 0) {
		avdl_log_error("couldn't transpile project\n");
		return -1;
	}

	// android
	if (avdl_platform_get() == AVDL_PLATFORM_ANDROID) {
		create_android_directory("avdl_build_android");

		avdl_android_object(&avdl_settings);

		// handle assets
		if ( avdl_assets(&avdl_settings) != 0) {
			avdl_log_error("could not handle project assets for android\n");
			return -1;
		}

		avdl_log("avdl project " BLU "\"%s\"" RESET " prepared successfully for android at " BLU "./avdl_build_android/" RESET, avdl_settings.project_name);

		return 0;
	}

	if (avdl_settings.translate_only) {
		avdl_log("avdl: done translating");
		return 0;
	}

	// from `.c` to `.o`
	if ( avdl_compile(&avdl_settings) != 0) {
		avdl_log_error("failed to compile '" BLU "%s" RESET "'", avdl_settings.project_name);
		return -1;
	}

	// cengine
	if ( avdl_compile_cengine(&avdl_settings) != 0) {
		avdl_log_error("failed to compile cengine\n");
		return -1;
	}

	// combine all `.o` to executable
	if ( avdl_link(&avdl_settings) != 0) {
		avdl_log_error("failed to link '" BLU "%s" RESET "'", avdl_settings.project_name);
		return -1;
	}

	// handle assets
	if ( avdl_assets(&avdl_settings) != 0) {
		avdl_log_error("failed to handle assets in '" BLU "%s" RESET "'");
		return -1;
	}

	avdl_log("avdl: project " BLU "\"%s\"" RESET " compiled successfully at " BLU "./avdl_build/" RESET, avdl_settings.project_name);

	// success!
	return 0;
}

int create_android_directory(const char *androidDirName) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	#else
	int isDir = is_dir(androidDirName);
	if (isDir == 0) {
		dir_create(androidDirName);
		struct avdl_string cenginePath;
		avdl_string_create(&cenginePath, 1024);
		avdl_string_cat(&cenginePath, avdl_pkg_GetProjectPath());
		avdl_string_cat(&cenginePath, "/share/avdl/android");
		if ( !avdl_string_isValid(&cenginePath) ) {
			avdl_log_error("cannot construct path of cengine: %s", avdl_string_getError(&cenginePath));
			avdl_string_clean(&cenginePath);
			return -1;
		}
		dir_copy_recursive(0, avdl_string_toCharPtr(&cenginePath), 0, androidDirName);
		avdl_string_clean(&cenginePath);
	}
	else
	if (isDir < 0) {
		avdl_log_error("file '%s' not a directory", androidDirName);
		return -1;
	}
	#endif

	return 0;
}

// Handles files to be transpiled
int transpile_file(const char *dirname, const char *filename, int fileIndex, int filesTotal) {

	// ignore `.` and `..`
	if (strcmp(filename, ".") == 0
	||  strcmp(filename, "..") == 0) {
		return 0;
	}

	// src file full path
	struct avdl_string srcFilePath;
	avdl_string_create(&srcFilePath, 1024);
	avdl_string_cat(&srcFilePath, dirname);
	avdl_string_cat(&srcFilePath, filename);
	if ( !avdl_string_isValid(&srcFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", dirname, filename, avdl_string_getError(&srcFilePath));
		avdl_string_clean(&srcFilePath);
		return -1;
	}

	// dst file full path
	struct avdl_string dstFilePath;
	avdl_string_create(&dstFilePath, 1024);
	avdl_string_cat(&dstFilePath, cache_dir);
	avdl_string_cat(&dstFilePath, filename);
	avdl_string_cat(&dstFilePath, ".c");
	if ( !avdl_string_isValid(&dstFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", cache_dir, filename, avdl_string_getError(&dstFilePath));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return -1;
	}

	// check file type
	struct stat statbuf;
	if (stat(avdl_string_toCharPtr(&srcFilePath), &statbuf) != 0) {
		avdl_log_error("Unable to stat file '%s': %s", avdl_string_toCharPtr(&srcFilePath), strerror(errno));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return -1;
	}

	// is directory - skip - maybe recursive compilation at some point?
	if (Avdl_FileOp_IsDirStat(&statbuf)) {
		avdl_log("skipping directory: %s", avdl_string_toCharPtr(&srcFilePath));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}
	else
	// is regular file - do nothing
	if (Avdl_FileOp_IsRegStat(&statbuf)) {
	}
	// not supporting other file types - skip
	else {
		avdl_log_error("Unsupported file type '%s' - skip\n", avdl_string_toCharPtr(&srcFilePath));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}

	// skip files already compiled (check last modified)
	// for the time being files don't need to be re-transpiled
	// if a header changes
	if ( !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), avdl_string_toCharPtr(&srcFilePath)) ) {
		//printf("avdl src file not modified, skipping transpilation of '%s' -> '%s'\n", buffer, buffer2);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}
	//printf("transpiling %s to %s\n", buffer, buffer2);

	included_files_num = 0;

	// initialise the parent node
	game_node = ast_create(AST_GAME);
	if (semanticAnalyser_convertToAst(game_node, avdl_string_toCharPtr(&srcFilePath)) != 0) {
		avdl_log_error("failed to do semantic analysis on '" BLU "%s" RESET "'", filename);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return -1;
	}

	// write results to destination file
	if (transpile_cglut(avdl_string_toCharPtr(&dstFilePath), game_node) != 0) {
		avdl_log_error("failed to transpile: %s -> %s", avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&dstFilePath));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return -1;
	}
	printf("avdl: transpiling - " YEL "%d%%" RESET "\r", (int)((float) (fileIndex)/filesTotal *100));

	avdl_string_clean(&srcFilePath);
	avdl_string_clean(&dstFilePath);

	return 0;
}

int avdl_transpile(struct AvdlSettings *avdl_settings) {

	printf("avdl: transpiling - " RED "0%%" RESET "\r");
	fflush(stdout);

	if ( Avdl_FileOp_ForFileInDirectory(avdl_settings->src_dir, transpile_file) != 0 ) {
		avdl_log_error("one or more files failed to transpile");
		return -1;
	}

	printf("avdl: transpiling - " GRN "100%%" RESET "\n");
	fflush(stdout);

	return 0;
}

int compile_file(const char *dirname, const char *filename, int fileIndex, int filesTotal) {

	// ignore `.` and `..`
	if (strcmp(filename, ".") == 0
	||  strcmp(filename, "..") == 0) {
		return 0;
	}

	// src file full path
	struct avdl_string srcFilePath;
	avdl_string_create(&srcFilePath, 1024);
	avdl_string_cat(&srcFilePath, dirname);
	avdl_string_cat(&srcFilePath, filename);
	if ( !avdl_string_isValid(&srcFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", dirname, filename, avdl_string_getError(&srcFilePath));
		avdl_string_clean(&srcFilePath);
		return -1;
	}

	// on android just copy src file to its destination
	if (avdl_platform_get() == AVDL_PLATFORM_ANDROID) {
		struct avdl_string androidFilePath;
		avdl_string_create(&androidFilePath, 1024);
		avdl_string_cat(&androidFilePath, "avdl_build_android/app/src/main/cpp/engine/");
		avdl_string_cat(&androidFilePath, filename);
		if ( !avdl_string_isValid(&androidFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s", "avdl_build_android/app/src/main/cpp/engine/", filename, avdl_string_getError(&androidFilePath));
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&androidFilePath);
			return -1;
		}
		file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&androidFilePath), 0);
		avdl_string_clean(&androidFilePath);
		return 0;
	}

	// dst file full path
	struct avdl_string dstFilePath;
	avdl_string_create(&dstFilePath, 1024);
	avdl_string_cat(&dstFilePath, avdl_string_toCharPtr(&srcFilePath));
	avdl_string_cat(&dstFilePath, ".o");
	if ( !avdl_string_isValid(&dstFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", dirname, filename, avdl_string_getError(&dstFilePath));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return -1;
	}

	// skip non-regular files (like directories)
	struct stat statbuf;
	if ( stat(avdl_string_toCharPtr(&srcFilePath), &statbuf) != 0 ) {
		avdl_log_error("Unable to stat file '%s': %s", avdl_string_toCharPtr(&srcFilePath), strerror(errno));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return -1;
	}

	// is regular file - do nothing
	if (Avdl_FileOp_IsRegStat(&statbuf)) {
	}
	// not supporting other file types - skip
	else {
		//printf("avdl error: Unsupported file type '%s' - skip\n", dir->d_name);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}

	// skip non-`.c` files
	if (strcmp(avdl_string_toCharPtr(&srcFilePath) +strlen(avdl_string_toCharPtr(&srcFilePath)) -2, ".c") != 0) {
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}

	// skip files already compiled (check last modified)
	// but if any header in `include/` has changed, compile everything
	if ( !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), avdl_string_toCharPtr(&srcFilePath))
	&&   !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), "include/") ) {
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}

	//printf("compiling %s\n", dir->d_name);

	// command string
	struct avdl_string commandString;
	avdl_string_create(&commandString, 1024);
	avdl_string_cat(&commandString, "gcc -O3 -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DAVDL_GAME_VERSION=\"\\\"");
	avdl_string_cat(&commandString, "0.0.0");
	avdl_string_cat(&commandString, "\\\"\" -DAVDL_GAME_REVISION=\"\\\"");
	avdl_string_cat(&commandString, "0");
	avdl_string_cat(&commandString, "\\\"\" -c -w ");
	// cengine headers
	avdl_string_cat(&commandString, " -I ");
	avdl_string_cat(&commandString, avdl_project_path);
	avdl_string_cat(&commandString, "/include ");

	avdl_string_cat(&commandString, avdl_string_toCharPtr(&srcFilePath));
	avdl_string_cat(&commandString, " -o ");
	avdl_string_cat(&commandString, avdl_string_toCharPtr(&dstFilePath));
	for (int i = 0; i < totalIncludeDirectories; i++) {
		avdl_string_cat(&commandString, " -I ");
		avdl_string_cat(&commandString, additionalIncludeDirectory[i]);
	}
	avdl_string_cat(&commandString, " -I include ");

	if ( !avdl_string_isValid(&commandString) ) {
		avdl_log_error("cannot construct path '%s%s': %s", dirname, filename, avdl_string_getError(&commandString));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		avdl_string_clean(&commandString);
		return -1;
	}
	//printf("avdl compile command: %s\n", buffer3);
	if (system(avdl_string_toCharPtr(&commandString))) {
		avdl_log_error("failed to compile file: " BLU "%s" RESET, filename);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		avdl_string_clean(&commandString);
		return -1;
	}

	printf("avdl: compiling - " YEL "%d%%" RESET "\r", (int)((float) (fileIndex)/filesTotal *100));
	fflush(stdout);

	avdl_string_clean(&srcFilePath);
	avdl_string_clean(&dstFilePath);
	avdl_string_clean(&commandString);
	return 0;
}

int avdl_compile(struct AvdlSettings *avdl_settings) {

	printf("avdl: compiling - " RED "0%%" RESET "\r");
	fflush(stdout);

	if ( Avdl_FileOp_ForFileInDirectory(cache_dir, compile_file) != 0) {
		avdl_log_error("one or more files failed to compile");
		return -1;
	}

	printf("avdl: compiling - " GRN "100%%" RESET "\n");
	fflush(stdout);

	return 0;
}

int avdl_compile_cengine(struct AvdlSettings *avdl_settings) {

	/*
	 * if not available, compile `cengine` and cache it
	 */
	char *outdir = ".avdl_cache/";

	// create .avdl_cache/cengine directory
	struct avdl_string cenginePath;
	avdl_string_create(&cenginePath, 1024);
	avdl_string_cat(&cenginePath, outdir);
	avdl_string_cat(&cenginePath, "cengine/");
	if ( !avdl_string_isValid(&cenginePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", outdir, "cengine/", avdl_string_getError(&cenginePath));
		avdl_string_clean(&cenginePath);
		return -1;
	}
	if (!is_dir(avdl_string_toCharPtr(&cenginePath))) {
		dir_create(avdl_string_toCharPtr(&cenginePath));
	}

	printf("avdl: compiling cengine - " RED "0%%" RESET "\r");
	fflush(stdout);
	char compile_command[DD_BUFFER_SIZE];
	for (int i = 0; i < cengine_files_total; i++) {

		strcpy(compile_command, "gcc -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DPKG_LOCATION=\\\"");
		if (avdl_settings->steam_mode && cengine_files[i].steam) {
			//printf("cengine steam file: %s\n", cengine_files[i].steam);
			//strcpy(compile_command, "g++ -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DPKG_LOCATION=\\\"");
			strcpy(compile_command, "g++ -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU ");
		}
		else
		if (cengine_files[i].main) {
			//printf("cengine file: %s\n", cengine_files[i].main);
			//strcpy(compile_command, "gcc -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DPKG_LOCATION=\\\"");
			strcpy(compile_command, "gcc -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU ");
		}
		// no file available for current settings
		else {
			continue;
		}

		// include the source file
		strcat(compile_command, avdl_settings->cengine_path);
		if (avdl_settings->steam_mode && cengine_files[i].steam) {
			strcat(compile_command, cengine_files[i].steam);
		}
		else
		if (cengine_files[i].main) {
			strcat(compile_command, cengine_files[i].main);
		}
		else {
			continue;
		}

		strcat(compile_command, " -DPKG_NAME=\"\\\"");
		strcat(compile_command, avdl_settings->project_name_code);
		strcat(compile_command, "\"\\\" ");

		// asset prefix
		if (avdl_settings->asset_prefix[0] != '\0') {
			strcat(compile_command, " -DGAME_ASSET_PREFIX=\"\\\"");
			strcat(compile_command, avdl_settings->asset_prefix);
			strcat(compile_command, "\"\\\" ");
		}

		if (avdl_settings->steam_mode) {
			strcat(compile_command, " -DAVDL_STEAM ");
		}
		strcat(compile_command, " -o ");
		//strcat(compile_command, buffer);
		//strcat(compile_command, "/");
		struct avdl_string cenginePathOut;
		avdl_string_create(&cenginePathOut, 1024);
		if (avdl_settings->steam_mode && cengine_files[i].steam) {
			avdl_string_cat(&cenginePathOut, outdir);
			avdl_string_cat(&cenginePathOut, "cengine/");
			avdl_string_cat(&cenginePathOut, cengine_files[i].steam);
			avdl_string_replaceEnding(&cenginePathOut, ".cpp", ".o");
		}
		else {
			avdl_string_cat(&cenginePathOut, outdir);
			avdl_string_cat(&cenginePathOut, "cengine/");
			avdl_string_cat(&cenginePathOut, cengine_files[i].main);
			avdl_string_replaceEnding(&cenginePathOut, ".c", ".o");
		}

		if ( !avdl_string_isValid(&cenginePathOut) ) {
			avdl_log_error("cannot construct path '%s%s%s': %s", outdir, "cengine/", cengine_files[i].steam, avdl_string_getError(&cenginePathOut));
			avdl_string_clean(&cenginePathOut);
			return -1;
		}
		strcat(compile_command, avdl_string_toCharPtr(&cenginePathOut));

		// cengine headers
		strcat(compile_command, " -I ");
		strcat(compile_command, avdl_settings->pkg_path);
		strcat(compile_command, "/include ");

		// cengine extra directories (mostly for custom dependencies)
		for (int i = 0; i < totalIncludeDirectories; i++) {
			strcat(compile_command, " -I ");
			strcat(compile_command, additionalIncludeDirectory[i]);
		}

		// skip files already compiled
		if ( Avdl_FileOp_DoesFileExist(avdl_string_toCharPtr(&cenginePathOut)) ) {
			//printf("skipping: %s\n", buffer);
			printf("avdl: compiling cengine - " YEL "%d%%" RESET "\r", (int)((float) (i+1)/cengine_files_total *100));
			fflush(stdout);
			continue;
		}

		//printf("cengine compile command: %s\n", compile_command);
		if (system(compile_command) != 0) {
			avdl_log_error("failed to compile cengine\n");
			avdl_string_clean(&cenginePath);
			return -1;
		}
////		if (!avdlQuietMode) {
			printf("avdl: compiling cengine - " YEL "%d%%" RESET "\r", (int)((float) (i+1)/cengine_files_total *100));
			fflush(stdout);
////		}
	}
////	if (!avdlQuietMode) {
		printf("avdl: compiling cengine - " GRN "100%%" RESET "\n");
////	}

	avdl_string_clean(&cenginePath);
	return 0;
}

static int create_executable_file(const char *filename, const char *content) {

	// make a `.sh` file to link executable with dependencies
	int fd = open(filename, O_RDWR | O_CREAT, 0777);
	if (fd == -1) {
		avdl_log_error("Unable to open '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	/*
	FILE *fsh = fdopen(fd, "w");
	if (!fsh) {
		printf("avdl error: Unable to open fd '%s': %s\n", filename, strerror(errno));
		return -1;
	}
	*/

	write(fd, content, strlen(content));
	//fclose(fsh);

	return 0;
}

int add_object_file(const char *dirname, const char *filename, int fileIndex, int filesTotal) {

	// ignore `.` and `..`
	if (strcmp(filename, ".") == 0
	||  strcmp(filename, "..") == 0) {
		return 0;
	}

	// src file full path
	struct avdl_string srcFilePath;
	avdl_string_create(&srcFilePath, 1024);
	avdl_string_cat(&srcFilePath, dirname);
	avdl_string_cat(&srcFilePath, filename);
	if ( !avdl_string_isValid(&srcFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", dirname, filename, avdl_string_getError(&srcFilePath));
		avdl_string_clean(&srcFilePath);
		return -1;
	}

	// skip non-regular files (like directories)
	struct stat statbuf;
	if (stat(avdl_string_toCharPtr(&srcFilePath), &statbuf) != 0) {
		avdl_log_error("Unable to stat file '%s': %s\n", avdl_string_toCharPtr(&srcFilePath), strerror(errno));
		avdl_string_clean(&srcFilePath);
		return -1;
	}

	// is regular file - add to link command
	if (Avdl_FileOp_IsRegStat(&statbuf)) {
		strcat(buffer, ".avdl_cache/");
		strcat(buffer, filename);
		strcat(buffer, ".c.o ");
	}
	// not supporting other file types - skip
	else {
		//printf("avdl error: Unsupported file type '%s' - skip\n", dir->d_name);
		avdl_string_clean(&srcFilePath);
		return 0;
	}

	/*
	// skip files already transpiled (check last modified)
	strcpy(buffer, dir->d_name);
	strcat(buffer, ".c");
	if ( faccessat(dst_dir, buffer, F_OK, 0) == 0 ) {
		struct stat statbuf2;
		if (fstatat(dst_dir, buffer, &statbuf2, 0) != 0) {
			printf("avdl error: Unable to stat file '%s/%s': %s\n", cache_dir, dir->d_name, strerror(errno));
			continue;
		}

		// transpiled file is same or newer (?) - skip transpilation
		if (difftime(statbuf2.st_mtime, statbuf.st_mtime) >= 0) {
			//printf("avdl src file not modified, skipping transpilation of '%s'\n", dir->d_name);
			continue;
		}
		printf("Last file src modification: %s\n", ctime(&statbuf.st_mtime));
		printf("Last file dst modification: %s\n", ctime(&statbuf2.st_mtime));

	}
	*/
	avdl_string_clean(&srcFilePath);
	return 0;
}

int avdl_link(struct AvdlSettings *avdl_settings) {

	printf("avdl: creating executable - " YEL "..." RESET "\r");

	// link the final executable

	char *outdir = "avdl_build/bin/";
	if (!is_dir("avdl_build")) {
		dir_create("avdl_build");
	}
	if (!is_dir(outdir)) {
		dir_create(outdir);
	}

	/*
	struct avdl_string linkCommand;
	avdl_string_create(&linkCommand, 1024);
	*/

	// prepare link command
	if (avdl_settings->steam_mode) {
		strcpy(buffer, "g++ -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU ");
	}
	else {
		strcpy(buffer, "gcc -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU ");
	}

	// add game files to link
////			for (int i = 0; i < input_file_total; i++) {
////				strcat(buffer, filename[i]);
////				strcat(buffer, " ");
////			}
	Avdl_FileOp_ForFileInDirectory(avdl_settings->src_dir, add_object_file);

	// add cengine files to link
	char tempDir[DD_BUFFER_SIZE];
	strcpy(tempDir, ".avdl_cache/cengine/");
	for (int i = 0; i < cengine_files_total; i++) {
		if (avdl_settings->steam_mode && cengine_files[i].steam) {
		}
		else
		if (cengine_files[i].main) {
		}
		else {
			continue;
		}
		strcat(buffer, tempDir);
		strcat(buffer, "/");
		if (avdl_settings->steam_mode && cengine_files[i].steam) {
			strcat(buffer, cengine_files[i].steam);
			buffer[strlen(buffer)-3] = 'o';
			buffer[strlen(buffer)-2] = '\0';
		}
		else {
			strcat(buffer, cengine_files[i].main);
			buffer[strlen(buffer)-1] = 'o';
		}
		strcat(buffer, " ");
	}

	// output file
	strcat(buffer, "-o ");
	strcat(buffer, outdir);
	strcat(buffer, "/");
	//strcat(buffer, gameName);
	//strcat(buffer, "avdl_game");
	strcat(buffer, avdl_settings->project_name_code);

	// link custom dependencies
	for (int i = 0; i < totalLibDirectories; i++) {
		strcat(buffer, " -L ");
		strcat(buffer, additionalLibDirectory[i]);
	}

	if (avdl_settings->standalone) {
		strcat(buffer, " -O3 -lm -l:libogg.so.0 -l:libopus.so.0 -l:libopusfile.so.0 -l:libpng16.so.16 -l:libSDL2-2.0.so.0 -l:libSDL2_mixer-2.0.so.0 -lpthread -lGL -l:libGLEW.so.2.2");
	}
	else {
		strcat(buffer, " -O3 -lm -logg -lopus -lopusfile -lpng -lSDL2 -lSDL2_mixer -lpthread -lGL -lGLEW");
	}

	if (avdl_settings->steam_mode) {
		strcat(buffer, " -lsteam_api ");
	}
	//printf("link command: %s\n", buffer);
	/*
	if ( !avdl_string_isValid(&linkCommand) ) {
		avdl_log_error("cannot construct link command: %s", avdl_string_getError(&linkCommand));
		avdl_string_clean(&linkCommand);
		return -1;
	}
	*/
	if (system(buffer)) {
		avdl_log_error("failed to create executable");
		return -1;
	}
			/*
	// on android put all object files in an android project
	if (avdl_platform_get() == AVDL_PLATFORM_ANDROID) {
		#if AVDL_IS_OS(AVDL_OS_WINDOWS)
		#else
		char *androidDir;
		if (outname) {
			androidDir = outname;
		}
		else {
			androidDir = "android";
		}

		if (create_android_directory(androidDir) < 0) {
			printf("avdl: error while linking\n");
			return -1;
		}

		// put all object files to android
		strcpy(buffer, androidDir);
		strcat(buffer, "/app/src/main/cpp/game/");
		dir_create(buffer);

		// in the destination, remove directory ("/something/myfile.c" -> "myfile.c")
		for (int i = 0; i < input_file_total; i++) {
			strcpy(buffer, androidDir);
			strcat(buffer, "/app/src/main/cpp/game/");
			char *rawFilename = filename[i];
			char *temp;
			while ((temp = strstr(rawFilename, "/"))) {
				rawFilename = temp+1;
			}
			strncat(buffer, rawFilename, 99);
			strcat(buffer, ".c");
			file_copy(filename[i], buffer, 0);
		}

		// folder to edit
		strcpy(buffer, androidDir);
		strcat(buffer, "/app/src/main/cpp/");
		int outDir = open(buffer, O_DIRECTORY);
		if (!outDir) {
			printf("avdl: can't open %s: %s\n", buffer, strerror(errno));
			return -1;
		}

		// files to swap in
		buffer[0] = '\0';
		for (int i = 0; i < input_file_total; i++) {
			strcat(buffer, "game/");
			char *rawFilename = filename[i];
			char *temp;
			while ((temp = strstr(rawFilename, "/"))) {
				rawFilename = temp+1;
			}
			strcat(buffer, rawFilename);
			strcat(buffer, ".c ");
		}

		// add in the avdl-compiled source files
		file_replace(outDir, "CMakeLists.txt.in", outDir, "CMakeLists.txt", "%AVDL_GAME_FILES%", buffer);
		close(outDir);

		// handle versioning
		strcpy(buffer, androidDir);
		strcat(buffer, "/app/");
		outDir = open(buffer, O_DIRECTORY);
		file_replace(outDir, "build.gradle.in", outDir, "build.gradle.in2", "%AVDL_PACKAGE_NAME%", gamePackageName);
		file_replace(outDir, "build.gradle.in2", outDir, "build.gradle.in3", "%AVDL_VERSION_CODE%", gameVersionCode);
		file_replace(outDir, "build.gradle.in3", outDir, "build.gradle", "%AVDL_VERSION_NAME%", gameVersion);
		close(outDir);

		if (gameIconFlat) {
			strcpy(buffer, androidDir);
			strcat(buffer, "/app/src/main/res/drawable/");
			strcat(buffer, gameIconFlat);
			file_copy(gameIconFlat, buffer, 0);
		}

		if (gameIconForeground) {
			strcpy(buffer, androidDir);
			strcat(buffer, "/app/src/main/res/drawable/");
			strcat(buffer, gameIconForeground);
			file_copy(gameIconForeground, buffer, 0);
		}

		if (gameIconBackground) {
			strcpy(buffer, androidDir);
			strcat(buffer, "/app/src/main/res/drawable/");
			strcat(buffer, gameIconBackground);
			file_copy(gameIconBackground, buffer, 0);
		}

		// project name
		strcpy(buffer, androidDir);
		strcat(buffer, "/app/src/main/res/values/");
		outDir = open(buffer, O_DIRECTORY);
		file_replace(outDir, "strings.xml.in", outDir, "strings.xml", "%AVDL_PROJECT_NAME%", gameName);
		close(outDir);
		#endif
	}
	*/
	char test_buffer[1024];
	strcpy(test_buffer, "./avdl_build/");
	strcat(test_buffer, avdl_settings->project_name_code);
	strcat(test_buffer, ".sh");

	char test_buffer2[1024];
	strcpy(test_buffer2, "LD_LIBRARY_PATH=./dependencies/ ./bin/");
	strcat(test_buffer2, avdl_settings->project_name_code);

	create_executable_file(test_buffer, test_buffer2);
	/*
	// stand-alone project
	if (avdl_pkg_IsDynamicLocation()) {
		if (avdl_pkg_IsDynamicDependencies()) {
			//
			// Stand-alone project with shared libraries
			//
			// ./bin/project
			//
			create_executable_file("./avdl_build/avdl_game.sh", "./bin/avdl_game");
		}
		else {
			//
			// Stand-alone project with static libraries
			//
			// LIB_PATH ./dependencies/ && ./bin/project
			//
			create_executable_file("./avdl_build/avdl_game.sh",
					"LD_LIBRARY_PATH=./dependencies/ ./bin/avdl_game");
		}
	}
	// installed project
	else {
		if (avdl_pkg_IsDynamicDependencies()) {
			//
			// Installed project with shared libraries
			//
			// no .sh - system will find the `bin/project` file
			//
		}
		else {
			//
			// Installed project with static libraries
			//
			// Should be invalid, would it make sense ?
			// ? project.sh references ./dependencies/ and ./bin/project
			//
			printf("Installable project with static libraries is not supported\n");
			return -1;
		}
	}
	*/

	printf("avdl: creating executable - " GRN "done" RESET "\n");
	return 0;
}

int asset_file(const char *dirname, const char *filename, int fileIndex, int filesTotal) {

	// ignore `.` and `..`
	if (strcmp(filename, ".") == 0
	||  strcmp(filename, "..") == 0) {
		return 0;
	}

	// src file full path
	struct avdl_string srcFilePath;
	avdl_string_create(&srcFilePath, 1024);
	avdl_string_cat(&srcFilePath, dirname);
	avdl_string_cat(&srcFilePath, filename);
	if ( !avdl_string_isValid(&srcFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", dirname, filename, avdl_string_getError(&srcFilePath));
		avdl_string_clean(&srcFilePath);
		return -1;
	}

	// on android, put assets in a specific directory
	if (avdl_platform_get() == AVDL_PLATFORM_ANDROID) {
		char *assetDir;

		if (strcmp(filename +strlen(filename) -4, ".ply") == 0
		||  strcmp(filename +strlen(filename) -4, ".ogg") == 0
		||  strcmp(filename +strlen(filename) -5, ".opus") == 0
		||  strcmp(filename +strlen(filename) -4, ".wav") == 0) {
			assetDir = "raw";
		}
		else
		if (strcmp(filename +strlen(filename) -4, ".bmp") == 0
		||  strcmp(filename +strlen(filename) -4, ".png") == 0) {
			assetDir = "drawable";
		}
		else {
			assetDir = "raw";
		}

		// android file full path
		struct avdl_string androidFilePath;
		avdl_string_create(&androidFilePath, 1024);
		avdl_string_cat(&androidFilePath, "avdl_build_android/");
		avdl_string_cat(&androidFilePath, "/app/src/main/res/");
		avdl_string_cat(&androidFilePath, assetDir);
		avdl_string_cat(&androidFilePath, "/");
		if ( !avdl_string_isValid(&androidFilePath) ) {
			avdl_log_error("cannot construct android file path: %s", avdl_string_getError(&androidFilePath));
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&androidFilePath);
			return -1;
		}
		dir_create(avdl_string_toCharPtr(&androidFilePath));
		strcat(avdl_string_toCharPtr(&androidFilePath), filename);

		file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&androidFilePath), 0);
		avdl_string_clean(&androidFilePath);

		avdl_string_clean(&srcFilePath);
		return 0;
	}

	char *outdir = "avdl_build/assets/";

	// dst file full path
	struct avdl_string dstFilePath;
	avdl_string_create(&dstFilePath, 1024);
	avdl_string_cat(&dstFilePath, outdir);
	avdl_string_cat(&dstFilePath, filename);
	if ( !avdl_string_isValid(&dstFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", outdir, filename, avdl_string_getError(&dstFilePath));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return -1;
	}

	// skip non-regular files (like directories)
	struct stat statbuf;
	if (stat(avdl_string_toCharPtr(&srcFilePath), &statbuf) != 0) {
		avdl_log_error("Unable to stat file '%s': %s\n", avdl_string_toCharPtr(&srcFilePath), strerror(errno));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return -1;
	}

	// is directory - skip - maybe recursive compilation at some point?
	if (Avdl_FileOp_IsDirStat(&statbuf)) {
		//printf("avdl skipping directory: %s\n", dir->d_name);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}
	else
	// is regular file - do nothing
	if (Avdl_FileOp_IsRegStat(&statbuf)) {
	}
	// not supporting other file types - skip
	else {
		//printf("avdl error: Unsupported file type '%s' - skip\n", dir->d_name);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}

	// skip files already compiled (check last modified)
	// but if any header in `include/` has changed, compile everything
	if ( !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), avdl_string_toCharPtr(&srcFilePath)) ) {
		//printf("avdl asset file not modified, skipping handling of '%s'\n", dir->d_name);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}
	//printf("handling %s\n", dir->d_name);

	/*
	 * Currently assets are just copy-pasted,
	 * however on a future version there will be more fine
	 * control of editing files to supported formats
	 * and throwing errors on unsupported formats.
	 */
	file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&dstFilePath), 0);

	printf("avdl: assets - " YEL "%d%%" RESET "\r", (int)((float) (fileIndex)/filesTotal *100));
	fflush(stdout);
	avdl_string_clean(&srcFilePath);
	avdl_string_clean(&dstFilePath);
	return 0;
}

// handle assets and put them in the final build
int avdl_assets(struct AvdlSettings *avdl_settings) {

	if (!is_dir("avdl_build")) {
		dir_create("avdl_build");
	}

	if (!is_dir("avdl_build/assets/")) {
		dir_create("avdl_build/assets/");
	}

	printf("avdl: assets - " RED "0%%" RESET "\r");
	fflush(stdout);

	Avdl_FileOp_ForFileInDirectory(avdl_settings->asset_dir, asset_file);

	printf("avdl: assets - " GRN "100%%" RESET "\n");
	fflush(stdout);

	return 0;
}

char big_buffer[2048];
int android_object_file(const char *dirname, const char *filename, int fileIndex, int filesTotal) {

	#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
	// ignore `.` and `..`
	if (strcmp(filename, ".") == 0
	||  strcmp(filename, "..") == 0) {
		return 0;
	}

	// only keep `.c` files
	if (strcmp(filename +strlen(filename) -2, ".c") != 0) {
		return 0;
	}

	// src file full path
	struct avdl_string srcFilePath;
	avdl_string_create(&srcFilePath, 1024);
	avdl_string_cat(&srcFilePath, dirname);
	avdl_string_cat(&srcFilePath, filename);
	if ( !avdl_string_isValid(&srcFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", dirname, filename, avdl_string_getError(&srcFilePath));
		avdl_string_clean(&srcFilePath);
		return -1;
	}

	strcat(big_buffer, "game/");
	strcat(big_buffer, filename);
	strcat(big_buffer, " ");

	// dst file full path
	struct avdl_string dstFilePath;
	avdl_string_create(&dstFilePath, 1024);
	avdl_string_cat(&dstFilePath, "avdl_build_android/app/src/main/cpp/game/");
	avdl_string_cat(&dstFilePath, filename);
	if ( !avdl_string_isValid(&dstFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", "avdl_build_android/app/src/main/cpp/game/", filename, avdl_string_getError(&dstFilePath));
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return -1;
	}

	file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&dstFilePath), 0);
	avdl_string_clean(&srcFilePath);
	avdl_string_clean(&dstFilePath);
	#endif

	return 0;
}

int avdl_android_object(struct AvdlSettings *avdl_settings) {

	#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
	// put all object files to android
	strcpy(buffer, "avdl_build_android/");
	strcat(buffer, "/app/src/main/cpp/game/");
	dir_create(buffer);

	big_buffer[0] = '\0';
	Avdl_FileOp_ForFileInDirectory(".avdl_cache/", android_object_file);

	// cpp directory
	struct avdl_string cppFilePath;
	avdl_string_create(&cppFilePath, 1024);
	avdl_string_cat(&cppFilePath, "avdl_build_android/");
	avdl_string_cat(&cppFilePath, "/app/src/main/cpp/");
	if ( !avdl_string_isValid(&cppFilePath) ) {
		avdl_log_error("cannot construct android cpp path: %s", avdl_string_getError(&cppFilePath));
		avdl_string_clean(&cppFilePath);
		return -1;
	}

	// folder to edit
	int outDir = open(avdl_string_toCharPtr(&cppFilePath), O_DIRECTORY);
	if (!outDir) {
		avdl_log_error("can't open %s: %s\n", avdl_string_toCharPtr(&cppFilePath), strerror(errno));
		avdl_string_clean(&cppFilePath);
		return -1;
	}

	// add in the avdl-compiled source files
	file_replace(outDir, "CMakeLists.txt.in", outDir, "CMakeLists.txt", "%AVDL_GAME_FILES%", big_buffer);
	close(outDir);

	avdl_string_clean(&cppFilePath);

	// handle versioning
	strcpy(buffer, "avdl_build_android/");
	strcat(buffer, "/app/");
	outDir = open(buffer, O_DIRECTORY);
	file_replace(outDir, "build.gradle.in", outDir, "build.gradle.in2", "%AVDL_PACKAGE_NAME%", avdl_settings->package);
	file_replace(outDir, "build.gradle.in2", outDir, "build.gradle.in3", "%AVDL_VERSION_CODE%", avdl_settings->version_code_str);
	file_replace(outDir, "build.gradle.in3", outDir, "build.gradle", "%AVDL_VERSION_NAME%", avdl_settings->version_name);
	close(outDir);

	strcpy(buffer, "avdl_build_android/");
	strcat(buffer, "/app/src/main/res/drawable/");
	strcat(buffer, avdl_settings->icon_path);
	file_copy(avdl_settings->icon_path, buffer, 0);

	strcpy(buffer, "avdl_build_android/");
	strcat(buffer, "/app/src/main/res/drawable/");
	strcat(buffer, avdl_settings->icon_foreground_path);
	file_copy(avdl_settings->icon_foreground_path, buffer, 0);

	strcpy(buffer, "avdl_build_android/");
	strcat(buffer, "/app/src/main/res/drawable/");
	strcat(buffer, avdl_settings->icon_background_path);
	file_copy(avdl_settings->icon_background_path, buffer, 0);

	// project name
	strcpy(buffer, "avdl_build_android/");
	strcat(buffer, "/app/src/main/res/values/");
	outDir = open(buffer, O_DIRECTORY);
	file_replace(outDir, "strings.xml.in", outDir, "strings.xml", "%AVDL_PROJECT_NAME%", avdl_settings->project_name);
	strcat(buffer, "strings.xml.in");
	file_remove(buffer);
	close(outDir);
	#endif
	return 0;
}
