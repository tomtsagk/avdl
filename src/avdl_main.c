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
#include "avdl_json.h"

#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <unistd.h>
#endif

// prepare imagemagick commands
#if AVDL_IS_OS(AVDL_OS_WINDOWS)
#define IMAGEMAGICK_COMPOSITE "magick.exe composite"
#define IMAGEMAGICK_CONVERT "magick.exe"
#define IMAGEMAGICK_PAREN_OPEN "("
#define IMAGEMAGICK_PAREN_CLOSE ")"
#else
#define IMAGEMAGICK_COMPOSITE "composite"
#define IMAGEMAGICK_CONVERT "magick"
#define IMAGEMAGICK_PAREN_OPEN "\\("
#define IMAGEMAGICK_PAREN_CLOSE "\\)"
#endif

extern enum AVDL_PLATFORM avdl_platform_temp;

const char cache_dir[] = ".avdl_cache/";
#define android_cache_dir ".avdl_cache/android"

extern float parsing_float;

char included_files[10][100];
int included_files_num = 0;

// game node, parent of all nodes
struct ast_node *game_node;

int create_android_directory(const char *androidDirName);
int create_quest2_directory(const char *dirName);
int create_d3d11_directory(struct AvdlSettings *avdl_settings, const char *dirName);

// compiler steps
int avdl_transpile(struct AvdlSettings *);
int avdl_compile(struct AvdlSettings *);
int avdl_compile_cengine(struct AvdlSettings *);
int avdl_link(struct AvdlSettings *);
int avdl_link_android(struct AvdlSettings *);
int avdl_assets(struct AvdlSettings *);
int avdl_directories(struct AvdlSettings *);
int avdl_metadata(struct AvdlSettings *);
int avdl_android_object(struct AvdlSettings *);
int avdl_quest2_object(struct AvdlSettings *);
int avdl_d3d11_object(struct AvdlSettings *);
int avdl_cmake(struct AvdlSettings *);
int avdl_makefile(struct AvdlSettings *);

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

	// check arguments
	int handle_return = avdl_arguments_handle(&avdl_settings, argc, argv);

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

	// editor mode
	if (avdl_settings.editor_mode) {
		avdl_log("start editor");
		if (system("avdl-editor") != 0) {
			avdl_log("error running `avdl-editor`");
			return -1;
		}
		return 0;
	}

	// load settings from the current project
	if (AvdlSettings_SetFromFile(&avdl_settings, "app.avdl") != 0) {
		avdl_log_error("failed to get project settings from '%s'", "app.avdl");
		return -1;
	}

	// display project data
	avdl_log("~ Project Details ~");
	avdl_log("Project Name: " BLU "%s" RESET, avdl_settings.project_name);
	avdl_log("Version: " BLU "%d" RESET " (" BLU "%s" RESET ")-" BLU "%d" RESET, avdl_settings.version_code, avdl_settings.version_name, avdl_settings.revision);
	avdl_log("Package: " BLU "%s" RESET, avdl_settings.package);
	avdl_log("CEngine location in: " BLU "%s" RESET, avdl_settings.cengine_path);
	avdl_log("~ Project Details end ~");
	avdl_log("");

	// create directories
	if ( avdl_directories(&avdl_settings) != 0) {
		avdl_log_error("could not create avdl directories\n");
		return -1;
	}

	// metadata
	if ( avdl_metadata(&avdl_settings) != 0) {
		avdl_log_error("could not create metadata\n");
		return -1;
	}

	// handle assets
	if ( avdl_assets(&avdl_settings) != 0) {
		avdl_log_error("could not create assets\n");
		return -1;
	}

	// makefile generation
	if (avdl_settings.makefile_mode) {
		if (avdl_makefile(&avdl_settings) != 0) {
			avdl_log_error("failed to generate " BLU "makefile" RESET " for " BLU "%s" RESET, avdl_settings.project_name);
			return -1;
		}
	}

	// from `.dd` to `.c` inside `.avdl_cache`
	if ( avdl_transpile(&avdl_settings) != 0) {
		avdl_log_error("failed to transpile project\n");
		return -1;
	}

	// cmake generation
	if (avdl_settings.cmake_mode) {
		if (avdl_cmake(&avdl_settings) != 0) {
			avdl_log_error("failed to generate " BLU "cmake" RESET " for " BLU "%s" RESET, avdl_settings.project_name);
			return -1;
		}
	}

	// translate only (-t)
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

	// report results
	avdl_time_end(&clock);
	if (avdl_settings.target_platform == AVDL_PLATFORM_ANDROID) {
		avdl_log("avdl: project " BLU "%s" RESET " compiled successfully at " BLU "./avdl_build_android/" RESET " in " BLU "%.3f" RESET " seconds", avdl_settings.project_name, avdl_time_getTimeDouble(&clock));
	}
	else
	if (avdl_settings.target_platform == AVDL_PLATFORM_QUEST2) {
		avdl_log("avdl: project " BLU "%s" RESET " compiled successfully at " BLU "./avdl_build_quest2/" RESET " in " BLU "%.3f" RESET " seconds", avdl_settings.project_name, avdl_time_getTimeDouble(&clock));
	}
	else {
		avdl_log("avdl: project " BLU "%s" RESET " compiled successfully at " BLU "./avdl_build/" RESET " in " BLU "%.3f" RESET " seconds", avdl_settings.project_name, avdl_time_getTimeDouble(&clock));
	}

	// success!
	return 0;
}

int create_android_directory(const char *androidDirName) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	#else
	int isDir = is_dir(androidDirName);
	if (isDir == 0) {
		dir_create(androidDirName);
	}
	else
	if (isDir < 0) {
		avdl_log_error("file '%s' not a directory", androidDirName);
		return -1;
	}

	// copy any needed files from template
	struct avdl_string cenginePath;
	avdl_string_create(&cenginePath, 1024);
	avdl_string_cat(&cenginePath, avdl_pkg_GetProjectPath());
	avdl_string_cat(&cenginePath, "/share/avdl/android");
	if ( !avdl_string_isValid(&cenginePath) ) {
		avdl_log_error("cannot construct path of cengine: %s", avdl_string_getError(&cenginePath));
		avdl_string_clean(&cenginePath);
		return -1;
	}
	dir_copy_recursive_ifNewer(0, avdl_string_toCharPtr(&cenginePath), 0, androidDirName);
	avdl_string_clean(&cenginePath);

	if (chmod(android_cache_dir "/gradlew", S_IXUSR | S_IWUSR | S_IRUSR) != 0) {
		avdl_log_error("cannot change mode of `gradlew`: %s", avdl_string_getError(&cenginePath));
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

int create_d3d11_directory(struct AvdlSettings *avdl_settings, const char *dirName) {
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
	struct avdl_dynamic_array cengineHeaders;
	if (Avdl_FileOp_GetFilesInDirectory(avdl_settings->cengine_path, &cengineHeaders) != 0) {
		avdl_log_error("Can't get cengine headers");
		return -1;
	}

	struct avdl_string avdlHeaders;
	avdl_string_create(&avdlHeaders, 4000);
	for (int i = 0; i < avdl_da_count(&cengineHeaders); i++) {
		struct avdl_string *str = avdl_da_get(&cengineHeaders, i);
		if (!avdl_string_endsIn(str, ".c") && !avdl_string_endsIn(str, ".cpp")) {
			continue;
		}
		avdl_string_cat(&avdlHeaders, "    <ClInclude Include=\"avdl_src/");
		avdl_string_cat(&avdlHeaders, avdl_string_toCharPtr(str));
		avdl_string_cat(&avdlHeaders, "\"/>\n");
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&cengineHeaders);
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

	// cengine files
	struct avdl_dynamic_array cengineFiles;
	if (Avdl_FileOp_GetFilesInDirectoryRecursive(avdl_settings->cengine_path, &cengineFiles) != 0) {
		avdl_log_error("Can't get cengine files");
		return -1;
	}

	// collect src
	struct avdl_string avdlSrc;
	avdl_string_create(&avdlSrc, 10000);
	for (int i = 0; i < avdl_da_count(&cengineFiles); i++) {
		struct avdl_string *str = avdl_da_get(&cengineFiles, i);

		if (!avdl_string_endsIn(str, ".c") && !avdl_string_endsIn(str, ".cpp")) {
			continue;
		}
		avdl_string_cat(&avdlSrc, "    <ClCompile Include=\"avdl_src/");
		avdl_string_cat(&avdlSrc, avdl_string_toCharPtr(str));
		avdl_string_cat(&avdlSrc, "\">\n");
		if (strcmp(avdl_string_toCharPtr(str), "avdl_engine_cpp.cpp") == 0
		||  strcmp(avdl_string_toCharPtr(str), "avdl_graphics_direct3d11.cpp") == 0
		||  strcmp(avdl_string_toCharPtr(str), "dd_log_cpp.cpp") == 0
		) {
			avdl_string_cat(&avdlSrc, "      <CompileAsWinRT>true</CompileAsWinRT>\n");
		}
		else {
			avdl_string_cat(&avdlSrc, "      <CompileAsWinRT>false</CompileAsWinRT>\n");
		}
		avdl_string_cat(&avdlSrc, "      <PrecompiledHeader>NotUsing</PrecompiledHeader>\n");
		avdl_string_cat(&avdlSrc, "    </ClCompile>\n");
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&cengineFiles);
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

static struct avdl_vec3 {
	float v[3];
};

int json_expect_array3f(struct avdl_json_object *json, struct avdl_vec3 *v) {

	if (avdl_json_getToken(json) != AVDL_JSON_ARRAY_START) {
		avdl_log_error("Json: expected array start for 3f: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
		return -1;
	}

	avdl_json_next(json);
	for (int i = 0; i < 3; i++) {
		if (avdl_json_getToken(json) != AVDL_JSON_FLOAT) {
			avdl_log_error("Json 3f: was expecting 3 floats but found something else: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
			return -1;
		}

		v->v[i] = avdl_json_getTokenFloat(json);
		avdl_json_next(json);
	}

	return 0;
}

static int component_counter = 0;
int json_expect_component(struct avdl_json_object *json, int fd, char *node_name) {

	// check main object
	if (avdl_json_getToken(json) != AVDL_JSON_OBJECT_START) {
		avdl_log_error("Json component should start with a '{': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
		return -1;
	}

	char component_name[100];
	strcpy(component_name, "c_");
	snprintf(component_name +2, 80, "%d", component_counter);
	avdl_json_next(json);
	while (avdl_json_getToken(json) != AVDL_JSON_OBJECT_END) {
		// find key
		if (avdl_json_getToken(json) != AVDL_JSON_KEY) {
			avdl_log_error("json expected key, got something else: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
			return -1;
		}
		//avdl_log("got component key: %s", avdl_json_getTokenString(json));
		char *content = "\t# found component \"";
		write(fd, content, strlen(content));
		content = avdl_json_getTokenString(json);
		write(fd, content, strlen(content));
		content = "\"\n";
		write(fd, content, strlen(content));

		if (strcmp(avdl_json_getTokenString(json), "name") == 0) {
			avdl_json_next(json);
			if (avdl_json_getToken(json) == AVDL_JSON_STRING) {
				if (strlen(avdl_json_getTokenString(json)) >= 99) {
					avdl_log_error("component type too big: %s", avdl_json_getTokenString(json));
					return -1;
				}
				char component_type[100];
				strcpy(component_type, avdl_json_getTokenString(json));

				//avdl_log("got component string: %s", avdl_json_getTokenString(json));
				char *content = "\t(def ref ";
				write(fd, content, strlen(content));
				content = component_type;
				write(fd, content, strlen(content));
				content = " ";
				write(fd, content, strlen(content));
				content = component_name;
				write(fd, content, strlen(content));
				content = ")\n";
				write(fd, content, strlen(content));

				content = "\t(= ";
				write(fd, content, strlen(content));
				content = component_name;
				write(fd, content, strlen(content));
				content = " (avdl_node_AddComponent ";
				write(fd, content, strlen(content));
				content = node_name;
				write(fd, content, strlen(content));
				content = " ";
				write(fd, content, strlen(content));
				content = component_type;
				write(fd, content, strlen(content));
				content = "))\n";
				write(fd, content, strlen(content));
			}
			else {
				//avdl_log("component something else?");
			}
		}
		else {
			if (strlen(avdl_json_getTokenString(json)) >= 99) {
				avdl_log_error("component variable too big: %s", avdl_json_getTokenString(json));
				return -1;
			}
			char component_variable[100];
			strcpy(component_variable, avdl_json_getTokenString(json));

			avdl_json_next(json);

			char *content = "\t(= ";
			write(fd, content, strlen(content));
			write(fd, component_name, strlen(component_name));
			content = ".";
			write(fd, content, strlen(content));
			write(fd, component_variable, strlen(component_variable));
			content = " ";
			write(fd, content, strlen(content));

			if (avdl_json_getToken(json) == AVDL_JSON_STRING) {
				content = "\"";
				write(fd, content, strlen(content));
				content = avdl_json_getTokenString(json);
				write(fd, content, strlen(content));
				content = "\"";
				write(fd, content, strlen(content));
			}
			else
			if (avdl_json_getToken(json) == AVDL_JSON_INT
			||  avdl_json_getToken(json) == AVDL_JSON_FLOAT) {
				content = avdl_json_getTokenString(json);
				write(fd, content, strlen(content));
			}
			else {
				avdl_log_error("unsupported json component variable value");
				return -1;
			}
			content = ")\n";
			write(fd, content, strlen(content));
		}

		/*
		avdl_json_next(json);
		if (avdl_json_getToken(json) == AVDL_JSON_STRING) {
			//avdl_log("got component string: %s", avdl_json_getTokenString(json));
			char *content = "\t# with component string \"";
			write(fd, content, strlen(content));
			content = avdl_json_getTokenString(json);
			write(fd, content, strlen(content));
			content = "\"\n";
			write(fd, content, strlen(content));
		}
		else {
			//avdl_log("component something else?");
		}
		*/

		avdl_json_next(json);
	}

	avdl_json_next(json);

	char *content = "\t(";
	write(fd, content, strlen(content));
	write(fd, component_name, strlen(component_name));
	content = ".after_create ";
	write(fd, content, strlen(content));
	write(fd, component_name, strlen(component_name));
	content = ")\n";
	write(fd, content, strlen(content));

	component_counter++;

	return 0;
}

static int transform_counter = 0;
int json_expect_node(struct avdl_json_object *json, int fd, char *node_parent_name) {

	// check main object
	if (avdl_json_getToken(json) != AVDL_JSON_OBJECT_START) {
		avdl_log_error("Json node should start with a '{': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
		return -1;
	}

	// generate node name
	char node_name[100];
	strcpy(node_name, "n_");
	snprintf(node_name +2, 80, "%d", transform_counter);

	if (transform_counter > 0) {

		// every node apart from the first one needs to have a parent
		if (!node_parent_name) {
			avdl_log_error("node parent name missing on non-root node");
			return -1;
		}

		// define node
		char *content = "\t(def ref avdl_node ";
		write(fd, content, strlen(content));
		content = node_name;
		write(fd, content, strlen(content));
		content = ")\n";
		write(fd, content, strlen(content));

		// add it as child to parent node
		content = "\t(= ";
		write(fd, content, strlen(content));
		content = node_name;
		write(fd, content, strlen(content));
		content = " (";
		write(fd, content, strlen(content));
		content = node_parent_name;
		write(fd, content, strlen(content));
		content = ".AddChild ";
		write(fd, content, strlen(content));
		content = node_parent_name;
		write(fd, content, strlen(content));
		content = "))\n\n";
		write(fd, content, strlen(content));

	}

	// generate transform name
	char transform_name[100];
	strcpy(transform_name, "t_");
	snprintf(transform_name +2, 80, "%d", transform_counter);

	{
		// define transform
		char *content = "\t(def ref avdl_transform ";
		write(fd, content, strlen(content));
		content = transform_name;
		write(fd, content, strlen(content));
		content = ")\n";
		write(fd, content, strlen(content));

		// get transform from current node
		content = "\t(= ";
		write(fd, content, strlen(content));
		content = transform_name;
		write(fd, content, strlen(content));
		content = "( ";
		write(fd, content, strlen(content));
		content = node_name;
		write(fd, content, strlen(content));
		content = ".GetLocalTransform ";
		write(fd, content, strlen(content));
		content = node_name;
		write(fd, content, strlen(content));
		content = "))\n\n";
		write(fd, content, strlen(content));
	}

	transform_counter++;

	avdl_json_next(json);
	while (avdl_json_getToken(json) != AVDL_JSON_OBJECT_END) {

		// find key
		if (avdl_json_getToken(json) != AVDL_JSON_KEY) {
			avdl_log_error("json expected key, got something else: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
			return -1;
		}
		//avdl_log("got key: %s", avdl_json_getTokenString(json));

		if (strcmp(avdl_json_getTokenString(json), "name") == 0) {
			avdl_json_next(json);
			if (avdl_json_getToken(json) == AVDL_JSON_STRING) {
				//avdl_log("got string name: %s", avdl_json_getTokenString(json));

				char *content = "\t(";
				write(fd, content, strlen(content));
				content = node_name;
				write(fd, content, strlen(content));
				content = ".SetName ";
				write(fd, content, strlen(content));
				content = node_name;
				write(fd, content, strlen(content));
				content = " \"";
				write(fd, content, strlen(content));
				content = avdl_json_getTokenString(json);
				write(fd, content, strlen(content));
				content = "\")\n";
				write(fd, content, strlen(content));
			}
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "position") == 0) {
			avdl_json_next(json);
			struct avdl_vec3 v;
			json_expect_array3f(json, &v);
			//avdl_log("got position: %f %f %f", v.v[0], v.v[1], v.v[2]);

			char *content = "\t(";
			write(fd, content, strlen(content));
			content = transform_name;
			write(fd, content, strlen(content));
			content = ".SetPosition3f ";
			write(fd, content, strlen(content));
			content = transform_name;
			write(fd, content, strlen(content));

			char numbuffer[100];

			content = " ";
			write(fd, content, strlen(content));
			snprintf(numbuffer, 90, "%f", v.v[0]);
			write(fd, numbuffer, strlen(numbuffer));

			content = " ";
			write(fd, content, strlen(content));
			snprintf(numbuffer, 90, "%f", v.v[1]);
			write(fd, numbuffer, strlen(numbuffer));

			content = " ";
			write(fd, content, strlen(content));
			snprintf(numbuffer, 90, "%f", v.v[2]);
			write(fd, numbuffer, strlen(numbuffer));

			content = ")\n";
			write(fd, content, strlen(content));
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "rotation") == 0) {
			avdl_json_next(json);
			struct avdl_vec3 v;
			json_expect_array3f(json, &v);
			//avdl_log("got rotation: %f %f %f", v.v[0], v.v[1], v.v[2]);

			char *content = "\t(";
			write(fd, content, strlen(content));
			content = transform_name;
			write(fd, content, strlen(content));
			content = ".SetRotation3f ";
			write(fd, content, strlen(content));
			content = transform_name;
			write(fd, content, strlen(content));

			char numbuffer[100];

			content = " ";
			write(fd, content, strlen(content));
			snprintf(numbuffer, 90, "%f", v.v[0]);
			write(fd, numbuffer, strlen(numbuffer));

			content = " ";
			write(fd, content, strlen(content));
			snprintf(numbuffer, 90, "%f", v.v[1]);
			write(fd, numbuffer, strlen(numbuffer));

			content = " ";
			write(fd, content, strlen(content));
			snprintf(numbuffer, 90, "%f", v.v[2]);
			write(fd, numbuffer, strlen(numbuffer));

			content = ")\n";
			write(fd, content, strlen(content));
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "scale") == 0) {
			avdl_json_next(json);
			struct avdl_vec3 v;
			json_expect_array3f(json, &v);
			//avdl_log("got scale: %f %f %f", v.v[0], v.v[1], v.v[2]);

			char *content = "\t(";
			write(fd, content, strlen(content));
			content = transform_name;
			write(fd, content, strlen(content));
			content = ".SetScale3f ";
			write(fd, content, strlen(content));
			content = transform_name;
			write(fd, content, strlen(content));

			char numbuffer[100];

			content = " ";
			write(fd, content, strlen(content));
			snprintf(numbuffer, 90, "%f", v.v[0]);
			write(fd, numbuffer, strlen(numbuffer));

			content = " ";
			write(fd, content, strlen(content));
			snprintf(numbuffer, 90, "%f", v.v[1]);
			write(fd, numbuffer, strlen(numbuffer));

			content = " ";
			write(fd, content, strlen(content));
			snprintf(numbuffer, 90, "%f", v.v[2]);
			write(fd, numbuffer, strlen(numbuffer));

			content = ")\n";
			write(fd, content, strlen(content));
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "components") == 0) {
			avdl_json_next(json);

			// expect array
			if (avdl_json_getToken(json) != AVDL_JSON_ARRAY_START) {
				avdl_log_error("Json expected array start '[': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
				return -1;
			}

			avdl_json_next(json);
			while (avdl_json_getToken(json) != AVDL_JSON_ARRAY_END) {
				json_expect_component(json, fd, node_name);
			}
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "children") == 0) {
			avdl_json_next(json);

			// expect array
			if (avdl_json_getToken(json) != AVDL_JSON_ARRAY_START) {
				avdl_log_error("Json expected array start '[': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
				return -1;
			}

			avdl_json_next(json);
			while (avdl_json_getToken(json) != AVDL_JSON_ARRAY_END) {
				char *content = "\n";
				write(fd, content, strlen(content));
				if (json_expect_node(json, fd, node_name) != 0) {
					avdl_log_error("json: error parsing node");
					return -1;
				}
			}
		}
		else {
			avdl_json_next(json);
			avdl_log("something else: %s", avdl_json_getTokenString(json));
			break;
		}
		avdl_json_next(json);
	}
	avdl_json_next(json);

	//return node;
	return 0;

}

static int avdl_json_to_dd(char *src, char *dst) {

	//avdl_log("translating json %s to dd %s", src, dst);

	struct avdl_json_object json;
	//avdl_log("json parse");

	// get function name from filename
	char id[100];

	// filename should have the form "src/my_file.json", first remove everything before and including "/"
	char *p = strstr(src, "/");
	if (p) {
		p++;
		strcpy(id, p);
	}
	else {
		strcpy(id, src);
	}

	// remove anything after and including "."
	p = strstr(id, ".");
	if (p) {
		p[0] = '\0';
	}
	//avdl_log("filename: %s", filename);
	//avdl_log("id: %s", id);

	file_remove(dst);
	int fd = open(dst, O_RDWR | O_CREAT, 0777);
	if (fd == -1) {
		avdl_log_error("Unable to open '%s': %s\n", dst, strerror(errno));
		return -1;
	}

	char *content = "(include \"avdl_custom_components.ddh\")\n\n(function void ";
	write(fd, content, strlen(content));
	content = id;
	write(fd, content, strlen(content));
	content = " {avdl_node n_0} {\n";
	write(fd, content, strlen(content));

	// init
	avdl_json_initFile(&json, src);
	avdl_json_next(&json);

	transform_counter = 0;
	component_counter = 0;
	if (json_expect_node(&json, fd, 0) != 0) {
		avdl_log_error("error translating json to dd");
		close(fd);
		avdl_json_deinit(&json);
		return -1;
	}
	//avdl_log("json parse complete");

	content = "})\n";
	write(fd, content, strlen(content));

	// clean
	close(fd);
	avdl_json_deinit(&json);
	return 0;
}

int avdl_transpile(struct AvdlSettings *avdl_settings) {

	if (avdl_settings->makefile_mode) {
		return 0;
	}

	// cache directory
	if (!is_dir(".avdl_cache")) {
		dir_create(".avdl_cache");
	}
	if (avdl_settings->target_platform == AVDL_PLATFORM_ANDROID) {
		if (!is_dir(android_cache_dir "/app/src/main/cpp/game/")) {
			dir_create(android_cache_dir "/app/src/main/cpp/game/");
		}
	}

	printf("avdl: transpiling - " RED "0%%" RESET "\r");
	fflush(stdout);

	// collect avdl project src
	struct avdl_dynamic_array srcFiles;
	Avdl_FileOp_GetFilesInDirectory(avdl_settings->src_dir, &srcFiles);

	// for each file
	for (int i = 0; i < avdl_da_count(&srcFiles); i++) {
		struct avdl_string *str = avdl_da_get(&srcFiles, i);

		// only avdl `.dd` files
		if (!avdl_string_endsIn(str, ".dd") && !avdl_string_endsIn(str, ".json")) {
			/*
			avdl_log("---");
			avdl_log("read: %s", avdl_string_toCharPtr(str));
			avdl_log("---");
			*/
			continue;
		}

		//avdl_log("about to transpile: %s", avdl_string_toCharPtr(str));

		// src file full path
		struct avdl_string srcFilePath;
		avdl_string_create(&srcFilePath, 1024);
		avdl_string_cat(&srcFilePath, avdl_settings->src_dir);
		avdl_string_cat(&srcFilePath, avdl_string_toCharPtr(str));
		if ( !avdl_string_isValid(&srcFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				avdl_settings->src_dir, avdl_string_toCharPtr(str),
				avdl_string_getError(&srcFilePath)
			);
			avdl_string_clean(&srcFilePath);
			return -1;
		}

		if (avdl_string_endsIn(str, ".json")) {

			struct avdl_string ddFilePath;
			avdl_string_create(&ddFilePath, 1024);
			avdl_string_cat(&ddFilePath, cache_dir);
			avdl_string_cat(&ddFilePath, avdl_string_toCharPtr(str));
			avdl_string_replaceEnding(&ddFilePath, ".json", ".dd");
			if ( !avdl_string_isValid(&ddFilePath) ) {
				avdl_log_error("cannot construct path of json to dd: '%s%s': %s",
					avdl_settings->src_dir, avdl_string_toCharPtr(str),
					avdl_string_getError(&ddFilePath)
				);
				avdl_string_clean(&ddFilePath);
				return -1;
			}

			if (avdl_json_to_dd(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&ddFilePath)) != 0) {
				avdl_log_error("could not translate json to dd: %s", avdl_string_toCharPtr(&srcFilePath));
				return -1;
			}

			avdl_string_copy(&srcFilePath, &ddFilePath);
		}

		// dst file full path
		struct avdl_string dstFilePath;
		avdl_string_create(&dstFilePath, 1024);
		// android transpiles directly to the gradle directory
		if (avdl_settings->target_platform == AVDL_PLATFORM_ANDROID) {
			avdl_string_cat(&dstFilePath, android_cache_dir);
			avdl_string_cat(&dstFilePath, "/app/src/main/cpp/game/");
		}
		else {
			avdl_string_cat(&dstFilePath, cache_dir);
		}
		avdl_string_cat(&dstFilePath, avdl_string_toCharPtr(str));
		avdl_string_cat(&dstFilePath, ".c");
		if ( !avdl_string_isValid(&dstFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				cache_dir, avdl_string_toCharPtr(str),
				avdl_string_getError(&dstFilePath)
			);
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
			continue;
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
			continue;
		}

		// skip files already compiled (check last modified)
		// but compile everything if a header file has changed
		if ( avdl_settings->use_cache
		&&   !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), avdl_string_toCharPtr(&srcFilePath))
		&&   !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), "include/") ) {
			//printf("avdl src file not modified, skipping transpilation of '%s' -> '%s'\n", avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&dstFilePath));
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&dstFilePath);
			continue;
		}
		//avdl_log("transpiling %s to %s", avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&dstFilePath));

		included_files_num = 0;

		// TODO: Find a better way to do this
		avdl_platform_temp = avdl_settings->target_platform;

		// initialise the parent node
		game_node = ast_create(AST_GAME);
		if (semanticAnalyser_convertToAst(game_node, avdl_string_toCharPtr(&srcFilePath)) != 0) {
			avdl_log_error("failed to do semantic analysis on '" BLU "%s" RESET "'", avdl_string_toCharPtr(&srcFilePath));
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
		printf("avdl: transpiling - " YEL "%d%%" RESET "\r", (int)((float) (i+1)/(avdl_da_count(&srcFiles)+1) *100));
		fflush(stdout);

		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);

	}
	Avdl_FileOp_GetFilesInDirectoryClean(&srcFiles);

	printf("avdl: transpiling - " GRN "100%%" RESET "\n");
	fflush(stdout);

	return 0;
}

int avdl_compile(struct AvdlSettings *avdl_settings) {

	if (avdl_settings->makefile_mode || avdl_settings->cmake_mode) {
		return 0;
	}

	if (avdl_settings->target_platform == AVDL_PLATFORM_ANDROID) {
		return avdl_android_object(avdl_settings);
	}
	else if (avdl_settings->target_platform == AVDL_PLATFORM_QUEST2) {
		avdl_quest2_object(avdl_settings);
		return 0;
	}
	else if (avdl_settings->target_platform == AVDL_PLATFORM_D3D11) {
		avdl_d3d11_object(avdl_settings);
		/*
		 * instructions after build is complete:
		 * * git clone https://github.com/ubawurinna/freetype-windows-binaries
		 * * cp include/<everything> avdl_build_d3d11/dependencies
		 * * cp "release dll"/<everything> avdl_build_d3d11/dependencies
		 */
		return 0;
	}

	// normal directories
	if (!is_dir("avdl_build")) {
		dir_create("avdl_build");
	}

	printf("avdl: compiling - " RED "0%%" RESET "\r");
	fflush(stdout);

	// collect avdl project src
	struct avdl_dynamic_array srcFiles;
	Avdl_FileOp_GetFilesInDirectory(cache_dir, &srcFiles);

	// filter out some files
	for (int i = 0; i < avdl_da_count(&srcFiles); i++) {
		struct avdl_string *str = avdl_da_get(&srcFiles, i);

		// skip non `.c` files
		if (!avdl_string_endsIn(str, ".c")) {
			continue;
		}

		// src file full path
		struct avdl_string srcFilePath;
		avdl_string_create(&srcFilePath, 1024);
		avdl_string_cat(&srcFilePath, cache_dir);
		avdl_string_cat(&srcFilePath, avdl_string_toCharPtr(str));
		if ( !avdl_string_isValid(&srcFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				cache_dir, avdl_string_toCharPtr(str),
				avdl_string_getError(&srcFilePath)
			);
			avdl_string_clean(&srcFilePath);
			return -1;
		}

		// on android just copy src file to its destination
		if (avdl_settings->target_platform == AVDL_PLATFORM_ANDROID) {
		/*
			struct avdl_string androidFilePath;
			avdl_string_create(&androidFilePath, 1024);
			avdl_string_cat(&androidFilePath, android_cache_dir "/app/src/main/cpp/engine/");
			avdl_string_cat(&androidFilePath, avdl_string_toCharPtr(str));
			if ( !avdl_string_isValid(&androidFilePath) ) {
				avdl_log_error("cannot construct path '%s%s': %s",
					android_cache_dir "/app/src/main/cpp/engine/", avdl_string_toCharPtr(str),
					avdl_string_getError(&androidFilePath)
				);
				avdl_string_clean(&srcFilePath);
				avdl_string_clean(&androidFilePath);
				return -1;
			}
			file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&androidFilePath), 0);
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&androidFilePath);
			*/
			continue;
		}

		// dst file full path
		struct avdl_string dstFilePath;
		avdl_string_create(&dstFilePath, 1024);
		avdl_string_cat(&dstFilePath, avdl_string_toCharPtr(&srcFilePath));
		avdl_string_cat(&dstFilePath, ".o");
		if ( !avdl_string_isValid(&dstFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				cache_dir, avdl_string_toCharPtr(str),
				avdl_string_getError(&dstFilePath)
			);
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
			continue;
		}

		// skip files already compiled (check last modified)
		// but if any header in `include/` has changed, compile everything
		if ( avdl_settings->use_cache
		&&   !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), avdl_string_toCharPtr(&srcFilePath))
		&&   !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), "include/") ) {
			//avdl_log("skipping file: %s", avdl_string_toCharPtr(&srcFilePath));
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&dstFilePath);
			continue;
		}

		//printf("compiling %s\n", dir->d_name);
		//avdl_log("compiling %s\n", avdl_string_toCharPtr(&srcFilePath));

		// command string
		struct avdl_string commandString;
		avdl_string_create(&commandString, 1024);
		avdl_string_cat(&commandString, "gcc -O3 -Wno-incompatible-pointer-types -DGLEW_NO_GLU -DAVDL_GAME_VERSION=\"\\\"");
		avdl_string_cat(&commandString, avdl_settings->version_name);
		avdl_string_cat(&commandString, "\\\"\" -DAVDL_GAME_REVISION=\"\\\"");
		avdl_string_cat(&commandString, "0");
		avdl_string_cat(&commandString, "\\\"\" -c -w ");
		//avdl_string_cat(&commandString, " -g -fsanitize=leak ");
		#if AVDL_IS_OS(AVDL_OS_WINDOWS)
		avdl_string_cat(&commandString, " -DAVDL_WINDOWS ");
		#elif AVDL_IS_OS(AVDL_OS_LINUX)
		avdl_string_cat(&commandString, " -DAVDL_LINUX ");
		#endif
		// cengine headers
		avdl_string_cat(&commandString, " -I ");
		avdl_string_cat(&commandString, avdl_settings->pkg_path);
		avdl_string_cat(&commandString, "/include ");
		avdl_string_cat(&commandString, " -I /usr/include/freetype2 ");

		avdl_string_cat(&commandString, avdl_string_toCharPtr(&srcFilePath));
		avdl_string_cat(&commandString, " -o ");
		avdl_string_cat(&commandString, avdl_string_toCharPtr(&dstFilePath));
		for (int i = 0; i < avdl_settings->total_include_directories; i++) {
			avdl_string_cat(&commandString, " -I ");
			avdl_string_cat(&commandString, avdl_settings->additional_include_directory[i]);
		}
		avdl_string_cat(&commandString, " -I include ");

		if ( !avdl_string_isValid(&commandString) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				cache_dir, avdl_string_toCharPtr(str),
				avdl_string_getError(&commandString)
			);
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&dstFilePath);
			avdl_string_clean(&commandString);
			return -1;
		}
		//printf("avdl compile command: %s\n", avdl_string_toCharPtr(&commandString));
		if (system(avdl_string_toCharPtr(&commandString))) {
			avdl_log_error("failed to compile file: " BLU "%s" RESET, avdl_string_toCharPtr(&srcFilePath));
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&dstFilePath);
			avdl_string_clean(&commandString);
			return -1;
		}

		printf("avdl: compiling - " YEL "%d%%" RESET "\r", (int)((float) (i+1)/(avdl_da_count(&srcFiles) +1) *100));
		fflush(stdout);

		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		avdl_string_clean(&commandString);
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&srcFiles);

	printf("avdl: compiling - " GRN "100%%" RESET "\n");
	fflush(stdout);

	return 0;
}

int avdl_compile_cengine(struct AvdlSettings *avdl_settings) {

	if (avdl_settings->makefile_mode || avdl_settings->cmake_mode) {
		return 0;
	}

	if (avdl_settings->target_platform == AVDL_PLATFORM_ANDROID) {
		return 0;
	}
	else if (avdl_settings->target_platform == AVDL_PLATFORM_QUEST2) {
		return 0;
	}
	else if (avdl_settings->target_platform == AVDL_PLATFORM_D3D11) {
		return 0;
	}

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


	struct avdl_dynamic_array cengineFiles;
	if (Avdl_FileOp_GetFilesInDirectoryRecursive(avdl_settings->cengine_path, &cengineFiles) != 0) {
		avdl_log_error("Can't get cengine files");
		return -1;
	}

	printf("avdl: compiling avdl engine - " RED "0%%" RESET "\r");
	fflush(stdout);
	char compile_command[6000];
	for (int i = 0; i < avdl_da_count(&cengineFiles); i++) {
		struct avdl_string *str = avdl_da_get(&cengineFiles, i);

		if (!avdl_string_endsIn(str, ".c") && !avdl_string_endsIn(str, ".cpp")) {
			continue;
		}

		struct avdl_string cEngFile;
		avdl_string_create(&cEngFile, 1024);
		avdl_string_cat(&cEngFile, avdl_string_toCharPtr(str));
		if (avdl_string_endsIn(&cEngFile, ".cpp")) {
			strcpy(compile_command, "g++ -c -DGLEW_NO_GLU ");
		}
		else {
			strcpy(compile_command, "gcc -Wno-incompatible-pointer-types -c -DGLEW_NO_GLU ");
		}
		avdl_string_clean(&cEngFile);

		//strcat(compile_command, " -g -fsanitize=leak ");

		#if AVDL_IS_OS(AVDL_OS_WINDOWS)
		strcat(compile_command, " -DAVDL_WINDOWS ");
		#elif AVDL_IS_OS(AVDL_OS_LINUX)
		strcat(compile_command, " -DAVDL_LINUX ");
		#endif

		// include the source file
		strcat(compile_command, avdl_settings->cengine_path);
		strcat(compile_command, avdl_string_toCharPtr(str));

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
		avdl_string_cat(&cenginePathOut, avdl_string_toCharPtr(str));
		if (avdl_string_endsIn(&cenginePathOut, ".cpp")) {
			avdl_string_replaceEnding(&cenginePathOut, ".cpp", ".o");
		}
		else {
			avdl_string_replaceEnding(&cenginePathOut, ".c", ".o");
		}
		Avdl_FileOp_CreateSubDirectories(0, avdl_string_toCharPtr(&cenginePathOut));

		if ( !avdl_string_isValid(&cenginePathOut) ) {
			avdl_log_error("cannot construct path '%s%s%s': %s", outdir, "cengine/", avdl_string_toCharPtr(str), avdl_string_getError(&cenginePathOut));
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
		for (int i = 0; i < avdl_settings->total_include_directories; i++) {
			strcat(compile_command, " -I ");
			strcat(compile_command, avdl_settings->additional_include_directory[i]);
		}

		// skip files already compiled
		if ( avdl_settings->use_cache && Avdl_FileOp_DoesFileExist(avdl_string_toCharPtr(&cenginePathOut)) ) {
			//printf("skipping: %s\n", buffer);
			printf("avdl: compiling avdl engine - " YEL "%d%%" RESET "\r", (int)((float) (i+1)/(avdl_da_count(&cengineFiles)+1) *100));
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
			printf("avdl: compiling avdl engine - " YEL "%d%%" RESET "\r", (int)((float) (i+1)/(avdl_da_count(&cengineFiles)+1) *100));
			fflush(stdout);
////		}
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&cengineFiles);
////	if (!avdlQuietMode) {
		printf("avdl: compiling avdl engine - " GRN "100%%" RESET "\n");
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

int avdl_link(struct AvdlSettings *avdl_settings) {

	if (avdl_settings->makefile_mode || avdl_settings->cmake_mode) {
		return 0;
	}

	if (avdl_settings->target_platform == AVDL_PLATFORM_ANDROID) {
		return avdl_link_android(avdl_settings);
	}
	else if (avdl_settings->target_platform == AVDL_PLATFORM_QUEST2) {
		return 0;
	}
	else if (avdl_settings->target_platform == AVDL_PLATFORM_D3D11) {
		return 0;
	}

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
		//avdl_string_cat(&link_cmd, " -g -fsanitize=leak ");
	}

	// possibly not needed when linking
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	avdl_string_cat(&link_cmd, " -DAVDL_WINDOWS ");
	#elif AVDL_IS_OS(AVDL_OS_LINUX)
	avdl_string_cat(&link_cmd, " -DAVDL_LINUX ");
	#endif

	// collect avdl project assets
	struct avdl_dynamic_array objFiles;
	Avdl_FileOp_GetFilesInDirectory(avdl_settings->src_dir, &objFiles);

	struct avdl_string objFilesStr;
	avdl_string_create(&objFilesStr, 100000);

	// filter out some files
	for (int i = 0; i < avdl_da_count(&objFiles); i++) {
		struct avdl_string *str = avdl_da_get(&objFiles, i);

		if (strcmp("." , avdl_string_toCharPtr(str)) == 0
		||  strcmp("..", avdl_string_toCharPtr(str)) == 0) {
			continue;
		}
		else {
			avdl_string_cat(&link_cmd, ".avdl_cache/");
			avdl_string_cat(&link_cmd, avdl_string_toCharPtr(str));
			avdl_string_cat(&link_cmd, ".c.o ");
		}

	}
	Avdl_FileOp_GetFilesInDirectoryClean(&objFiles);

	struct avdl_dynamic_array cengineFiles;
	if (Avdl_FileOp_GetFilesInDirectoryRecursive(avdl_settings->cengine_path, &cengineFiles) != 0) {
		avdl_log_error("Can't get cengine files");
		return -1;
	}

	// add cengine files to link
	for (int i = 0; i < avdl_da_count(&cengineFiles); i++) {
		struct avdl_string *str = avdl_da_get(&cengineFiles, i);

		if (!avdl_string_endsIn(str, ".c") && !avdl_string_endsIn(str, ".cpp")) {
			continue;
		}

		avdl_string_cat(&link_cmd, ".avdl_cache/cengine/");

		struct avdl_string tempfile;
		avdl_string_create(&tempfile, 1024);
		avdl_string_cat(&tempfile, avdl_string_toCharPtr(str));
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
	Avdl_FileOp_GetFilesInDirectoryClean(&cengineFiles);

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
	//printf("link command: %s\n", avdl_string_toCharPtr(&link_cmd));
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

int avdl_link_android(struct AvdlSettings *avdl_settings) {


	#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
	// check java requirements
	FILE *f = popen("java -version 2>&1", "r");
	if (!f) {
		avdl_log_error("can't find java using `java --version`");
		return -1;
	}

	char buffer[128];
	char buffer2[128];
	buffer2[0] = '\0';
	char *p;
	while (fgets(buffer, 128, f)) {
		if (strstr(buffer, " version ")) {
			p = strstr(buffer, "\"");
			if (!p) {
				continue;
			}
			if ((p+1)[0] == '\0') {
				continue;
			}
			p++;
			char *l = p;
			while (l[0] != '"' && l[0] != '\0') l++;
			if (l[0] == '\0') {
				continue;
			}
			strncpy(buffer2, p, (l-p));
			break;
		}
	}
	pclose(f);
	if (strlen(buffer2) <= 0) {
		avdl_log_error("unable to parse java version:");
		system("java -version");
		return -1;
	}

	if (strncmp(buffer2, "17", 2) != 0) {
		avdl_log_error("could not detect Java 17, android builds can only be made with Java 17");
		return -1;
	}
	#endif

	// get initial directory
	struct avdl_string initialDirectory;
	avdl_string_create(&initialDirectory, 1024);
	Avdl_FileOp_GetCurrentDirectory(&initialDirectory);
	if ( !avdl_string_isValid(&initialDirectory) ) {
		avdl_log_error("link_android: cannot construct current directory: %s", avdl_string_getError(&initialDirectory));
		avdl_string_clean(&initialDirectory);
		return -1;
	}

	if (chdir(android_cache_dir) != 0) {
		avdl_log_error("unable to change to android directory");
		return -1;
	}

	avdl_log("avdl: generating apk, this might take a while ...");

	if (system("./gradlew build >> .avdl_android_log") != 0) {

		if (chdir(avdl_string_toCharPtr(&initialDirectory)) != 0) {
			avdl_log_error("unable to change to default directory");
			return -1;
		}

		avdl_log_error("error creating android apks, see logs in .avdl_cache/android/.avdl_android_log");
		return -1;
	}

	if (chdir(avdl_string_toCharPtr(&initialDirectory)) != 0) {
		avdl_log_error("unable to change to default directory");
		return -1;
	}

	// copy results to output
	if (!is_dir("avdl_build_android")) {
		dir_create("avdl_build_android");
	}

	struct avdl_string projectReleaseApk;
	avdl_string_create(&projectReleaseApk, 1024);
	avdl_string_cat(&projectReleaseApk, "avdl_build_android/");
	avdl_string_cat(&projectReleaseApk, avdl_settings->project_name_code);
	avdl_string_cat(&projectReleaseApk, "-release-unsigned.apk");
	if ( !avdl_string_isValid(&projectReleaseApk) ) {
		avdl_log_error("link_android: cannot construct release apk name: %s", avdl_string_getError(&projectReleaseApk));
		avdl_string_clean(&projectReleaseApk);
		return -1;
	}

	struct avdl_string projectDebugApk;
	avdl_string_create(&projectDebugApk, 1024);
	avdl_string_cat(&projectDebugApk, "avdl_build_android/");
	avdl_string_cat(&projectDebugApk, avdl_settings->project_name_code);
	avdl_string_cat(&projectDebugApk, "-debug.apk");
	if ( !avdl_string_isValid(&projectDebugApk) ) {
		avdl_log_error("link_android: cannot construct debug apk name: %s", avdl_string_getError(&projectDebugApk));
		avdl_string_clean(&projectDebugApk);
		return -1;
	}

	file_copy(android_cache_dir "/app/build/outputs/apk/release/app-universal-release-unsigned.apk", avdl_string_toCharPtr(&projectReleaseApk), 0);
	file_copy(android_cache_dir "/app/build/outputs/apk/debug/app-universal-debug.apk", avdl_string_toCharPtr(&projectDebugApk), 0);
	file_copy(android_cache_dir "/app/build/outputs/native-debug-symbols/release/native-debug-symbols.zip", "avdl_build_android/native-debug-symbols.zip", 0);
	file_copy(android_cache_dir "/app/build/outputs/mapping/release/mapping.txt", "avdl_build_android/mapping.txt", 0);

	return 0;
}

// handle assets and put them in the final build
int avdl_assets(struct AvdlSettings *avdl_settings) {

	if (avdl_settings->makefile_mode || avdl_settings->cmake_mode) {
		return 0;
	}

	if (avdl_settings->target_platform == AVDL_PLATFORM_LINUX) {
		if (!is_dir("avdl_build")) {
			dir_create("avdl_build");
		}

		if (!is_dir("avdl_build/assets/")) {
			dir_create("avdl_build/assets/");
		}
	}

	printf("avdl: assets - " RED "0%%" RESET "\r");
	fflush(stdout);

	/*
	int outDir = open("avdl_build_d3d11/", O_DIRECTORY);
	if (!outDir) {
		printf("avdl: can't open %s: %s\n", "avdl_build_d3d11/", strerror(errno));
		return -1;
	}
	*/
	// collect avdl project assets
	struct avdl_dynamic_array assetFiles;
	Avdl_FileOp_GetFilesInDirectory(avdl_settings->asset_dir, &assetFiles);

	// filter out some files
	for (int i = 0; i < avdl_da_count(&assetFiles); i++) {
		struct avdl_string *str = avdl_da_get(&assetFiles, i);

		// ignore `.` and `..`
		if (strcmp(avdl_string_toCharPtr(str), ".") == 0
		||  strcmp(avdl_string_toCharPtr(str), "..") == 0) {
			continue;
		}

		// sanity check
		if (strstr(avdl_string_toCharPtr(str), "-")) {
			avdl_log_error("asset contains invalid character '-': " BLU "%s" RESET, avdl_string_toCharPtr(str));
			return -1;
		}

		// src file full path
		struct avdl_string srcFilePath;
		avdl_string_create(&srcFilePath, 1024);
		avdl_string_cat(&srcFilePath, avdl_settings->asset_dir);
		avdl_string_cat(&srcFilePath, avdl_string_toCharPtr(str));
		if ( !avdl_string_isValid(&srcFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				avdl_settings->asset_dir, avdl_string_toCharPtr(str),
				avdl_string_getError(&srcFilePath)
			);
			avdl_string_clean(&srcFilePath);
			return -1;
		}

		// on android, put assets in a specific directory
		if (avdl_settings->target_platform == AVDL_PLATFORM_ANDROID) {
			char *assetDir;

			if (avdl_string_endsIn(str, ".wav")
			||  avdl_string_endsIn(str, ".ogg")) {
				assetDir = "res/raw";
			}
			else
			if (avdl_string_endsIn(str, ".bmp")
			||  avdl_string_endsIn(str, ".png")) {
				assetDir = "res/drawable";
			}
			else {
				assetDir = "assets";
			}

			// android file full path
			struct avdl_string androidFilePath;
			avdl_string_create(&androidFilePath, 1024);
			avdl_string_cat(&androidFilePath, android_cache_dir "/app/src/main/");
			avdl_string_cat(&androidFilePath, assetDir);
			avdl_string_cat(&androidFilePath, "/");
			if ( !avdl_string_isValid(&androidFilePath) ) {
				avdl_log_error("cannot construct android file path: %s", avdl_string_getError(&androidFilePath));
				avdl_string_clean(&srcFilePath);
				avdl_string_clean(&androidFilePath);
				return -1;
			}
			dir_create(avdl_string_toCharPtr(&androidFilePath));
			avdl_string_cat(&androidFilePath, avdl_string_toCharPtr(str));
			if ( !avdl_string_isValid(&androidFilePath) ) {
				avdl_log_error("cannot construct android file path #2: %s", avdl_string_getError(&androidFilePath));
				avdl_string_clean(&srcFilePath);
				avdl_string_clean(&androidFilePath);
				return -1;
			}

			if (Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&androidFilePath), avdl_string_toCharPtr(&srcFilePath)) ) {
				file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&androidFilePath), 0);
			}
			avdl_string_clean(&androidFilePath);

			avdl_string_clean(&srcFilePath);
			continue;
		}
		else
		// on quest2, put assets in a specific directory
		if (avdl_settings->target_platform == AVDL_PLATFORM_QUEST2) {
			char *assetDir = "";

			if (avdl_string_endsIn(str, ".ogg")
			||  avdl_string_endsIn(str, ".wav")) {
				assetDir = "res/raw";
			}
			else
			if (avdl_string_endsIn(str, ".bmp")
			||  avdl_string_endsIn(str, ".png")) {
				assetDir = "res/drawable";
			}
			else {
				assetDir = "assets";
			}

			// quest2 file full path
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
			avdl_string_cat(&androidFilePath, avdl_string_toCharPtr(str));

			file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&androidFilePath), 0);
			avdl_string_clean(&androidFilePath);

			avdl_string_clean(&srcFilePath);
			continue;
		}
		else
		// on d3d11, put assets in a specific directory
		if (avdl_settings->target_platform == AVDL_PLATFORM_D3D11) {
			char *assetDir = "assets";

			if (!is_dir("avdl_build_d3d11/assets/")) {
				dir_create("avdl_build_d3d11/assets/");
			}

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
			avdl_string_cat(&d3d11FilePath, avdl_string_toCharPtr(str));

			file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&d3d11FilePath), 0);
			avdl_string_clean(&d3d11FilePath);

			avdl_string_clean(&srcFilePath);
			continue;
		}

		char *outdir = "avdl_build/assets/";

		// dst file full path
		struct avdl_string dstFilePath;
		avdl_string_create(&dstFilePath, 1024);
		avdl_string_cat(&dstFilePath, outdir);
		avdl_string_cat(&dstFilePath, avdl_string_toCharPtr(str));
		if ( !avdl_string_isValid(&dstFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				outdir, avdl_string_toCharPtr(str),
				avdl_string_getError(&dstFilePath)
			);
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
			continue;
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
			continue;
		}

		// skip files already compiled (check last modified)
		// but if any header in `include/` has changed, compile everything
		if ( avdl_settings->use_cache && !Avdl_FileOp_IsFileOlderThan(avdl_string_toCharPtr(&dstFilePath), avdl_string_toCharPtr(&srcFilePath)) ) {
			//printf("avdl asset file not modified, skipping handling of '%s'\n", dir->d_name);
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&dstFilePath);
			continue;
		}
		//printf("handling %s\n", dir->d_name);

		/*
		 * Currently assets are just copy-pasted,
		 * however on a future version there will be more fine
		 * control of editing files to supported formats
		 * and throwing errors on unsupported formats.
		 */
		file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&dstFilePath), 0);

		printf("avdl: assets - " YEL "%d%%" RESET "\r", (int)((float) (i+1)/(avdl_da_count(&assetFiles)+1) *100));
		fflush(stdout);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&assetFiles);

	if (avdl_settings->target_platform == AVDL_PLATFORM_D3D11) {

		struct avdl_string assetFilesStr;
		avdl_string_create(&assetFilesStr, 100000);

		// filter out some files
		for (int i = 0; i < avdl_da_count(&assetFiles); i++) {
			struct avdl_string *str = avdl_da_get(&assetFiles, i);
			if (strcmp("." , avdl_string_toCharPtr(str)) == 0
			||  strcmp("..", avdl_string_toCharPtr(str)) == 0) {
				avdl_da_remove(&assetFiles, 1, i);
				i--;
				continue;
			}

			// images (textures)
			if (avdl_string_endsIn(str, ".png")) {
				avdl_string_cat(&assetFilesStr, "  <ItemGroup>\n");
				avdl_string_cat(&assetFilesStr, "    <ImageContentTask Include=\"assets/");
				avdl_string_cat(&assetFilesStr, avdl_string_toCharPtr(str));
				avdl_string_cat(&assetFilesStr, "\">\n");
				avdl_string_cat(&assetFilesStr, "      <FileType>Image</FileType>\n");
				avdl_string_cat(&assetFilesStr, "      <DestinationFolders>$(OutDir)/assets</DestinationFolders>\n");
				avdl_string_cat(&assetFilesStr, "      <ContentOutput >$(OutDir)/assets/%(Filename).dds</ContentOutput>\n");
				avdl_string_cat(&assetFilesStr, "    </ImageContentTask>\n");
				avdl_string_cat(&assetFilesStr, "  </ItemGroup>\n");
			}
			else
			// json (localisation)
			// ply (3d meshes)
			// ttf (fonts)
			if (avdl_string_endsIn(str, ".json")
			||  avdl_string_endsIn(str, ".ply")
			||  avdl_string_endsIn(str, ".ttf")) {
				avdl_string_cat(&assetFilesStr, "  <ItemGroup>\n");
				avdl_string_cat(&assetFilesStr, "    <CopyFileToFolders Include=\"assets/");
				avdl_string_cat(&assetFilesStr, avdl_string_toCharPtr(str));
				avdl_string_cat(&assetFilesStr, "\">\n");
				avdl_string_cat(&assetFilesStr, "      <FileType>Document</FileType>\n");
				avdl_string_cat(&assetFilesStr, "      <DestinationFolders>$(OutDir)/assets</DestinationFolders>\n");
				avdl_string_cat(&assetFilesStr, "    </CopyFileToFolders>\n");
				avdl_string_cat(&assetFilesStr, "  </ItemGroup>\n");
			}
			// asset unsupported for d3d11
			else {
			}
		}

		if (!avdl_string_isValid(&assetFilesStr)) {
			avdl_log_error("cannot construct asset files str");
			return -1;
		}

		file_replace(0, "avdl_build_d3d11/avdl_project.vcxproj.in4",
			0, "avdl_build_d3d11/avdl_project.vcxproj",
			"%AVDL_PROJECT_ASSETS%", avdl_string_toCharPtr(&assetFilesStr)
		);
		avdl_string_clean(&assetFilesStr);

		// package file
		file_replace(0, "avdl_build_d3d11/Package.appxmanifest.in",
			0, "avdl_build_d3d11/Package.appxmanifest.in2",
			"%AVDL_WINDOWS_ID%",
			"8047DarkDimension.Rue-CardGame"
		);
		file_replace(0, "avdl_build_d3d11/Package.appxmanifest.in2",
			0, "avdl_build_d3d11/Package.appxmanifest.in3",
			"%AVDL_VERSION%",
			"1.0.0.0"
		);
		file_replace(0, "avdl_build_d3d11/Package.appxmanifest.in3",
			0, "avdl_build_d3d11/Package.appxmanifest.in4",
			"%AVDL_WINDOWS_PUBLISHER%",
			"CN=F02BE368-CAA8-48A5-ACFD-482F4512EC85"
		);

		file_replace(0, "avdl_build_d3d11/Package.appxmanifest.in4",
			0, "avdl_build_d3d11/Package.appxmanifest.in5",
			"%AVDL_PROJECT_NAME%",
			"Rue - Card Game"
		);
		file_replace(0, "avdl_build_d3d11/Package.appxmanifest.in5",
			0, "avdl_build_d3d11/Package.appxmanifest.in6",
			"%AVDL_PUBLISHER_NAME%",
			"Afloofdev"
		);
		file_replace(0, "avdl_build_d3d11/Package.appxmanifest.in6",
			0, "avdl_build_d3d11/Package.appxmanifest.in7",
			"%AVDL_PROJECT_NAME2%",
			"Rue - Card Game 2"
		);
		file_replace(0, "avdl_build_d3d11/Package.appxmanifest.in7",
			0, "avdl_build_d3d11/Package.appxmanifest",
			"%AVDL_PROJECT_DESCRIPTION%",
			"Avdl Description"
		);
	}

	printf("avdl: assets - " GRN "100%%" RESET "\n");
	fflush(stdout);

	return 0;
}

int avdl_directories(struct AvdlSettings *avdl_settings) {

	if (!is_dir(".avdl_cache")) {
		dir_create(".avdl_cache");
	}

	if (avdl_settings->target_platform == AVDL_PLATFORM_ANDROID) {
		create_android_directory(android_cache_dir);
	}
	else if (avdl_settings->target_platform == AVDL_PLATFORM_QUEST2) {
		create_quest2_directory("avdl_build_quest2");
	}
	else if (avdl_settings->target_platform == AVDL_PLATFORM_D3D11) {
		create_d3d11_directory(avdl_settings, "avdl_build_d3d11");
	}

	return 0;

}
int avdl_metadata(struct AvdlSettings *avdl_settings) {

	// create big icon
	if (Avdl_FileOp_IsFileOlderThan(".avdl_cache/icon_768x768.png", "metadata/icon_background.png")) {
		if (system(IMAGEMAGICK_COMPOSITE " -quiet metadata/icon_foreground.png metadata/icon_background.png -resize 768 .avdl_cache/icon_768x768.png") != 0) {
			avdl_log_error("could not create icon 768x768 using ImageMagick");
			return -1;
		}
	}

	// create cropped icon
	if (Avdl_FileOp_IsFileOlderThan(".avdl_cache/icon_cropped_512x512.png", ".avdl_cache/icon_768x768.png")) {
		if (system(IMAGEMAGICK_CONVERT " -quiet .avdl_cache/icon_768x768.png -gravity center -crop 512x512+0+0 +repage .avdl_cache/icon_cropped_512x512.png") != 0) {
			avdl_log_error("could not create cropped icon 512x512 using ImageMagick, retrying with `convert`");
			if (system("convert -quiet .avdl_cache/icon_768x768.png -gravity center -crop 512x512+0+0 +repage .avdl_cache/icon_cropped_512x512.png") != 0) {
				avdl_log_error("could not create cropped icon 512x512 using ImageMagick again");
				return -1;
			}
		}
	}
	if (Avdl_FileOp_IsFileOlderThan(".avdl_cache/icon_cropped_256x256.png", ".avdl_cache/icon_cropped_512x512.png")) {
		if (system(IMAGEMAGICK_CONVERT " -quiet .avdl_cache/icon_cropped_512x512.png -resize 256 .avdl_cache/icon_cropped_256x256.png") != 0) {
			avdl_log_error("could not create cropped icon 256x256 using ImageMagick, retrying with `convert`");
			if (system("convert -quiet .avdl_cache/icon_cropped_512x512.png -resize 256 .avdl_cache/icon_cropped_256x256.png") != 0) {
				avdl_log_error("could not create cropped icon 256x256 using ImageMagick again");
				return -1;
			}
		}
	}

	// create small icon - Linux and Windows
	if (avdl_settings->target_platform == AVDL_PLATFORM_LINUX
	||  avdl_settings->target_platform == AVDL_PLATFORM_WINDOWS) {
		if (Avdl_FileOp_IsFileOlderThan("assets/icon_64x64.png", "metadata/icon_foreground.png")
		||  Avdl_FileOp_IsFileOlderThan("assets/icon_64x64.png", "metadata/icon_background.png")) {
			if (system(IMAGEMAGICK_COMPOSITE " -quiet metadata/icon_foreground.png metadata/icon_background.png -resize 64 assets/icon_64x64.png") != 0) {
				avdl_log_error("could not create icon 64x64 using ImageMagick");
				return -1;
			}
		}
	}

	// create backwards compatibility icon for android
	if (avdl_settings->target_platform == AVDL_PLATFORM_ANDROID) {
		if (Avdl_FileOp_IsFileOlderThan(android_cache_dir "/app/src/main/res/drawable/icon.png", ".avdl_cache/icon_cropped_512x512.png")) {
			if (file_copy(".avdl_cache/icon_cropped_512x512.png", android_cache_dir "/app/src/main/res/drawable/icon.png", 0) != 0) {
				avdl_log_error("could not create backwards compatibility icon for Android using ImageMagick");
				return -1;
			}
		}
	}
	else
	if (avdl_settings->target_platform == AVDL_PLATFORM_QUEST2) {
		if (!is_dir("avdl_build_quest2/res/drawable")) {
			dir_create("avdl_build_quest2/res/drawable");
		}
		if (file_copy(".avdl_cache/icon_cropped_512x512.png", "avdl_build_quest2/res/drawable/icon.png", 0) != 0) {
			avdl_log_error("could not create backwards compatibility icon for Quest 2 using ImageMagick");
			return -1;
		}
	}

	// create ico for windows
	if (avdl_settings->target_platform == AVDL_PLATFORM_WINDOWS) {

		if (system(IMAGEMAGICK_CONVERT " -quiet .avdl_cache/icon_cropped_256x256.png " IMAGEMAGICK_PAREN_OPEN " -clone 0 -resize 16 " IMAGEMAGICK_PAREN_CLOSE " " IMAGEMAGICK_PAREN_OPEN " -clone 0 -resize 24 " IMAGEMAGICK_PAREN_CLOSE " " IMAGEMAGICK_PAREN_OPEN " -clone 0 -resize 32 " IMAGEMAGICK_PAREN_CLOSE " " IMAGEMAGICK_PAREN_OPEN " -clone 0 -resize 48 " IMAGEMAGICK_PAREN_CLOSE " "IMAGEMAGICK_PAREN_OPEN " -clone 0 -resize 64 " IMAGEMAGICK_PAREN_CLOSE " metadata/icon.ico") != 0) {
			avdl_log_error("could not create ICO for Windows using ImageMagick, retring with `convert`");
			if (system(IMAGEMAGICK_CONVERT " -quiet .avdl_cache/icon_cropped_256x256.png " IMAGEMAGICK_PAREN_OPEN " -clone 0 -resize 16 " IMAGEMAGICK_PAREN_CLOSE " " IMAGEMAGICK_PAREN_OPEN " -clone 0 -resize 24 " IMAGEMAGICK_PAREN_CLOSE " " IMAGEMAGICK_PAREN_OPEN " -clone 0 -resize 32 " IMAGEMAGICK_PAREN_CLOSE " " IMAGEMAGICK_PAREN_OPEN " -clone 0 -resize 48 " IMAGEMAGICK_PAREN_CLOSE " "IMAGEMAGICK_PAREN_OPEN " -clone 0 -resize 64 " IMAGEMAGICK_PAREN_CLOSE " metadata/icon.ico") != 0) {
				avdl_log_error("could not create ICO for Windows using ImageMagick again");
				return -1;
			}
		}

		// resource file for windows
		struct avdl_string rcGenCmd;
		avdl_string_create(&rcGenCmd, 1024);
		avdl_string_cat(&rcGenCmd, "echo 'IDI_ICON1 ICON DISCARDABLE \"metadata/icon.ico\"' > ");
		avdl_string_cat(&rcGenCmd, avdl_settings->project_name_code);
		avdl_string_cat(&rcGenCmd, ".rc");
		if ( !avdl_string_isValid(&rcGenCmd) ) {
			avdl_log_error("could not create command to generate resource file for Windows");
			return -1;
		}

		if (system(avdl_string_toCharPtr(&rcGenCmd)) != 0) {
			avdl_log_error("could not create resource file for Windows");
			return -1;
		}
	}

	// Direct3D11
	if (avdl_settings->target_platform == AVDL_PLATFORM_D3D11) {

		#if !AVDL_IS_OS(AVDL_OS_WINDOWS)

		printf("avdl: metadata - " RED "%d%%" RESET "\r", 0);
		fflush(stdout);

		// check imagemagick is present
		if (system("magick --version > /dev/null")) {
			avdl_log_error("could not use imagemagick's `convert`");
			return -1;
		}

		printf("avdl: metadata - " YEL "%d%%" RESET "\r", (int)((float) (1)/8 *100));
		fflush(stdout);

		// create metadata directory
		if (!is_dir("avdl_build_d3d11/metadata")) {
			dir_create("avdl_build_d3d11/metadata");
		}

		printf("avdl: metadata - " YEL "%d%%" RESET "\r", (int)((float) (2)/8 *100));
		fflush(stdout);

		// create store logo
		/*
		if (system("magick xc:red -resize 200x200 avdl_build_d3d11/assets/avdl_logo_store.scale-400.png")) {
			avdl_log_error("could not create store logo");
			return -1;
		}
		*/
		if (system("magick xc:red -resize 50x50 avdl_build_d3d11/metadata/avdl_logo_store.png")) {
			avdl_log_error("could not create store logo");
			return -1;
		}

		printf("avdl: metadata - " YEL "%d%%" RESET "\r", (int)((float) (3)/8 *100));
		fflush(stdout);

		// logo square 150x150
		if (system("magick xc:red -resize 150x150 avdl_build_d3d11/metadata/avdl_logo_square_150x150.png")) {
			avdl_log_error("could not create square logo 150x150");
			return -1;
		}
		/*
		if (system("magick xc:red -resize 600x600 avdl_build_d3d11/metadata/avdl_logo_square_150x150.scale-400.png")) {
			avdl_log_error("could not create square logo 150x150");
			return -1;
		}
		*/

		printf("avdl: metadata - " YEL "%d%%" RESET "\r", (int)((float) (4)/8 *100));
		fflush(stdout);

		// logo square 44x44
		if (system("magick xc:red -resize 44x44 avdl_build_d3d11/metadata/avdl_logo_square_44x44.png")) {
			avdl_log_error("could not create square logo 150x150");
			return -1;
		}
		/*
		if (system("magick xc:red -resize 256x256 avdl_build_d3d11/metadata/avdl_logo_square_44x44.scale-400.png")) {
			avdl_log_error("could not create square logo 150x150");
			return -1;
		}
		*/

		printf("avdl: metadata - " YEL "%d%%" RESET "\r", (int)((float) (5)/8 *100));
		fflush(stdout);

		// logo wide 310x150
		/*
		if (system("magick xc:red -resize 1240x600 avdl_build_d3d11/metadata/avdl_logo_wide_310x150.scale-400.png")) {
			avdl_log_error("could not create wide logo 310x150");
			return -1;
		}
		*/
		if (system("magick xc:red -resize 310x150! avdl_build_d3d11/metadata/avdl_logo_wide_310x150.png")) {
			avdl_log_error("could not create wide logo 310x150");
			return -1;
		}

		printf("avdl: metadata - " YEL "%d%%" RESET "\r", (int)((float) (6)/8 *100));
		fflush(stdout);

		// splash screen
		if (system("magick xc:red -resize 620x300! avdl_build_d3d11/metadata/avdl_splash_screen.png")) {
			avdl_log_error("could not create splash screen");
			return -1;
		}
		/*
		if (system("magick xc:red -resize 2480x1200 avdl_build_d3d11/metadata/avdl_splash_screen.scale-400.png")) {
			avdl_log_error("could not create splash screen");
			return -1;
		}
		*/

		printf("avdl: metadata - " YEL "%d%%" RESET "\r", (int)((float) (7)/8 *100));
		fflush(stdout);

		// lock screen logo
		if (system("magick xc:red -resize 24x24 avdl_build_d3d11/metadata/avdl_logo_lockscreen.png")) {
			avdl_log_error("could not create lockscreen logo");
			return -1;
		}
		/*
		if (system("magick xc:red -resize 96x96 avdl_build_d3d11/metadata/avdl_logo_lockscreen.scale-400.png")) {
			avdl_log_error("could not create lockscreen logo");
			return -1;
		}
		*/

		#endif
	}

	printf("avdl: metadata - " GRN "100%%" RESET "\n");
	fflush(stdout);

	// all good
	return 0;

}

int avdl_android_object(struct AvdlSettings *avdl_settings) {

	#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
	// put all object files to android
	dir_create(android_cache_dir "/app/src/main/cpp/game/");

	// collect avdl android project source
	struct avdl_string objFilesStr;
	avdl_string_create(&objFilesStr, 100000);
	struct avdl_dynamic_array objFiles;
	Avdl_FileOp_GetFilesInDirectory(android_cache_dir "/app/src/main/cpp/game/", &objFiles);
	for (int i = 0; i < avdl_da_count(&objFiles); i++) {
		struct avdl_string *str = avdl_da_get(&objFiles, i);

		if (!avdl_string_endsIn(str, ".c")) {
			continue;
		}
		else {
			avdl_string_cat(&objFilesStr, "game/");
			avdl_string_cat(&objFilesStr, avdl_string_toCharPtr(str));
			avdl_string_cat(&objFilesStr, " ");
		}
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&objFiles);

	if (!avdl_string_isValid(&objFilesStr)) {
		avdl_log_error("unable to construct obj files for android");
		return -1;
	}

	// cpp directory
	struct avdl_string cppFilePath;
	avdl_string_create(&cppFilePath, 1024);
	avdl_string_cat(&cppFilePath, android_cache_dir "/app/src/main/cpp/");
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
	avdl_string_clean(&cppFilePath);

	// add in the avdl-compiled source files
	file_replace(outDir, "CMakeLists.txt.in", outDir, "CMakeLists.txt.in2", "%AVDL_GAME_FILES%", avdl_string_toCharPtr(&objFilesStr));
	avdl_string_clean(&objFilesStr);

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
		avdl_string_clean(&cflags);
	}

	close(outDir);

	// handle versioning
	outDir = open(android_cache_dir "/app/", O_DIRECTORY);
	file_replace(outDir, "build.gradle.in", outDir, "build.gradle.in2", "%AVDL_PACKAGE_NAME%", avdl_settings->package);
	file_replace(outDir, "build.gradle.in2", outDir, "build.gradle.in3", "%AVDL_VERSION_CODE%", avdl_settings->version_code_str);
	file_replace(outDir, "build.gradle.in3", outDir, "build.gradle", "%AVDL_VERSION_NAME%", avdl_settings->version_name);
	file_remove(android_cache_dir "/app/build.gradle.in");
	file_remove(android_cache_dir "/app/build.gradle.in2");
	file_remove(android_cache_dir "/app/build.gradle.in3");
	file_remove(android_cache_dir "/app/build.gradle.in.googleplay");
	close(outDir);

	// add dependencies if needed
	if (avdl_settings->googleplay_mode || avdl_settings->admob_ads) {
		file_write(android_cache_dir "/app/build.gradle", "\ndependencies {\n", 1);
		if (avdl_settings->googleplay_mode) {
			file_write(android_cache_dir "/app/build.gradle", "\timplementation 'com.google.android.gms:play-services-games-v2:17.0.0'\n", 1);
		}

		if (avdl_settings->admob_ads) {
			file_write(android_cache_dir "/app/build.gradle", "\timplementation 'com.google.android.gms:play-services-ads:22.1.0'\n", 1);
		}
		file_write(android_cache_dir "/app/build.gradle", "}\n", 1);
	}

	/*
	// backwards compatible icon
	struct avdl_string iconPath;
	avdl_string_create(&iconPath, 1024);
	avdl_string_cat(&iconPath, android_cache_dir "/app/src/main/res/drawable/");
	avdl_string_cat(&iconPath, avdl_settings->icon_path);
	if (!avdl_string_isValid(&iconPath)) {
		avdl_log_error("unable to construct icon path: %s", avdl_string_getError(&iconPath));
		return -1;
	}
	file_copy(avdl_settings->icon_path, avdl_string_toCharPtr(&iconPath), 0);
	avdl_string_clean(&iconPath);
	*/

	// adaptable icon foreground
	struct avdl_string foregroundPath;
	avdl_string_create(&foregroundPath, 1024);
	avdl_string_cat(&foregroundPath, android_cache_dir "/app/src/main/res/drawable/icon_foreground.png");
	if (!avdl_string_isValid(&foregroundPath)) {
		avdl_log_error("unable to construct icon path: %s", avdl_string_getError(&foregroundPath));
		return -1;
	}
	file_copy(avdl_settings->icon_foreground_path, avdl_string_toCharPtr(&foregroundPath), 0);
	avdl_string_clean(&foregroundPath);

	// adaptable icon background
	struct avdl_string backgroundPath;
	avdl_string_create(&backgroundPath, 1024);
	avdl_string_cat(&backgroundPath, android_cache_dir "/app/src/main/res/drawable/icon_background.png");
	if (!avdl_string_isValid(&backgroundPath)) {
		avdl_log_error("unable to construct icon path: %s", avdl_string_getError(&backgroundPath));
		return -1;
	}
	file_copy(avdl_settings->icon_background_path, avdl_string_toCharPtr(&backgroundPath), 0);
	avdl_string_clean(&backgroundPath);

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
	if (!is_dir(android_cache_dir "/app/src/main/res/values/")) {
		dir_create(android_cache_dir "/app/src/main/res/values/");
	}
	file_write(android_cache_dir "/app/src/main/res/values/strings.xml", avdl_string_toCharPtr(&values_file), 0);
	avdl_string_clean(&values_file);

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
		avdl_string_clean(&metadata);
		avdl_string_clean(&permissions);
		return -1;
	}

	if ( !avdl_string_isValid(&ads_imports) ) {
		avdl_log_error("cannot construct admob imports: %s", avdl_string_getError(&ads_imports));
		avdl_string_clean(&metadata);
		avdl_string_clean(&permissions);
		avdl_string_clean(&ads_imports);
		return -1;
	}

	if ( !avdl_string_isValid(&ads_declarations) ) {
		avdl_log_error("cannot construct admob declarations: %s", avdl_string_getError(&ads_declarations));
		avdl_string_clean(&metadata);
		avdl_string_clean(&permissions);
		avdl_string_clean(&ads_imports);
		avdl_string_clean(&ads_declarations);
		return -1;
	}

	if ( !avdl_string_isValid(&ads_init) ) {
		avdl_log_error("cannot construct admob init: %s", avdl_string_getError(&ads_init));
		avdl_string_clean(&metadata);
		avdl_string_clean(&permissions);
		avdl_string_clean(&ads_imports);
		avdl_string_clean(&ads_declarations);
		avdl_string_clean(&ads_init);
		return -1;
	}

	if ( !avdl_string_isValid(&ads_functions) ) {
		avdl_log_error("cannot construct admob functions: %s", avdl_string_getError(&ads_functions));
		avdl_string_clean(&metadata);
		avdl_string_clean(&permissions);
		avdl_string_clean(&ads_imports);
		avdl_string_clean(&ads_declarations);
		avdl_string_clean(&ads_init);
		avdl_string_clean(&ads_functions);
		return -1;
	}

	if (avdl_settings->googleplay_mode) {
		file_replace(0,
			android_cache_dir "/app/src/main/java/dev/afloof/avdl/AvdlActivity.java.in.googleplay", 0,
			android_cache_dir "/app/src/main/java/dev/afloof/avdl/AvdlActivity.java.in2",
			"%AVDL_ADS_IMPORT%",
			avdl_string_toCharPtr(&ads_imports)
		);
	}
	else {
		file_replace(0,
			android_cache_dir "/app/src/main/java/dev/afloof/avdl/AvdlActivity.java.in", 0,
			android_cache_dir "/app/src/main/java/dev/afloof/avdl/AvdlActivity.java.in2",
			"%AVDL_ADS_IMPORT%",
			avdl_string_toCharPtr(&ads_imports)
		);
	}
	file_replace(0,
		android_cache_dir "/app/src/main/java/dev/afloof/avdl/AvdlActivity.java.in2", 0,
		android_cache_dir "/app/src/main/java/dev/afloof/avdl/AvdlActivity.java.in3",
		"%AVDL_ADS_DECLARATIONS%",
		avdl_string_toCharPtr(&ads_declarations)
	);
	file_replace(0,
		android_cache_dir "/app/src/main/java/dev/afloof/avdl/AvdlActivity.java.in3", 0,
		android_cache_dir "/app/src/main/java/dev/afloof/avdl/AvdlActivity.java.in4",
		"%AVDL_ADS_INIT%",
		avdl_string_toCharPtr(&ads_init)
	);
	file_replace(0,
		android_cache_dir "/app/src/main/java/dev/afloof/avdl/AvdlActivity.java.in4", 0,
		android_cache_dir "/app/src/main/java/dev/afloof/avdl/AvdlActivity.java",
		"%AVDL_ADS_FUNCTIONS%",
		avdl_string_toCharPtr(&ads_functions)
	);

	file_replace(0,
		android_cache_dir "/app/src/main/AndroidManifest.xml.in", 0,
		android_cache_dir "/app/src/main/AndroidManifest.xml.in2",
		"%AVDL_METADATA%",
		avdl_string_toCharPtr(&metadata)
	);

	file_replace(0,
		android_cache_dir "/app/src/main/AndroidManifest.xml.in2", 0,
		android_cache_dir "/app/src/main/AndroidManifest.xml",
		"%AVDL_PERMISSIONS%",
		avdl_string_toCharPtr(&permissions)
	);

	file_remove(
		android_cache_dir "/app/src/main/AndroidManifest.xml.in2"
	);
	file_remove(
		android_cache_dir "/app/src/main/AndroidManifest.xml.in"
	);

	avdl_string_clean(&metadata);
	avdl_string_clean(&ads_functions);
	avdl_string_clean(&ads_init);
	avdl_string_clean(&ads_declarations);
	avdl_string_clean(&permissions);
	avdl_string_clean(&ads_imports);

	#endif
	return 0;
}

int avdl_quest2_object(struct AvdlSettings *avdl_settings) {

	#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
	// put all object files to android
	dir_create("avdl_build_quest2/src/");

	// copy project src files

	// collect quest2 project object
	struct avdl_string objFilesStr;
	avdl_string_create(&objFilesStr, 10000);
	struct avdl_dynamic_array objFiles;
	Avdl_FileOp_GetFilesInDirectory(".avdl_cache", &objFiles);
	for (int i = 0; i < avdl_da_count(&objFiles); i++) {
		struct avdl_string *str = avdl_da_get(&objFiles, i);

		if (strcmp(avdl_string_toCharPtr(str), "..") == 0
		||  strcmp(avdl_string_toCharPtr(str), "." ) == 0) {
			continue;
		}

		if (!avdl_string_endsIn(str, ".c")) {
			avdl_da_remove(&objFiles, 1, i);
			i--;
		}
		else {
			avdl_string_cat(&objFilesStr, "../../../src/");
			avdl_string_cat(&objFilesStr, avdl_string_toCharPtr(str));
			avdl_string_cat(&objFilesStr, " ");
		}

		#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
		// src file full path
		struct avdl_string srcFilePath;
		avdl_string_create(&srcFilePath, 1024);
		avdl_string_cat(&srcFilePath, ".avdl_cache/");
		avdl_string_cat(&srcFilePath, avdl_string_toCharPtr(str));
		if ( !avdl_string_isValid(&srcFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				".avdl_cache", avdl_string_toCharPtr(str),
				avdl_string_getError(&srcFilePath)
			);
			avdl_string_clean(&srcFilePath);
			return -1;
		}

		// dst file full path
		struct avdl_string dstFilePath;
		avdl_string_create(&dstFilePath, 1024);
		avdl_string_cat(&dstFilePath, "avdl_build_quest2/src/");
		avdl_string_cat(&dstFilePath, avdl_string_toCharPtr(str));
		if ( !avdl_string_isValid(&dstFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				android_cache_dir "/app/src/main/cpp/game/", avdl_string_toCharPtr(str),
				avdl_string_getError(&dstFilePath)
			);
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&dstFilePath);
			return -1;
		}

		file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&dstFilePath), 0);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		#endif
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&objFiles);

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
	avdl_string_clean(&cppFilePath);

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
		avdl_string_clean(&cflags);
	}

	file_replace(outDir, "Android.mk.in2", outDir, "Android.mk", "%AVDL_GAME_FILES%", avdl_string_toCharPtr(&objFilesStr));
	avdl_string_clean(&objFilesStr);
	close(outDir);

	// handle versioning
	outDir = open("avdl_build_quest2/Projects/Android/", O_DIRECTORY);
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

	/*
	struct avdl_string iconPath;
	avdl_string_create(&iconPath, 1024);
	avdl_string_cat(&iconPath, "avdl_build_quest2/");
	avdl_string_cat(&iconPath, "/res/drawable/");
	avdl_string_cat(&iconPath, avdl_settings->icon_path);
	if (!avdl_string_isValid(&iconPath)) {
		avdl_log_error("unable to construct icon path: %s", avdl_string_getError(&iconPath));
		return -1;
	}
	file_copy(avdl_settings->icon_path, avdl_string_toCharPtr(&iconPath), 0);
	avdl_string_clean(&iconPath);
	*/

	struct avdl_string foregroundPath;
	avdl_string_create(&foregroundPath, 1024);
	avdl_string_cat(&foregroundPath, "avdl_build_quest2/");
	avdl_string_cat(&foregroundPath, "/res/drawable/icon_foreground.png");
	if (!avdl_string_isValid(&foregroundPath)) {
		avdl_log_error("unable to construct foreground path: %s", avdl_string_getError(&foregroundPath));
		return -1;
	}
	file_copy(avdl_settings->icon_foreground_path, avdl_string_toCharPtr(&foregroundPath), 0);
	avdl_string_clean(&foregroundPath);

	struct avdl_string backgroundPath;
	avdl_string_create(&backgroundPath, 1024);
	avdl_string_cat(&backgroundPath, "avdl_build_quest2/");
	avdl_string_cat(&backgroundPath, "/res/drawable/icon_background.png");
	if (!avdl_string_isValid(&backgroundPath)) {
		avdl_log_error("unable to construct background path: %s", avdl_string_getError(&foregroundPath));
		return -1;
	}
	file_copy(avdl_settings->icon_background_path, avdl_string_toCharPtr(&backgroundPath), 0);
	avdl_string_clean(&backgroundPath);

	if (!is_dir("avdl_build_quest2/res/values/")) {
		dir_create("avdl_build_quest2/res/values/");
	}

	// project name
	outDir = open("avdl_build_quest2/res/values/", O_DIRECTORY);
	file_replace(outDir, "strings.xml.in", outDir, "strings.xml", "%AVDL_PROJECT_NAME%", avdl_settings->project_name);
	file_remove("avdl_build_quest2/res/values/strings.xml.in");
	close(outDir);
	#endif
	return 0;
}

int avdl_d3d11_object(struct AvdlSettings *avdl_settings) {

	#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
	// put all object files to android
	dir_create("avdl_build_d3d11/src/");

	// copy project src files

	// collect avdl project source
	struct avdl_string objFilesStr;
	avdl_string_create(&objFilesStr, 10000);
	struct avdl_dynamic_array objFiles;
	Avdl_FileOp_GetFilesInDirectory(".avdl_cache", &objFiles);
	for (int i = 0; i < avdl_da_count(&objFiles); i++) {
		struct avdl_string *str = avdl_da_get(&objFiles, i);
		if (!avdl_string_endsIn(str, ".c")) {
			continue;
		}
		else {
			avdl_string_cat(&objFilesStr, "    <ClCompile Include=\"src/");
			avdl_string_cat(&objFilesStr, avdl_string_toCharPtr(str));
			avdl_string_cat(&objFilesStr, "\">\n");
			avdl_string_cat(&objFilesStr, "      <CompileAsWinRT>false</CompileAsWinRT>\n");
			avdl_string_cat(&objFilesStr, "      <PrecompiledHeader>NotUsing</PrecompiledHeader>\n");
			avdl_string_cat(&objFilesStr, "    </ClCompile>\n");
		}

		#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
		// src file full path
		struct avdl_string srcFilePath;
		avdl_string_create(&srcFilePath, 1024);
		avdl_string_cat(&srcFilePath, ".avdl_cache/");
		avdl_string_cat(&srcFilePath, avdl_string_toCharPtr(str));
		if ( !avdl_string_isValid(&srcFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				".avdl_cache", avdl_string_toCharPtr(str),
				avdl_string_getError(&srcFilePath)
			);
			avdl_string_clean(&srcFilePath);
			return -1;
		}

		// dst file full path
		struct avdl_string dstFilePath;
		avdl_string_create(&dstFilePath, 1024);
		avdl_string_cat(&dstFilePath, "avdl_build_d3d11/src/");
		avdl_string_cat(&dstFilePath, avdl_string_toCharPtr(str));
		if ( !avdl_string_isValid(&dstFilePath) ) {
			avdl_log_error("cannot construct path '%s%s': %s",
				"avdl_build_d3d11/src/", avdl_string_toCharPtr(str),
				avdl_string_getError(&dstFilePath)
			);
			avdl_string_clean(&srcFilePath);
			avdl_string_clean(&dstFilePath);
			return -1;
		}

		file_copy(avdl_string_toCharPtr(&srcFilePath), avdl_string_toCharPtr(&dstFilePath), 0);
		avdl_string_clean(&srcFilePath);
		avdl_string_clean(&dstFilePath);
		#endif
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&objFiles);

	if (!avdl_string_isValid(&objFilesStr)) {
		avdl_log_error("unable to collect d3d11 object files");
		return -1;
	}

	int outDir = open("avdl_build_d3d11/", O_DIRECTORY);
	if (!outDir) {
		printf("avdl: can't open %s: %s\n", "avdl_build_d3d11/", strerror(errno));
		return -1;
	}

	file_replace(outDir, "avdl_project.vcxproj.in3",
		outDir, "avdl_project.vcxproj.in4",
		"%AVDL_PROJECT_SRC%", avdl_string_toCharPtr(&objFilesStr)
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

	if (!is_dir("avdl_build_d3d11/assets/")) {
		dir_create("avdl_build_d3d11/assets/");
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

int avdl_cmake(struct AvdlSettings *avdl_settings) {

	// copy cmake template to current directory
	struct avdl_string path;
	avdl_string_create(&path, 1024);
	avdl_string_cat(&path, avdl_pkg_GetProjectPath());
	avdl_string_cat(&path, "/share/avdl/templates/CMakeLists.txt");
	if ( !avdl_string_isValid(&path) ) {
		avdl_log_error("could not construct cmake path");
		avdl_string_clean(&path);
		return -1;
	}
	file_copy(avdl_string_toCharPtr(&path), "CMakeLists.txt", 0);
	avdl_string_clean(&path);

	// cengine files
	struct avdl_dynamic_array cengineFiles;
	if (Avdl_FileOp_GetFilesInDirectoryRecursive(avdl_settings->cengine_path, &cengineFiles) != 0) {
		avdl_log_error("Can't get cengine files");
		return -1;
	}

	// collect avdl source
	struct avdl_string avdl_src;
	avdl_string_create(&avdl_src, 5024);
	for (int i = 0; i < avdl_da_count(&cengineFiles); i++) {
		struct avdl_string *str = avdl_da_get(&cengineFiles, i);
		if (!avdl_string_endsIn(str, ".c") && !avdl_string_endsIn(str, ".cpp")) {
			continue;
		}
		avdl_string_cat(&avdl_src, "cengine/");
		avdl_string_cat(&avdl_src, avdl_string_toCharPtr(str));
		avdl_string_cat(&avdl_src, " ");
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&cengineFiles);
	if ( !avdl_string_isValid(&avdl_src) ) {
		avdl_log_error("could not construct avdl_src path");
		avdl_string_clean(&avdl_src);
		return -1;
	}

	// collect avdl project source
	struct avdl_dynamic_array srcFiles;
	Avdl_FileOp_GetFilesInDirectory(".avdl_cache", &srcFiles);
	for (int i = 0; i < avdl_da_count(&srcFiles); i++) {
		struct avdl_string *str = avdl_da_get(&srcFiles, i);
		if (!avdl_string_endsIn(str, ".dd.c")) {
			avdl_da_remove(&srcFiles, 1, i);
			i--;
		}
	}

	// project data in cmake
	struct avdl_string cmake_data;
	avdl_string_create(&cmake_data, 5024);
	avdl_string_cat(&cmake_data, "set(AVDL_PROJECT_NAME \"");
	avdl_string_cat(&cmake_data, avdl_settings->project_name);
	avdl_string_cat(&cmake_data, "\")\n");
	avdl_string_cat(&cmake_data, "set(AVDL_PROJECT_NAME_CODE ");
	avdl_string_cat(&cmake_data, avdl_settings->project_name_code);
	avdl_string_cat(&cmake_data, ")\n");
	avdl_string_cat(&cmake_data, "set(AVDL_VERSION_NAME ");
	avdl_string_cat(&cmake_data, avdl_settings->version_name);
	avdl_string_cat(&cmake_data, ")\n");
	avdl_string_cat(&cmake_data, "set(AVDL_SRC ");
	avdl_string_cat(&cmake_data, avdl_string_toCharPtr(&avdl_src));

	// avdl project src
	for (int i = 0; i < avdl_da_count(&srcFiles); i++) {
		struct avdl_string *str = avdl_da_get(&srcFiles, i);
		avdl_string_cat(&cmake_data, ".avdl_cache/");
		avdl_string_cat(&cmake_data, avdl_string_toCharPtr(str));
		avdl_string_cat(&cmake_data, " ");
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&srcFiles);

	avdl_string_cat(&cmake_data, avdl_settings->project_name_code);
	avdl_string_cat(&cmake_data, ".rc ");
	avdl_string_cat(&cmake_data, ")\n");
	avdl_string_cat(&cmake_data, "set(AVDL_ASSETS ");

	// collect avdl project assets
	struct avdl_dynamic_array assetFiles;
	Avdl_FileOp_GetFilesInDirectory("assets", &assetFiles);

	// put assets into cmake
	for (int i = 0; i < avdl_da_count(&assetFiles); i++) {
		struct avdl_string *str = avdl_da_get(&assetFiles, i);
		if (strcmp("." , avdl_string_toCharPtr(str)) == 0
		||  strcmp("..", avdl_string_toCharPtr(str)) == 0) {
			continue;
		}
		else {
			avdl_string_cat(&cmake_data, "assets/");
			avdl_string_cat(&cmake_data, avdl_string_toCharPtr(str));
			avdl_string_cat(&cmake_data, " ");
		}
	}
	Avdl_FileOp_GetFilesInDirectoryClean(&assetFiles);

	avdl_string_cat(&cmake_data, ")\n");
	if ( !avdl_string_isValid(&cmake_data) ) {
		avdl_log_error("could not construct cmake_data");
		avdl_string_clean(&cmake_data);
		return -1;
	}

	file_write("avdl_project.cmake", avdl_string_toCharPtr(&cmake_data), 0);

	avdl_string_clean(&avdl_src);
	avdl_string_clean(&cmake_data);
	return 0;
}

int avdl_makefile(struct AvdlSettings *avdl_settings) {

	// copy the makefile template to the current directory
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
	avdl_string_clean(&makefilePath);

	// inject project details to the makefile
	file_replace(0, ".makefile", 0, ".makefile1", "%AVDL_PROJECT_NAME%", avdl_settings->project_name);
	file_replace(0, ".makefile1", 0, ".makefile2", "%AVDL_PROJECT_NAME_CODE%", avdl_settings->project_name_code);
	file_replace(0, ".makefile2", 0, ".makefile3", "%AVDL_VERSION_NAME%", avdl_settings->version_name);
	file_replace(0, ".makefile3", 0, ".makefile4", "%AVDL_VERSION_CODE%", avdl_settings->version_code_str);
	file_replace(0, ".makefile4", 0, "makefile", "%AVDL_PACKAGE_NAME%", avdl_settings->package);
	file_remove(".makefile");
	file_remove(".makefile1");
	file_remove(".makefile2");
	file_remove(".makefile3");
	file_remove(".makefile4");

	return 0;
}
