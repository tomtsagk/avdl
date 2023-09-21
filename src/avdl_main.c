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
#include "avdl_settings.h"
#include "avdl_pkg.h"
#include "avdl_log.h"
#include "avdl_string.h"
#include "avdl_arguments.h"
#include "avdl_time.h"

#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <unistd.h>
#endif

char big_buffer[100000];

extern enum AVDL_PLATFORM avdl_platform_temp;

const char cache_dir[] = ".avdl_cache/";

extern float parsing_float;

char included_files[10][100];
int included_files_num = 0;

// buffer for general use
#define DD_BUFFER_SIZE 6000
char buffer[DD_BUFFER_SIZE];

// game node, parent of all nodes
struct ast_node *game_node;

int create_android_directory(const char *androidDirName);
int create_quest2_directory(const char *dirName);
int create_d3d11_directory(const char *dirName);
int collect_src_files(const char *dirname, const char *filename, int fileIndex, int filesTotal);
int collect_asset_files(const char *dirname, const char *filename, int fileIndex, int filesTotal);

char *cengine_files[] = {
	"avdl_assetManager.c",
	"avdl_data.c",
	"avdl_localisation.c",
	"avdl_localisation_cpp.cpp",
	"avdl_particle_system.c",
	"avdl_shaders.c",
	"dd_dynamic_array.c",
	"dd_filetomesh.c",
	"dd_fov.c",
	"dd_game.c",
	"dd_gamejolt.c",
	"dd_image.c",
	"dd_json.c",
	"dd_log.c",
	"dd_log_cpp.cpp",
	"dd_math.c",
	"dd_matrix.c",
	"dd_mesh.c",
	"dd_meshColour.c",
	"dd_meshTexture.c",
	"dd_mouse.c",
	"dd_sound.c",
	"avdl_whereami.c",
	"dd_string3d.c",
	"dd_vec3.c",
	"dd_vec4.c",
	"dd_world.c",
	"avdl_input.c",
	"avdl_physics.c",
	"avdl_collider.c",
	"avdl_collider_aabb.c",
	"avdl_collider_sphere.c",
	"avdl_rigidbody.c",
	"avdl_graphics_opengl.c",
	"avdl_graphics_direct3d11.cpp",
	"avdl_engine.c",
	"main.c",
	"avdl_steam.c",
	"avdl_steam_cpp.cpp",
	"main_cpp.cpp",
	"avdl_achievements.c",
	"avdl_achievements_steam.cpp",
	"avdl_multiplayer.c",
	"avdl_multiplayer_steam.cpp",
	"avdl_engine_cpp.cpp",
	"avdl_time.c",
	"avdl_webapi.c",
	"avdl_webapi_cpp.cpp",
	"avdl_ads.c",
	"avdl_font.c",
};
unsigned int cengine_files_total = sizeof(cengine_files) /sizeof(char *);

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
	"dd_sound.h",
	"avdl_whereami.h",
	"dd_string3d.h",
	"dd_vec2.h",
	"dd_vec3.h",
	"dd_vec4.h",
	"dd_world.h",
	"avdl_input.h",
	"avdl_achievements.h",
	"avdl_multiplayer.h",
	"avdl_physics.h",
	"avdl_collider.h",
	"avdl_collider_aabb.h",
	"avdl_collider_sphere.h",
	"avdl_rigidbody.h",
	"avdl_graphics.h",
	"avdl_engine.h",
	"avdl_time.h",
	"avdl_webapi.h",
	"avdl_ads.h",
	"avdl_font.h",
};
unsigned int cengine_headers_total = sizeof(cengine_headers) /sizeof(char *);

// compiler steps
int avdl_transpile(struct AvdlSettings *);
int avdl_compile(struct AvdlSettings *);
int avdl_compile_cengine(struct AvdlSettings *);
int avdl_link(struct AvdlSettings *);
int avdl_assets(struct AvdlSettings *);
int avdl_android_object(struct AvdlSettings *);
int avdl_quest2_object(struct AvdlSettings *);
int avdl_d3d11_object(struct AvdlSettings *);

const char *avdl_project_path;

// temp hack
struct AvdlSettings *avdl_settings_ptr = 0;
enum AVDL_PLATFORM avdl_target_platform;

// hide main when doing unit tests - temporary solution
#ifdef AVDL_UNIT_TEST
#define AVDL_MAIN avdl_main
#else
#define AVDL_MAIN main
#endif

// init data, parse, exit
int AVDL_MAIN(int argc, char *argv[]) {

	// measure time
	struct avdl_time clock;
	avdl_time_start(&clock);

	// project settings
	struct AvdlSettings avdl_settings;
	if (AvdlSettings_Create(&avdl_settings) != 0) {
		avdl_log_error("unable to initialise avdl settings");
		return -1;
	}
	avdl_settings_ptr = &avdl_settings;

	// temp solutions
	avdl_project_path = avdl_settings.pkg_path;

	int handle_return = avdl_arguments_handle(&avdl_settings, argc, argv);

	avdl_target_platform = avdl_settings.target_platform;

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

	// makefile generation
	if (avdl_settings.makefile_mode) {
		avdl_log("Generating makefile ...");
		struct avdl_string makefilePath;
		avdl_string_create(&makefilePath, 1024);
		avdl_string_cat(&makefilePath, avdl_pkg_GetProjectPath());
		avdl_string_cat(&makefilePath, "/share/avdl/templates/makefile");
		if ( !avdl_string_isValid(&makefilePath) ) {
			avdl_log_error("could not construct makefile path");
			avdl_string_clean(&makefilePath);
			return -1;
		}
		file_copy(avdl_string_toCharPtr(&makefilePath), ".makefile", 0);
		file_replace(0, ".makefile", 0, ".makefile1", "%AVDL_PROJECT_NAME%", avdl_settings.project_name);
		file_replace(0, ".makefile1", 0, ".makefile2", "%AVDL_PROJECT_NAME_CODE%", avdl_settings.project_name_code);
		file_replace(0, ".makefile2", 0, ".makefile3", "%AVDL_VERSION_NAME%", avdl_settings.version_name);
		file_replace(0, ".makefile3", 0, ".makefile4", "%AVDL_VERSION_CODE%", avdl_settings.version_code_str);
		file_replace(0, ".makefile4", 0, "makefile", "%AVDL_PACKAGE_NAME%", avdl_settings.package);
		file_remove(".makefile");
		file_remove(".makefile1");
		file_remove(".makefile2");
		file_remove(".makefile3");
		file_remove(".makefile4");
		avdl_log(BLU "makefile" RESET " for avdl project " BLU "%s" RESET " successfully generated" RESET, avdl_settings.project_name);
		return 0;
	}

	// cmake generation
	if (avdl_settings.cmake_mode) {

		struct avdl_string path;
		avdl_string_create(&path, 1024);
		avdl_string_cat(&path, avdl_pkg_GetProjectPath());
		avdl_string_cat(&path, "/share/avdl/templates/CMakeLists.txt");
		if ( !avdl_string_isValid(&path) ) {
			avdl_log_error("could not construct cmake path");
			avdl_string_clean(&path);
			return -1;
		}

		// collect avdl source
		struct avdl_string avdl_src;
		avdl_string_create(&avdl_src, 5024);
		for (int i = 0; i < cengine_files_total; i++) {
			avdl_string_cat(&avdl_src, "cengine/");
			avdl_string_cat(&avdl_src, cengine_files[i]);
			avdl_string_cat(&avdl_src, " ");
		}
		if ( !avdl_string_isValid(&avdl_src) ) {
			avdl_log_error("could not construct avdl_src path");
			avdl_string_clean(&avdl_src);
			return -1;
		}

		// collect avdl project source
		big_buffer[0] = '\0';
		if ( Avdl_FileOp_ForFileInDirectory(".avdl_cache", collect_src_files) != 0 ) {
			avdl_log_error("failed to collect source files");
			return -1;
		}

		// project data in cmake
		struct avdl_string cmake_data;
		avdl_string_create(&cmake_data, 5024);
		avdl_string_cat(&cmake_data, "set(AVDL_PROJECT_NAME \"");
		avdl_string_cat(&cmake_data, avdl_settings.project_name);
		avdl_string_cat(&cmake_data, "\")\n");
		avdl_string_cat(&cmake_data, "set(AVDL_PROJECT_NAME_CODE ");
		avdl_string_cat(&cmake_data, avdl_settings.project_name_code);
		avdl_string_cat(&cmake_data, ")\n");
		avdl_string_cat(&cmake_data, "set(AVDL_VERSION_NAME ");
		avdl_string_cat(&cmake_data, avdl_settings.version_name);
		avdl_string_cat(&cmake_data, ")\n");
		avdl_string_cat(&cmake_data, "set(AVDL_SRC ");
		avdl_string_cat(&cmake_data, avdl_string_toCharPtr(&avdl_src));
		avdl_string_cat(&cmake_data, big_buffer);
		avdl_string_cat(&cmake_data, avdl_settings.project_name_code);
		avdl_string_cat(&cmake_data, ".rc ");
		avdl_string_cat(&cmake_data, ")\n");
		avdl_string_cat(&cmake_data, "set(AVDL_ASSETS ");

		// collect avdl project assets
		big_buffer[0] = '\0';
		if ( Avdl_FileOp_ForFileInDirectory("assets", collect_asset_files) != 0 ) {
			avdl_log_error("failed to collect source files");
			return -1;
		}

		avdl_string_cat(&cmake_data, big_buffer);
		avdl_string_cat(&cmake_data, ")\n");
		if ( !avdl_string_isValid(&cmake_data) ) {
			avdl_log_error("could not construct cmake_data");
			avdl_string_clean(&cmake_data);
			return -1;
		}

		file_write("avdl_project.cmake", avdl_string_toCharPtr(&cmake_data), 0);
		file_copy(avdl_string_toCharPtr(&path), "CMakeLists.txt", 0);
		/*
		file_copy(avdl_string_toCharPtr(&path), ".cmake", 0);
		file_replace(0, ".cmake", 0, ".cmake1", "%AVDL_PROJECT_NAME%", avdl_settings.project_name);
		file_replace(0, ".cmake1", 0, ".cmake2", "%AVDL_VERSION_NAME%", avdl_settings.version_name);
		file_replace(0, ".cmake2", 0, ".cmake3", "%AVDL_SRC%", avdl_string_toCharPtr(&avdl_src));
		file_replace(0, ".cmake3", 0, "CMakeLists.txt", "%AVDL_PROJECT_SRC%", big_buffer);
		file_remove(".cmake");
		file_remove(".cmake1");
		file_remove(".cmake2");
		file_remove(".cmake3");
		*/
		avdl_log(BLU "cmake" RESET " for avdl project " BLU "%s" RESET " successfully generated" RESET, avdl_settings.project_name);
		return 0;
	}

	// normal directories
	if (!is_dir("avdl_build")) {
		dir_create("avdl_build");
	}

	// cache directory
	if (!is_dir(".avdl_cache")) {
		dir_create(".avdl_cache");
	}

	// from `.dd` to `.c`
	if ( avdl_transpile(&avdl_settings) != 0) {
		avdl_log_error("couldn't transpile project\n");
		return -1;
	}

	// android
	if (avdl_settings.target_platform == AVDL_PLATFORM_ANDROID) {
		create_android_directory("avdl_build_android");

		avdl_android_object(&avdl_settings);

		// handle assets
		if ( avdl_assets(&avdl_settings) != 0) {
			avdl_log_error("could not handle project assets for android\n");
			return -1;
		}

		avdl_log("avdl project " BLU "%s" RESET " prepared successfully for android at " BLU "./avdl_build_android/" RESET, avdl_settings.project_name);

		return 0;
	}
	// quest2
	else if (avdl_settings.target_platform == AVDL_PLATFORM_QUEST2) {
		create_quest2_directory("avdl_build_quest2");

		avdl_quest2_object(&avdl_settings);

		// handle assets
		if ( avdl_assets(&avdl_settings) != 0) {
			avdl_log_error("could not handle project assets for quest2\n");
			return -1;
		}

		avdl_log("avdl project " BLU "%s" RESET " prepared successfully for quest2 at " BLU "./avdl_build_quest2/" RESET, avdl_settings.project_name);

		return 0;
	}
	// d3d11
	else if (avdl_settings.target_platform == AVDL_PLATFORM_D3D11) {
		create_d3d11_directory("avdl_build_d3d11");

		//avdl_quest2_object(&avdl_settings);
		avdl_d3d11_object(&avdl_settings);

		// handle assets
		if ( avdl_assets(&avdl_settings) != 0) {
			avdl_log_error("could not handle project assets for d3d11\n");
			return -1;
		}

		avdl_log("avdl project " BLU "%s" RESET " prepared successfully for d3d11 at " BLU "./avdl_build_quest2/" RESET, avdl_settings.project_name);

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
		avdl_log_error("failed to handle assets");
		return -1;
	}

	// report results
	avdl_time_end(&clock);
	avdl_log("avdl: project " BLU "%s" RESET " compiled successfully at " BLU "./avdl_build/" RESET " in " BLU "%.3f" RESET " seconds", avdl_settings.project_name, avdl_time_getTimeDouble(&clock));

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

int create_quest2_directory(const char *dirName) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	#else
	int isDir = is_dir(dirName);
	if (isDir == 0) {
		dir_create(dirName);
		struct avdl_string cenginePath;
		avdl_string_create(&cenginePath, 1024);
		avdl_string_cat(&cenginePath, avdl_pkg_GetProjectPath());
		avdl_string_cat(&cenginePath, "/share/avdl/quest2");
		if ( !avdl_string_isValid(&cenginePath) ) {
			avdl_log_error("cannot construct path of quest2: %s", avdl_string_getError(&cenginePath));
			avdl_string_clean(&cenginePath);
			return -1;
		}
		dir_copy_recursive(0, avdl_string_toCharPtr(&cenginePath), 0, dirName);
		avdl_string_clean(&cenginePath);
	}
	else
	if (isDir < 0) {
		avdl_log_error("file '%s' not a directory", dirName);
		return -1;
	}
	#endif

	return 0;
}

int create_d3d11_directory(const char *dirName) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	#else
	int isDir = is_dir(dirName);
	if (isDir == 0) {
		dir_create(dirName);
		struct avdl_string cenginePath;
		avdl_string_create(&cenginePath, 1024);
		avdl_string_cat(&cenginePath, avdl_pkg_GetProjectPath());
		avdl_string_cat(&cenginePath, "/share/avdl/d3d11");
		if ( !avdl_string_isValid(&cenginePath) ) {
			avdl_log_error("cannot construct path of quest2: %s", avdl_string_getError(&cenginePath));
			avdl_string_clean(&cenginePath);
			return -1;
		}
		dir_copy_recursive(0, avdl_string_toCharPtr(&cenginePath), 0, dirName);
		avdl_string_clean(&cenginePath);
	}
	else
	if (isDir < 0) {
		avdl_log_error("file '%s' not a directory", dirName);
		return -1;
	}

	// avdl_project.vcxproj file path src
	struct avdl_string vcPath;
	avdl_string_create(&vcPath, 1024);
	avdl_string_cat(&vcPath, dirName);
	avdl_string_cat(&vcPath, "/");
	//avdl_string_cat(&vcPath, "/avdl_project.vcxproj.in");
	if ( !avdl_string_isValid(&vcPath) ) {
		avdl_log_error("cannot construct path for `avdl_project.vcxproj`: %s", avdl_string_getError(&vcPath));
		avdl_string_clean(&vcPath);
		return -1;
	}

	int outDir = open(avdl_string_toCharPtr(&vcPath), O_DIRECTORY);
	if (!outDir) {
		printf("avdl: can't open %s: %s\n", avdl_string_toCharPtr(&vcPath), strerror(errno));
		return -1;
	}

	// collect headers
	struct avdl_string avdlHeaders;
	avdl_string_create(&avdlHeaders, 4000);
	for (int i = 0; i < cengine_headers_total; i++) {
		avdl_string_cat(&avdlHeaders, "    <ClInclude Include=\"avdl_src/");
		avdl_string_cat(&avdlHeaders, cengine_headers[i]);
		avdl_string_cat(&avdlHeaders, "\"/>\n");
	}
	if ( !avdl_string_isValid(&avdlHeaders) ) {
		avdl_log_error("cannot construct path for d3d11 avdl headers: %s", avdl_string_getError(&avdlHeaders));
		avdl_log_error("chars in string: %d/%d", avdlHeaders.errorCharacters, avdlHeaders.maxCharacters);
		avdl_string_clean(&avdlHeaders);
		return -1;
	}
	file_replace(outDir, "avdl_project.vcxproj.in",
		outDir, "avdl_project.vcxproj.in2",
		"%AVDL_HEADERS%", avdl_string_toCharPtr(&avdlHeaders)
	);

	// collect src
	struct avdl_string avdlSrc;
	avdl_string_create(&avdlSrc, 10000);
	for (int i = 0; i < cengine_files_total; i++) {
		avdl_string_cat(&avdlSrc, "    <ClCompile Include=\"avdl_src/");
		avdl_string_cat(&avdlSrc, cengine_files[i]);
		avdl_string_cat(&avdlSrc, "\">\n");
		avdl_string_cat(&avdlSrc, "      <CompileAsWinRT>false</CompileAsWinRT>\n");
		avdl_string_cat(&avdlSrc, "      <PrecompiledHeader>NotUsing</PrecompiledHeader>\n");
		avdl_string_cat(&avdlSrc, "    </ClCompile>\n");
	}
	if ( !avdl_string_isValid(&avdlSrc) ) {
		avdl_log_error("cannot construct path for d3d11 avdl src: %s", avdl_string_getError(&avdlSrc));
		avdl_log_error("chars in string: %d/%d", avdlSrc.errorCharacters, avdlHeaders.maxCharacters);
		avdl_string_clean(&avdlSrc);
		return -1;
	}
	file_replace(outDir, "avdl_project.vcxproj.in2",
		outDir, "avdl_project.vcxproj.in3",
		"%AVDL_SRC%", avdl_string_toCharPtr(&avdlSrc)
	);

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
	// but compile everything if a header file has changed
	if ( avdl_settings_ptr->use_cache
	&&   !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), avdl_string_toCharPtr(&srcFilePath))
	&&   !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), "include/") ) {
		//printf("avdl src file not modified, skipping transpilation of '%s' -> '%s'\n", buffer, buffer2);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}
	//printf("transpiling %s to %s\n", buffer, buffer2);

	included_files_num = 0;

	// TODO: Find a better way to do this
	avdl_platform_temp = avdl_settings_ptr->target_platform;

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
	if (avdl_settings_ptr->target_platform == AVDL_PLATFORM_ANDROID) {
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
	if ( avdl_settings_ptr->use_cache
	&&   !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), avdl_string_toCharPtr(&srcFilePath))
	&&   !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), "include/") ) {
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		return 0;
	}

	//printf("compiling %s\n", dir->d_name);

	// command string
	struct avdl_string commandString;
	avdl_string_create(&commandString, 1024);
	avdl_string_cat(&commandString, "gcc -O3 -DGLEW_NO_GLU -DAVDL_GAME_VERSION=\"\\\"");
	avdl_string_cat(&commandString, avdl_settings_ptr->version_name);
	avdl_string_cat(&commandString, "\\\"\" -DAVDL_GAME_REVISION=\"\\\"");
	avdl_string_cat(&commandString, "0");
	avdl_string_cat(&commandString, "\\\"\" -c -w ");
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	avdl_string_cat(&commandString, " -DAVDL_WINDOWS ");
	#elif AVDL_IS_OS(AVDL_OS_LINUX)
	avdl_string_cat(&commandString, " -DAVDL_LINUX ");
	#endif
	// cengine headers
	avdl_string_cat(&commandString, " -I ");
	avdl_string_cat(&commandString, avdl_project_path);
	avdl_string_cat(&commandString, "/include ");
	avdl_string_cat(&commandString, " -I /usr/include/freetype2 ");

	avdl_string_cat(&commandString, avdl_string_toCharPtr(&srcFilePath));
	avdl_string_cat(&commandString, " -o ");
	avdl_string_cat(&commandString, avdl_string_toCharPtr(&dstFilePath));
	for (int i = 0; i < avdl_settings_ptr->total_include_directories; i++) {
		avdl_string_cat(&commandString, " -I ");
		avdl_string_cat(&commandString, avdl_settings_ptr->additional_include_directory[i]);
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

		struct avdl_string cEngFile;
		avdl_string_create(&cEngFile, 1024);
		avdl_string_cat(&cEngFile, cengine_files[i]);
		if (avdl_string_endsIn(&cEngFile, ".cpp")) {
			strcpy(compile_command, "g++ -w -c -DGLEW_NO_GLU ");
		}
		else {
			strcpy(compile_command, "gcc -w -c -DGLEW_NO_GLU ");
		}
		avdl_string_clean(&cEngFile);

		#if AVDL_IS_OS(AVDL_OS_WINDOWS)
		strcat(compile_command, " -DAVDL_WINDOWS ");
		#elif AVDL_IS_OS(AVDL_OS_LINUX)
		strcat(compile_command, " -DAVDL_LINUX ");
		#endif

		// include the source file
		strcat(compile_command, avdl_settings->cengine_path);
		strcat(compile_command, cengine_files[i]);

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
		avdl_string_cat(&cenginePathOut, outdir);
		avdl_string_cat(&cenginePathOut, "cengine/");
		avdl_string_cat(&cenginePathOut, cengine_files[i]);
		if (avdl_string_endsIn(&cenginePathOut, ".cpp")) {
			avdl_string_replaceEnding(&cenginePathOut, ".cpp", ".o");
		}
		else {
			avdl_string_replaceEnding(&cenginePathOut, ".c", ".o");
		}

		if ( !avdl_string_isValid(&cenginePathOut) ) {
			avdl_log_error("cannot construct path '%s%s%s': %s", outdir, "cengine/", cengine_files[i], avdl_string_getError(&cenginePathOut));
			avdl_string_clean(&cenginePathOut);
			return -1;
		}
		strcat(compile_command, avdl_string_toCharPtr(&cenginePathOut));

		// cengine headers
		strcat(compile_command, " -I ");
		strcat(compile_command, avdl_settings->pkg_path);
		strcat(compile_command, "/include ");
		strcat(compile_command, " -I /usr/include/freetype2 ");

		// cengine extra directories (mostly for custom dependencies)
		for (int i = 0; i < avdl_settings_ptr->total_include_directories; i++) {
			strcat(compile_command, " -I ");
			strcat(compile_command, avdl_settings_ptr->additional_include_directory[i]);
		}

		// skip files already compiled
		if ( avdl_settings_ptr->use_cache && Avdl_FileOp_DoesFileExist(avdl_string_toCharPtr(&cenginePathOut)) ) {
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

	// prepare link command
	struct avdl_string link_cmd;
	avdl_string_create(&link_cmd, 4096);
	if (avdl_settings->cpp_mode) {
		avdl_string_cat(&link_cmd, "g++ -DGLEW_NO_GLU ");
	}
	else {
		avdl_string_cat(&link_cmd, "gcc -DGLEW_NO_GLU ");
	}

	// possibly not needed when linking
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	avdl_string_cat(&link_cmd, " -DAVDL_WINDOWS ");
	#elif AVDL_IS_OS(AVDL_OS_LINUX)
	avdl_string_cat(&link_cmd, " -DAVDL_LINUX ");
	#endif

	// add game files to link
	buffer[0] = '\0';
	Avdl_FileOp_ForFileInDirectory(avdl_settings->src_dir, add_object_file);
	avdl_string_cat(&link_cmd, buffer);

	// add cengine files to link
	char tempDir[DD_BUFFER_SIZE];
	strcpy(tempDir, ".avdl_cache/cengine/");
	for (int i = 0; i < cengine_files_total; i++) {
		avdl_string_cat(&link_cmd, tempDir);
		avdl_string_cat(&link_cmd, "/");

		struct avdl_string tempfile;
		avdl_string_create(&tempfile, 1024);
		avdl_string_cat(&tempfile, cengine_files[i]);
		if (avdl_string_endsIn(&tempfile, ".cpp")) {
			avdl_string_replaceEnding(&tempfile, ".cpp", ".o");
		}
		else
		if (avdl_string_endsIn(&tempfile, ".c")) {
			avdl_string_replaceEnding(&tempfile, ".c", ".o");
		}
		avdl_string_cat(&link_cmd, avdl_string_toCharPtr(&tempfile));
		avdl_string_clean(&tempfile);
		avdl_string_cat(&link_cmd, " ");
	}

	// output file
	avdl_string_cat(&link_cmd, "-o ");
	avdl_string_cat(&link_cmd, outdir);
	avdl_string_cat(&link_cmd, "/");
	avdl_string_cat(&link_cmd, avdl_settings->project_name_code);

	// link custom dependencies
	for (int i = 0; i < avdl_settings->total_lib_directories; i++) {
		avdl_string_cat(&link_cmd, " -L ");
		avdl_string_cat(&link_cmd, avdl_settings->additional_lib_directory[i]);
	}

	if (avdl_settings->standalone) {
		avdl_string_cat(&link_cmd, " -O3 -lm -l:libogg.so.0 -l:libpng16.so.16 -l:libSDL2-2.0.so.0 -l:libSDL2_mixer-2.0.so.0 -lpthread -lGL -l:libGLEW.so.2.2 -l:libfreetype.so.6");
	}
	else {
		avdl_string_cat(&link_cmd, " -O3 -lm -logg -lpng -lSDL2 -lSDL2_mixer -lpthread -lGL -lGLEW -lfreetype");
	}

	if (avdl_settings->steam_mode) {
		avdl_string_cat(&link_cmd, " -lsteam_api ");
	}
	//printf("link command: %s\n", buffer);
	if ( !avdl_string_isValid(&link_cmd) ) {
		avdl_log_error("failed to construct link command");
		avdl_string_clean(&link_cmd);
		return -1;
	}
	if (system(avdl_string_toCharPtr(&link_cmd))) {
		avdl_log_error("failed to create executable");
		avdl_string_clean(&link_cmd);
		return -1;
	}
	avdl_string_clean(&link_cmd);
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
	strcat(test_buffer2, " $@");

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

	// sanity check
	for (const char *p = filename; p[0] != '\0'; p++) {
		if (p[0] == '-') {
			avdl_log_error("filename contains invalid character '-': " BLU "%s" RESET, filename);
			return -1;
		}
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
	if (avdl_target_platform == AVDL_PLATFORM_ANDROID) {
		char *assetDir;

		if (strcmp(filename +strlen(filename) -4, ".ogg") == 0
		||  strcmp(filename +strlen(filename) -4, ".wav") == 0) {
			assetDir = "res/raw";
		}
		else
		if (strcmp(filename +strlen(filename) -4, ".bmp") == 0
		||  strcmp(filename +strlen(filename) -4, ".png") == 0) {
			assetDir = "res/drawable";
		}
		else {
			assetDir = "assets";
		}

		// android file full path
		struct avdl_string androidFilePath;
		avdl_string_create(&androidFilePath, 1024);
		avdl_string_cat(&androidFilePath, "avdl_build_android/");
		avdl_string_cat(&androidFilePath, "/app/src/main/");
		avdl_string_cat(&androidFilePath, assetDir);
		avdl_string_cat(&androidFilePath, "/");
		if ( !avdl_string_isValid(&androidFilePath) ) {
			avdl_log_error("cannot construct android file path: %s", avdl_string_getError(&androidFilePath));
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&androidFilePath);
			return -1;
		}
		dir_create(avdl_string_toCharPtr(&androidFilePath));
		avdl_string_cat(&androidFilePath, filename);

		file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&androidFilePath), 0);
		avdl_string_clean(&androidFilePath);

		avdl_string_clean(&srcFilePath);
		return 0;
	}
	else
	// on quest2, put assets in a specific directory
	if (avdl_target_platform == AVDL_PLATFORM_QUEST2) {
		char *assetDir = "";

		if (strcmp(filename +strlen(filename) -4, ".ogg") == 0
		||  strcmp(filename +strlen(filename) -4, ".wav") == 0) {
			assetDir = "res/raw";
		}
		else
		if (strcmp(filename +strlen(filename) -4, ".bmp") == 0
		||  strcmp(filename +strlen(filename) -4, ".png") == 0) {
			assetDir = "res/drawable";
		}
		else {
			assetDir = "assets";
		}

		// android file full path
		struct avdl_string androidFilePath;
		avdl_string_create(&androidFilePath, 1024);
		avdl_string_cat(&androidFilePath, "avdl_build_quest2/");
		avdl_string_cat(&androidFilePath, assetDir);
		avdl_string_cat(&androidFilePath, "/");
		if ( !avdl_string_isValid(&androidFilePath) ) {
			avdl_log_error("cannot construct quest2 file path: %s", avdl_string_getError(&androidFilePath));
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&androidFilePath);
			return -1;
		}
		dir_create(avdl_string_toCharPtr(&androidFilePath));
		avdl_string_cat(&androidFilePath, filename);

		file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&androidFilePath), 0);
		avdl_string_clean(&androidFilePath);

		avdl_string_clean(&srcFilePath);
		return 0;
	}
	else
	// on d3d11, put assets in a specific directory
	if (avdl_target_platform == AVDL_PLATFORM_D3D11) {
		char *assetDir = "Assets";

		// d3d11 file full path
		struct avdl_string d3d11FilePath;
		avdl_string_create(&d3d11FilePath, 1024);
		avdl_string_cat(&d3d11FilePath, "avdl_build_d3d11/");
		avdl_string_cat(&d3d11FilePath, assetDir);
		avdl_string_cat(&d3d11FilePath, "/");
		if ( !avdl_string_isValid(&d3d11FilePath) ) {
			avdl_log_error("cannot construct d3d11 file path: %s", avdl_string_getError(&d3d11FilePath));
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&d3d11FilePath);
			return -1;
		}
		dir_create(avdl_string_toCharPtr(&d3d11FilePath));
		avdl_string_cat(&d3d11FilePath, filename);

		file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&d3d11FilePath), 0);
		avdl_string_clean(&d3d11FilePath);

		avdl_string_clean(&srcFilePath);

		if (strcmp(filename +strlen(filename) -4, ".png") == 0) {
			strcat(big_buffer, "  <ItemGroup>\n");
			strcat(big_buffer, "    <ImageContentTask Include=\"Assets/");
			strcat(big_buffer, filename);
			strcat(big_buffer, "\">\n");
			strcat(big_buffer, "      <FileType>Image</FileType>\n");
			strcat(big_buffer, "      <DestinationFolders>$(OutDir)/assets</DestinationFolders>\n");
			strcat(big_buffer, "      <ContentOutput >$(OutDir)/Assets/%(Filename).dds</ContentOutput>\n");
			strcat(big_buffer, "    </ImageContentTask>\n");
			strcat(big_buffer, "  </ItemGroup>\n");
		}
		else
		if (strcmp(filename +strlen(filename) -4, ".ply") == 0) {
			strcat(big_buffer, "  <ItemGroup>\n");
			strcat(big_buffer, "    <CopyFileToFolders Include=\"Assets/");
			strcat(big_buffer, filename);
			strcat(big_buffer, "\">\n");
			strcat(big_buffer, "      <FileType>Document</FileType>\n");
			strcat(big_buffer, "      <DestinationFolders>$(OutDir)/assets</DestinationFolders>\n");
			strcat(big_buffer, "    </CopyFileToFolders>\n");
			strcat(big_buffer, "  </ItemGroup>\n");
		}
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
	if ( avdl_settings_ptr->use_cache && !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), avdl_string_toCharPtr(&srcFilePath)) ) {
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

	big_buffer[0] = '\0';
	if (Avdl_FileOp_ForFileInDirectory(avdl_settings->asset_dir, asset_file) != 0) {
		return -1;
	}

	int outDir = open("avdl_build_d3d11/", O_DIRECTORY);
	if (!outDir) {
		printf("avdl: can't open %s: %s\n", "avdl_build_d3d11/", strerror(errno));
		return -1;
	}

	file_replace(outDir, "avdl_project.vcxproj.in4",
		outDir, "avdl_project.vcxproj.in5",
		"%AVDL_PROJECT_ASSETS%", big_buffer
	);

	printf("avdl: assets - " GRN "100%%" RESET "\n");
	fflush(stdout);

	return 0;
}

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

int quest2_object_file(const char *dirname, const char *filename, int fileIndex, int filesTotal) {

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

	strcat(big_buffer, "../../../src/");
	strcat(big_buffer, filename);
	strcat(big_buffer, " ");

	// dst file full path
	struct avdl_string dstFilePath;
	avdl_string_create(&dstFilePath, 1024);
	avdl_string_cat(&dstFilePath, "avdl_build_quest2/src/");
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

int d3d11_object_file(const char *dirname, const char *filename, int fileIndex, int filesTotal) {

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

	strcat(big_buffer, "    <ClCompile Include=\"src/");
	strcat(big_buffer, filename);
	strcat(big_buffer, "\">\n");
	strcat(big_buffer, "      <CompileAsWinRT>false</CompileAsWinRT>\n");
	strcat(big_buffer, "      <PrecompiledHeader>NotUsing</PrecompiledHeader>\n");
	strcat(big_buffer, "    </ClCompile>\n");

	// dst file full path
	struct avdl_string dstFilePath;
	avdl_string_create(&dstFilePath, 1024);
	avdl_string_cat(&dstFilePath, "avdl_build_d3d11/src/");
	avdl_string_cat(&dstFilePath, filename);
	if ( !avdl_string_isValid(&dstFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", "avdl_build_d3d11/src/", filename, avdl_string_getError(&dstFilePath));
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
	file_replace(outDir, "CMakeLists.txt.in", outDir, "CMakeLists.txt.in2", "%AVDL_GAME_FILES%", big_buffer);

	// add C flags
	{
		struct avdl_string cflags;
		avdl_string_create(&cflags, 1024);

		// admob ads
		if (avdl_settings->admob_ads) {
			avdl_string_cat(&cflags, " -DAVDL_ADMOB ");
		}

		// game version
		avdl_string_cat(&cflags, " -DAVDL_GAME_VERSION=\\\\\\\"");
		avdl_string_cat(&cflags, avdl_settings->version_name);
		avdl_string_cat(&cflags, "\\\\\\\" ");

		if (!avdl_string_isValid(&cflags)) {
			avdl_log_error("unable to construct cflags for android: %s", avdl_string_getError(&cflags));
			avdl_string_clean(&cflags);
			return -1;
		}
		file_replace(outDir, "CMakeLists.txt.in2", outDir, "CMakeLists.txt", "%AVDL_C_FLAGS%", avdl_string_toCharPtr(&cflags));
	}

	close(outDir);

	avdl_string_clean(&cppFilePath);

	// handle versioning
	strcpy(buffer, "avdl_build_android/");
	strcat(buffer, "/app/");
	outDir = open(buffer, O_DIRECTORY);
	file_replace(outDir, "build.gradle.in", outDir, "build.gradle.in2", "%AVDL_PACKAGE_NAME%", avdl_settings->package);
	file_replace(outDir, "build.gradle.in2", outDir, "build.gradle.in3", "%AVDL_VERSION_CODE%", avdl_settings->version_code_str);
	file_replace(outDir, "build.gradle.in3", outDir, "build.gradle", "%AVDL_VERSION_NAME%", avdl_settings->version_name);
	file_remove("avdl_build_android/app/build.gradle.in");
	file_remove("avdl_build_android/app/build.gradle.in2");
	file_remove("avdl_build_android/app/build.gradle.in3");
	file_remove("avdl_build_android/app/build.gradle.in.googleplay");
	close(outDir);

	// add dependencies if needed
	if (avdl_settings->googleplay_mode || avdl_settings->admob_ads) {
		file_write("avdl_build_android/app/build.gradle", "\ndependencies {\n", 1);
		if (avdl_settings->googleplay_mode) {
			file_write("avdl_build_android/app/build.gradle", "\timplementation 'com.google.android.gms:play-services-games-v2:17.0.0'\n", 1);
		}

		if (avdl_settings->admob_ads) {
			file_write("avdl_build_android/app/build.gradle", "\timplementation 'com.google.android.gms:play-services-ads:22.1.0'\n", 1);
		}
		file_write("avdl_build_android/app/build.gradle", "}\n", 1);
	}

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

	// strings
	struct avdl_string values_file;
	avdl_string_create(&values_file, 2048 *2);
	avdl_string_cat(&values_file, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	avdl_string_cat(&values_file, "<resources>\n");
	avdl_string_cat(&values_file, "	<string name=\"app_name\">");
	avdl_string_cat(&values_file, avdl_settings->project_name);
	avdl_string_cat(&values_file, "</string>\n");
	if (avdl_settings->googleplay_mode) {
		avdl_string_cat(&values_file, "	<string translatable=\"false\" name=\"game_services_project_id\">");
		avdl_string_cat(&values_file, avdl_settings->googleplay_id);
		avdl_string_cat(&values_file, "</string>\n");

		for (int i = 0; i < avdl_settings->googleplay_achievement_count; i++) {
			avdl_string_cat(&values_file, "	<string translatable=\"false\" name=\"");
			avdl_string_cat(&values_file, avdl_settings->googleplay_achievement[i].api);
			avdl_string_cat(&values_file, "\">");
			avdl_string_cat(&values_file, avdl_settings->googleplay_achievement[i].id);
			avdl_string_cat(&values_file, "</string>\n");
		}

	}

	// admob project id
	if (avdl_settings->admob_ads) {
		avdl_string_cat(&values_file, "	<string translatable=\"false\" name=\"game_services_ad_project_id\">");
		avdl_string_cat(&values_file, avdl_settings->admob_ads_id);
		avdl_string_cat(&values_file, "</string>\n");

		// fullscreen ad id
		if (avdl_settings->admob_ads_fullscreen) {
			avdl_string_cat(&values_file, "	<string translatable=\"false\" name=\"game_services_fullscreen_ad_id\">");
			avdl_string_cat(&values_file, avdl_settings->admob_ads_fullscreen_id);
			avdl_string_cat(&values_file, "</string>\n");
		}

		// rewards ad id
		if (avdl_settings->admob_ads_rewarded) {
			avdl_string_cat(&values_file, "	<string translatable=\"false\" name=\"game_services_rewarded_ad_id\">");
			avdl_string_cat(&values_file, avdl_settings->admob_ads_rewarded_id);
			avdl_string_cat(&values_file, "</string>\n");
		}
	}

	avdl_string_cat(&values_file, "</resources>\n");
	if (!avdl_string_isValid(&values_file)) {
		avdl_log_error("could not construct `strings.xml`: %s", avdl_string_getError(&values_file));
		avdl_log_error("was trying for %d", values_file.errorCharacters);
		return -1;
	}
	if (!is_dir("avdl_build_android/app/src/main/res/values/")) {
		dir_create("avdl_build_android/app/src/main/res/values/");
	}
	file_write("avdl_build_android/app/src/main/res/values/strings.xml", avdl_string_toCharPtr(&values_file), 0);

	// collect metadata and permissions
	struct avdl_string metadata;
	avdl_string_create(&metadata, 1024);

	struct avdl_string permissions;
	avdl_string_create(&permissions, 1024);

	struct avdl_string ads_imports;
	avdl_string_create(&ads_imports, 1024);

	struct avdl_string ads_declarations;
	avdl_string_create(&ads_declarations, 1024);

	struct avdl_string ads_init;
	avdl_string_create(&ads_init, 1024);

	struct avdl_string ads_functions;
	avdl_string_create(&ads_functions, 2048 *2);

	// modify Android Manifest based on google play mode or not
	if (avdl_settings->googleplay_mode) {
		avdl_string_cat(&metadata,
			"\n\t\t<!-- Google Play Services -->\n"
			"\t\t<meta-data android:name=\"com.google.android.gms.games.APP_ID\"\n"
			"\t\t	android:value=\"@string/game_services_project_id\"\n"
			"\t\t/>\n"
		);
	}

	if (avdl_settings->admob_ads) {
		avdl_string_cat(&metadata,
			"\n\t\t<!-- Admob Ads -->\n"
			"\t\t<meta-data\n"
			"\t\t	android:name=\"com.google.android.gms.ads.APPLICATION_ID\"\n"
			"\t\t	android:value=\"@string/game_services_ad_project_id\"\n"
			"\t\t/>\n"
		);

		avdl_string_cat(&permissions,
			"\n\t<!-- Admob Ads -->\n"
			"\t<uses-permission android:name=\"com.google.android.gms.permission.AD_ID\"/>\n"
		);

		avdl_string_cat(&ads_imports,
			"// ads\n"
			"import com.google.android.gms.ads.MobileAds;\n"
			"import com.google.android.gms.ads.initialization.InitializationStatus;\n"
			"import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;\n"
			"import com.google.android.gms.ads.interstitial.InterstitialAd;\n"
			"import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;\n"
			"import com.google.android.gms.ads.AdRequest;\n"
			"import com.google.android.gms.ads.LoadAdError;\n"
			"import com.google.android.gms.ads.FullScreenContentCallback;\n"
			"import com.google.android.gms.ads.AdError;\n"

			"import com.google.android.gms.ads.rewarded.RewardedAd;\n"
			"import com.google.android.gms.ads.rewarded.RewardItem;\n"
			"import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;\n"
			"import com.google.android.gms.ads.OnUserEarnedRewardListener;\n"
		);

		avdl_string_cat(&ads_init,
			"\n\t\tMobileAds.initialize(this, new OnInitializationCompleteListener() {\n"
			"\t\t	@Override\n"
			"\t\t	public void onInitializationComplete(InitializationStatus initializationStatus) {\n"
			"\t\t	}\n"
			"\t\t});\n"
		);

		// fullscreen ads
		if (avdl_settings->admob_ads_fullscreen) {
			avdl_string_cat(&ads_declarations,
				"\n\tpublic static InterstitialAd mInterstitialAd;\n"
			);
			avdl_string_cat(&ads_functions,
				"public void loadFullscreenAd(int X) {\n"
				"	// ad already loaded - skip\n"
				"	if ( mInterstitialAd != null ) {\n"
				"		return;\n"
				"	}\n"
				"	runOnUiThread(() -> {\n"
				"		AdRequest adRequest = new AdRequest.Builder().build();\n"
				"		InterstitialAd.load(AvdlActivity.activity, getString(R.string.game_services_fullscreen_ad_id), adRequest,\n"
				"			new InterstitialAdLoadCallback() {\n"
				"				@Override\n"
				"				public void onAdLoaded(InterstitialAd interstitialAd) {\n"
				"					AvdlActivity.mInterstitialAd = interstitialAd;\n"
				"				}\n"
				"				@Override\n"
				"				public void onAdFailedToLoad(LoadAdError loadAdError) {\n"
				"					AvdlActivity.mInterstitialAd = null;\n"
				"				}\n"
				"			}\n"
				"		);\n"
				"	});\n"
				"}\n"
				"public void showFullscreenAd(int X) {\n"
				"	// no ad loaded to show\n"
				"	if (AvdlActivity.mInterstitialAd == null) {\n"
				"		return;\n"
				"	}\n"
				"	runOnUiThread(() -> {\n"
				"		if (AvdlActivity.mInterstitialAd != null) {\n"
				"			AvdlActivity.mInterstitialAd.setFullScreenContentCallback(new FullScreenContentCallback() {\n"
				"				// ad clicked\n"
				"				@Override\n"
				"				public void onAdClicked() {\n"
				"				}\n"
				"				// ad dismissed\n"
				"				@Override\n"
				"				public void onAdDismissedFullScreenContent() {\n"
				"					AvdlActivity.mInterstitialAd = null;\n"
				"				}\n"
				"				// failed\n"
				"				@Override\n"
				"				public void onAdFailedToShowFullScreenContent(AdError adError) {\n"
				"					AvdlActivity.mInterstitialAd = null;\n"
				"				}\n"
				"				// impression\n"
				"				@Override\n"
				"				public void onAdImpression() {\n"
				"				}\n"
				"				// success\n"
				"				@Override\n"
				"				public void onAdShowedFullScreenContent() {\n"
				"				}\n"
				"			});\n"
				"			AvdlActivity.mInterstitialAd.show(AvdlActivity.activity);\n"
				"		} else {\n"
				"			// ad is not loaded\n"
				"		}\n"
				"	});\n"
				"}\n"
			);
		}
		else {
			avdl_string_cat(&ads_functions,
				"public void loadFullscreenAd(int X) {\n"
				"}\n"
				"public void showFullscreenAd(int X) {\n"
				"}\n"
			);
		}

		// rewarded ads
		if (avdl_settings->admob_ads_rewarded) {
			avdl_string_cat(&ads_declarations,
				"\n\tpublic static RewardedAd mRewardedAd;\n"
			);
			avdl_string_cat(&ads_functions,
				"public void loadRewardedAd(int X) {\n"
				"	// ad already loaded - skip\n"
				"	if ( mRewardedAd != null ) {\n"
				"		return;\n"
				"	}\n"
				"	runOnUiThread(() -> {\n"
				"		AdRequest adRequest = new AdRequest.Builder().build();\n"
				"		RewardedAd.load(AvdlActivity.activity, getString(R.string.game_services_rewarded_ad_id), adRequest,\n"
				"			new RewardedAdLoadCallback() {\n"
				"				@Override\n"
				"				public void onAdLoaded(RewardedAd ad) {\n"
				"					AvdlActivity.mRewardedAd = ad;\n"
				"				}\n"
				"				@Override\n"
				"				public void onAdFailedToLoad(LoadAdError loadAdError) {\n"
				"					AvdlActivity.mRewardedAd = null;\n"
				"				}\n"
				"			}\n"
				"		);\n"
				"	});\n"
				"}\n"
				"public void showRewardedAd(int X) {\n"
				"	// no ad loaded to show\n"
				"	if (AvdlActivity.mRewardedAd == null) {\n"
				"		return;\n"
				"	}\n"
				"	runOnUiThread(() -> {\n"
				"		if (AvdlActivity.mRewardedAd != null) {\n"
				"			AvdlActivity.mRewardedAd.setFullScreenContentCallback(new FullScreenContentCallback() {\n"
				"				// ad clicked\n"
				"				@Override\n"
				"				public void onAdClicked() {\n"
				"				}\n"
				"				// ad dismissed\n"
				"				@Override\n"
				"				public void onAdDismissedFullScreenContent() {\n"
				"					AvdlActivity.mRewardedAd = null;\n"
				"				}\n"
				"				// failed\n"
				"				@Override\n"
				"				public void onAdFailedToShowFullScreenContent(AdError adError) {\n"
				"					AvdlActivity.mRewardedAd = null;\n"
				"				}\n"
				"				// impression\n"
				"				@Override\n"
				"				public void onAdImpression() {\n"
				"				}\n"
				"				// success\n"
				"				@Override\n"
				"				public void onAdShowedFullScreenContent() {\n"
				"				}\n"
				"			});\n"
				"			AvdlActivity.mRewardedAd.show(AvdlActivity.activity, new OnUserEarnedRewardListener() {\n"
				"				@Override\n"
				"				public void onUserEarnedReward(RewardItem rewardItem) {\n"
				"					int rewardAmount = rewardItem.getAmount();\n"
				"					String rewardType = rewardItem.getType();\n"
				"					nativeOnRewardedAd(rewardAmount, rewardType);\n"
				"				}\n"
				"			});\n"
				"		} else {\n"
				"			// ad is not loaded\n"
				"		}\n"
				"	});\n"
				"}\n"
			);
		}
		else {
			avdl_string_cat(&ads_functions,
				"public void loadRewardedAd(int X) {\n"
				"}\n"
				"public void showRewardedAd(int X) {\n"
				"}\n"
			);
		}

	}
	else {
		avdl_string_cat(&ads_functions,
			"public void loadFullscreenAd(int X) {\n"
			"}\n"
			"public void showFullscreenAd(int X) {\n"
			"}\n"
			"public void loadRewardedAd(int X) {\n"
			"}\n"
			"public void showRewardedAd(int X) {\n"
			"}\n"
		);
	}

	if ( !avdl_string_isValid(&metadata) ) {
		avdl_log_error("cannot construct android manifest metadata: %s", avdl_string_getError(&metadata));
		avdl_string_clean(&metadata);
		return -1;
	}

	if ( !avdl_string_isValid(&permissions) ) {
		avdl_log_error("cannot construct android manifest permissions: %s", avdl_string_getError(&permissions));
		avdl_string_clean(&permissions);
		return -1;
	}

	if ( !avdl_string_isValid(&ads_imports) ) {
		avdl_log_error("cannot construct admob imports: %s", avdl_string_getError(&ads_imports));
		avdl_string_clean(&permissions);
		return -1;
	}

	if ( !avdl_string_isValid(&ads_declarations) ) {
		avdl_log_error("cannot construct admob declarations: %s", avdl_string_getError(&ads_declarations));
		avdl_string_clean(&permissions);
		return -1;
	}

	if ( !avdl_string_isValid(&ads_init) ) {
		avdl_log_error("cannot construct admob init: %s", avdl_string_getError(&ads_init));
		avdl_string_clean(&permissions);
		return -1;
	}

	if ( !avdl_string_isValid(&ads_functions) ) {
		avdl_log_error("cannot construct admob functions: %s", avdl_string_getError(&ads_functions));
		avdl_string_clean(&permissions);
		return -1;
	}

	if (avdl_settings->googleplay_mode) {
		file_replace(0,
			"avdl_build_android/app/src/main/java/org/darkdimension/avdl/AvdlActivity.java.in.googleplay", 0,
			"avdl_build_android/app/src/main/java/org/darkdimension/avdl/AvdlActivity.java.in2",
			"%AVDL_ADS_IMPORT%",
			avdl_string_toCharPtr(&ads_imports)
		);
	}
	else {
		file_replace(0,
			"avdl_build_android/app/src/main/java/org/darkdimension/avdl/AvdlActivity.java.in", 0,
			"avdl_build_android/app/src/main/java/org/darkdimension/avdl/AvdlActivity.java.in2",
			"%AVDL_ADS_IMPORT%",
			avdl_string_toCharPtr(&ads_imports)
		);
	}
	file_replace(0,
		"avdl_build_android/app/src/main/java/org/darkdimension/avdl/AvdlActivity.java.in2", 0,
		"avdl_build_android/app/src/main/java/org/darkdimension/avdl/AvdlActivity.java.in3",
		"%AVDL_ADS_DECLARATIONS%",
		avdl_string_toCharPtr(&ads_declarations)
	);
	file_replace(0,
		"avdl_build_android/app/src/main/java/org/darkdimension/avdl/AvdlActivity.java.in3", 0,
		"avdl_build_android/app/src/main/java/org/darkdimension/avdl/AvdlActivity.java.in4",
		"%AVDL_ADS_INIT%",
		avdl_string_toCharPtr(&ads_init)
	);
	file_replace(0,
		"avdl_build_android/app/src/main/java/org/darkdimension/avdl/AvdlActivity.java.in4", 0,
		"avdl_build_android/app/src/main/java/org/darkdimension/avdl/AvdlActivity.java",
		"%AVDL_ADS_FUNCTIONS%",
		avdl_string_toCharPtr(&ads_functions)
	);

	file_replace(0,
		"avdl_build_android/app/src/main/AndroidManifest.xml.in", 0,
		"avdl_build_android/app/src/main/AndroidManifest.xml.in2",
		"%AVDL_METADATA%",
		avdl_string_toCharPtr(&metadata)
	);

	file_replace(0,
		"avdl_build_android/app/src/main/AndroidManifest.xml.in2", 0,
		"avdl_build_android/app/src/main/AndroidManifest.xml",
		"%AVDL_PERMISSIONS%",
		avdl_string_toCharPtr(&permissions)
	);

	file_remove(
		"avdl_build_android/app/src/main/AndroidManifest.xml.in2"
	);
	file_remove(
		"avdl_build_android/app/src/main/AndroidManifest.xml.in"
	);

	#endif
	return 0;
}

int avdl_quest2_object(struct AvdlSettings *avdl_settings) {

	#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
	// put all object files to android
	strcpy(buffer, "avdl_build_quest2/");
	strcat(buffer, "/src/");
	dir_create(buffer);

	// copy project src files
	big_buffer[0] = '\0';
	Avdl_FileOp_ForFileInDirectory(".avdl_cache/", quest2_object_file);

	// Android.mk directory
	struct avdl_string cppFilePath;
	avdl_string_create(&cppFilePath, 1024);
	avdl_string_cat(&cppFilePath, "avdl_build_quest2/");
	avdl_string_cat(&cppFilePath, "/Projects/Android/jni/");
	if ( !avdl_string_isValid(&cppFilePath) ) {
		avdl_log_error("cannot construct quest2 src path: %s", avdl_string_getError(&cppFilePath));
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

	// add C flags
	{
		struct avdl_string cflags;
		avdl_string_create(&cflags, 1024);

		// game version
		avdl_string_cat(&cflags, " -DAVDL_GAME_VERSION=\"\\\"");
		avdl_string_cat(&cflags, avdl_settings->version_name);
		avdl_string_cat(&cflags, "\\\"\" ");

		// oculus mode
		if (avdl_settings->oculus_mode) {
			avdl_string_cat(&cflags, " -DAVDL_OCULUS ");
			avdl_string_cat(&cflags, " -DAVDL_OCULUS_PROJECT_ID=\\\"");
			avdl_string_cat(&cflags, avdl_settings->oculus_project_id);
			avdl_string_cat(&cflags, "\\\" ");
		}

		if (!avdl_string_isValid(&cflags)) {
			avdl_log_error("unable to construct cflags for android: %s", avdl_string_getError(&cflags));
			avdl_string_clean(&cflags);
			return -1;
		}
		file_replace(outDir, "Android.mk.in", outDir, "Android.mk.in2", "%AVDL_CFLAGS%", avdl_string_toCharPtr(&cflags));
	}

	file_replace(outDir, "Android.mk.in2", outDir, "Android.mk", "%AVDL_GAME_FILES%", big_buffer);
	close(outDir);

	avdl_string_clean(&cppFilePath);

	// handle versioning
	strcpy(buffer, "avdl_build_quest2/");
	strcat(buffer, "/Projects/Android/");
	outDir = open(buffer, O_DIRECTORY);
	file_replace(outDir, "build.gradle.in", outDir, "build.gradle.in2", "%AVDL_PACKAGE_NAME%", avdl_settings->package);
	file_replace(outDir, "build.gradle.in2", outDir, "build.gradle.in3", "%AVDL_VERSION_CODE%", avdl_settings->version_code_str);
	file_replace(outDir, "build.gradle.in3", outDir, "build.gradle", "%AVDL_VERSION_NAME%", avdl_settings->version_name);
	file_remove("avdl_build_quest2/Projects/Android/build.gradle.in");
	file_remove("avdl_build_quest2/Projects/Android/build.gradle.in2");
	file_remove("avdl_build_quest2/Projects/Android/build.gradle.in3");
	close(outDir);

	if (!is_dir("avdl_build_quest2/res/drawable/")) {
		dir_create("avdl_build_quest2/res/drawable/");
	}

	strcpy(buffer, "avdl_build_quest2/");
	strcat(buffer, "/res/drawable/");
	strcat(buffer, avdl_settings->icon_path);
	file_copy(avdl_settings->icon_path, buffer, 0);

	strcpy(buffer, "avdl_build_quest2/");
	strcat(buffer, "/res/drawable/");
	strcat(buffer, avdl_settings->icon_foreground_path);
	file_copy(avdl_settings->icon_foreground_path, buffer, 0);

	strcpy(buffer, "avdl_build_quest2/");
	strcat(buffer, "/res/drawable/");
	strcat(buffer, avdl_settings->icon_background_path);
	file_copy(avdl_settings->icon_background_path, buffer, 0);

	if (!is_dir("avdl_build_quest2/res/values/")) {
		dir_create("avdl_build_quest2/res/values/");
	}

	// project name
	strcpy(buffer, "avdl_build_quest2/");
	strcat(buffer, "/res/values/");
	outDir = open(buffer, O_DIRECTORY);
	file_replace(outDir, "strings.xml.in", outDir, "strings.xml", "%AVDL_PROJECT_NAME%", avdl_settings->project_name);
	strcat(buffer, "strings.xml.in");
	file_remove("avdl_build_quest2/res/values/strings.xml.in");
	close(outDir);
	#endif
	return 0;
}

int avdl_d3d11_object(struct AvdlSettings *avdl_settings) {

	#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
	// put all object files to android
	strcpy(buffer, "avdl_build_d3d11/");
	strcat(buffer, "/src/");
	dir_create(buffer);

	// copy project src files
	big_buffer[0] = '\0';
	Avdl_FileOp_ForFileInDirectory(".avdl_cache/", d3d11_object_file);

	int outDir = open("avdl_build_d3d11/", O_DIRECTORY);
	if (!outDir) {
		printf("avdl: can't open %s: %s\n", "avdl_build_d3d11/", strerror(errno));
		return -1;
	}

	file_replace(outDir, "avdl_project.vcxproj.in3",
		outDir, "avdl_project.vcxproj.in4",
		"%AVDL_PROJECT_SRC%", big_buffer
	);
	/*
	// Android.mk directory
	struct avdl_string cppFilePath;
	avdl_string_create(&cppFilePath, 1024);
	avdl_string_cat(&cppFilePath, "avdl_build_quest2/");
	avdl_string_cat(&cppFilePath, "/Projects/Android/jni/");
	if ( !avdl_string_isValid(&cppFilePath) ) {
		avdl_log_error("cannot construct quest2 src path: %s", avdl_string_getError(&cppFilePath));
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

	// add C flags
	{
		struct avdl_string cflags;
		avdl_string_create(&cflags, 1024);

		// game version
		avdl_string_cat(&cflags, " -DAVDL_GAME_VERSION=\"\\\"");
		avdl_string_cat(&cflags, avdl_settings->version_name);
		avdl_string_cat(&cflags, "\\\"\" ");

		// oculus mode
		if (avdl_settings->oculus_mode) {
			avdl_string_cat(&cflags, " -DAVDL_OCULUS ");
			avdl_string_cat(&cflags, " -DAVDL_OCULUS_PROJECT_ID=\\\"");
			avdl_string_cat(&cflags, avdl_settings->oculus_project_id);
			avdl_string_cat(&cflags, "\\\" ");
		}

		if (!avdl_string_isValid(&cflags)) {
			avdl_log_error("unable to construct cflags for android: %s", avdl_string_getError(&cflags));
			avdl_string_clean(&cflags);
			return -1;
		}
		file_replace(outDir, "Android.mk.in", outDir, "Android.mk.in2", "%AVDL_CFLAGS%", avdl_string_toCharPtr(&cflags));
	}

	file_replace(outDir, "Android.mk.in2", outDir, "Android.mk", "%AVDL_GAME_FILES%", big_buffer);
	close(outDir);

	avdl_string_clean(&cppFilePath);

	// handle versioning
	strcpy(buffer, "avdl_build_quest2/");
	strcat(buffer, "/Projects/Android/");
	outDir = open(buffer, O_DIRECTORY);
	file_replace(outDir, "build.gradle.in", outDir, "build.gradle.in2", "%AVDL_PACKAGE_NAME%", avdl_settings->package);
	file_replace(outDir, "build.gradle.in2", outDir, "build.gradle.in3", "%AVDL_VERSION_CODE%", avdl_settings->version_code_str);
	file_replace(outDir, "build.gradle.in3", outDir, "build.gradle", "%AVDL_VERSION_NAME%", avdl_settings->version_name);
	file_remove("avdl_build_quest2/Projects/Android/build.gradle.in");
	file_remove("avdl_build_quest2/Projects/Android/build.gradle.in2");
	file_remove("avdl_build_quest2/Projects/Android/build.gradle.in3");
	close(outDir);
	*/

	if (!is_dir("avdl_build_d3d11/Assets/")) {
		dir_create("avdl_build_d3d11/Assets/");
	}

	/*
	 * icons and metadata
	 *
	strcpy(buffer, "avdl_build_d3d11/");
	strcat(buffer, "/Assets/");
	strcat(buffer, avdl_settings->icon_path);
	file_copy(avdl_settings->icon_path, buffer, 0);

	strcpy(buffer, "avdl_build_d3d11/");
	strcat(buffer, "/Assets/");
	strcat(buffer, avdl_settings->icon_foreground_path);
	file_copy(avdl_settings->icon_foreground_path, buffer, 0);

	strcpy(buffer, "avdl_build_quest2/");
	strcat(buffer, "/res/drawable/");
	strcat(buffer, avdl_settings->icon_background_path);
	file_copy(avdl_settings->icon_background_path, buffer, 0);

	if (!is_dir("avdl_build_quest2/res/values/")) {
		dir_create("avdl_build_quest2/res/values/");
	}

	// project name
	strcpy(buffer, "avdl_build_quest2/");
	strcat(buffer, "/res/values/");
	outDir = open(buffer, O_DIRECTORY);
	file_replace(outDir, "strings.xml.in", outDir, "strings.xml", "%AVDL_PROJECT_NAME%", avdl_settings->project_name);
	strcat(buffer, "strings.xml.in");
	file_remove("avdl_build_quest2/res/values/strings.xml.in");
	close(outDir);
	*/
	#endif
	return 0;
}

int collect_src_files(const char *dirname, const char *filename, int fileIndex, int filesTotal) {

	// ignore `.` and `..`
	if (strcmp(filename, ".") == 0
	||  strcmp(filename, "..") == 0) {
		return 0;
	}

	// src file full path
	struct avdl_string srcFilePath;
	avdl_string_create(&srcFilePath, 1024);
	avdl_string_cat(&srcFilePath, dirname);
	avdl_string_cat(&srcFilePath, "/");
	avdl_string_cat(&srcFilePath, filename);
	if ( !avdl_string_isValid(&srcFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", dirname, filename, avdl_string_getError(&srcFilePath));
		avdl_string_clean(&srcFilePath);
		return -1;
	}

	// check file type
	struct stat statbuf;
	if (stat(avdl_string_toCharPtr(&srcFilePath), &statbuf) != 0) {
		avdl_log_error("Unable to stat file '%s': %s", avdl_string_toCharPtr(&srcFilePath), strerror(errno));
		avdl_string_clean(&srcFilePath);
		return -1;
	}

	// is directory - skip - maybe recursive compilation at some point?
	if (Avdl_FileOp_IsDirStat(&statbuf)) {
		avdl_log("skipping directory: %s", avdl_string_toCharPtr(&srcFilePath));
		avdl_string_clean(&srcFilePath);
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
		return 0;
	}

	strcat(big_buffer, avdl_string_toCharPtr(&srcFilePath));
	strcat(big_buffer, " ");

	avdl_string_clean(&srcFilePath);

	return 0;
}

int collect_asset_files(const char *dirname, const char *filename, int fileIndex, int filesTotal) {

	// ignore `.` and `..`
	if (strcmp(filename, ".") == 0
	||  strcmp(filename, "..") == 0) {
		return 0;
	}

	// src file full path
	struct avdl_string srcFilePath;
	avdl_string_create(&srcFilePath, 1024);
	avdl_string_cat(&srcFilePath, dirname);
	avdl_string_cat(&srcFilePath, "/");
	avdl_string_cat(&srcFilePath, filename);
	if ( !avdl_string_isValid(&srcFilePath) ) {
		avdl_log_error("cannot construct path '%s%s': %s", dirname, filename, avdl_string_getError(&srcFilePath));
		avdl_string_clean(&srcFilePath);
		return -1;
	}

	// check file type
	struct stat statbuf;
	if (stat(avdl_string_toCharPtr(&srcFilePath), &statbuf) != 0) {
		avdl_log_error("Unable to stat file '%s': %s", avdl_string_toCharPtr(&srcFilePath), strerror(errno));
		avdl_string_clean(&srcFilePath);
		return -1;
	}

	// is directory - skip - maybe recursive at some point?
	if (Avdl_FileOp_IsDirStat(&statbuf)) {
		avdl_log("skipping directory: %s", avdl_string_toCharPtr(&srcFilePath));
		avdl_string_clean(&srcFilePath);
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
		return 0;
	}

	strcat(big_buffer, avdl_string_toCharPtr(&srcFilePath));
	strcat(big_buffer, " ");

	avdl_string_clean(&srcFilePath);

	return 0;
}
