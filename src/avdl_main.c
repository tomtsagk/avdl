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

#if !AVDL_IS_OS(AVDL_OS_WINDOWS)
#include <unistd.h>
#endif

// Test terminal colours
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

const char cache_dir[] = ".avdl_cache/";

extern float parsing_float;

char included_files[10][100];
int included_files_num = 0;

// buffer for general use
#define DD_BUFFER_SIZE 2048
char buffer[DD_BUFFER_SIZE];

// game node, parent of all nodes
struct ast_node *game_node;

#define MAX_INPUT_FILES 20

char *includePath = 0;

char *installLocation = "";
int installLocationDynamic = 0;
char *saveLocation = "";

char *additionalIncludeDirectory[10];
int totalIncludeDirectories = 0;
char *additionalLibDirectory[10];
int totalLibDirectories = 0;

int create_android_directory(const char *androidDirName);

int avdlSteamMode = 0;

int avdlQuietMode = 0;
int avdlStandalone = 0;

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
	{"dd_string3d.c", 0},
	{"dd_vec3.c", 0},
	{"dd_vec4.c", 0},
	{"dd_world.c", 0},
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
	"dd_string3d.h",
	"dd_vec2.h",
	"dd_vec3.h",
	"dd_vec4.h",
	"dd_world.h",
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

const char *cengine_path;

// init data, parse, exit
#ifdef AVDL_UNIT_TEST
int avdl_main(int argc, char *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif

	// project settings
	struct AvdlSettings avdl_settings;
	AvdlSettings_Create(&avdl_settings);

	if (AvdlSettings_SetFromFile(&avdl_settings, "app.avdl") != 0) {
		printf("avdl error: failed to get project settings from '%s'\n", "app.avdl");
		return -1;
	}

	// get cengine path
	cengine_path = avdl_pkg_GetCenginePath();
	if (!cengine_path) {
		printf("avdl error: cannot get cengine path\n");
		return -1;
	}

	/*
	printf("looking for source files in: %s\n", avdl_settings.src_dir);
	printf("looking for asset files in: %s\n", avdl_settings.asset_dir);
	*/
	printf("~ Project Details ~\n");
	printf("Project Name: %s\n", avdl_settings.project_name);
	printf("Version: %d (%s)-%d\n", avdl_settings.version_code, avdl_settings.version_name, avdl_settings.revision);
	printf("Icon: %s\n", avdl_settings.icon_path);
	printf("Package: %s\n", avdl_settings.package);
	printf("CEngine location in: %s\n", cengine_path);
	printf("~ Project Details end ~\n");
	printf("\n");

	if (!is_dir(".avdl_cache")) {
		dir_create(".avdl_cache");
	}

	// from `.dd` to `.c`
	if ( avdl_transpile(&avdl_settings) != 0) {
		printf("avdl: error transpiling project\n");
		return -1;
	}

	// from `.c` to `.o`
	if ( avdl_compile(&avdl_settings) != 0) {
		printf("avdl: error compiling project\n");
		return -1;
	}

	// cengine
	if ( avdl_compile_cengine(&avdl_settings) != 0) {
		printf("avdl: error compiling cengine\n");
		return -1;
	}

	// combine all `.o` to executable
	if ( avdl_link(&avdl_settings) != 0) {
		printf("avdl: error linking project\n");
		return -1;
	}

	// handle assets
	if ( avdl_assets(&avdl_settings) != 0) {
		printf("avdl: error handling project assets\n");
		return -1;
	}

	printf("avdl project " BLU "\"%s\"" RESET " compiled successfully at " BLU "./avdl_build/" RESET "\n", avdl_settings.project_name);

	return 0;

	avdl_platform_initialise();
	avdl_initProjectLocation();

	/* tweakable data
	 */
	//int show_ast = 0;
	int show_struct_table = 0;
	char filename[MAX_INPUT_FILES][100];
	int input_file_total = 0;
	char *outname = 0;
	char *gameName = "avdl_game";
	char *gameVersion = "0.0.0";
	char *gameRevision = "0";
	int getCengine = 0;

	// android versioning
	char *gameVersionCode = "0";
	char *gamePackageName = "com.company.appname";
	char *gameIconFlat = 0;
	char *gameIconForeground = 0;
	char *gameIconBackground = 0;

	/*
	 * phases
	 *
	 * translate: translate `.dd` files to `.c`
	 * compile: compile `.c` files to `.o`
	 * link: link all `.o` files to an executable
	 *
	 */
	int translate = 1;
	int compile = 1;
	int link = 1;

	// parse arguments
	for (int i = 1; i < argc; i++) {

		// dash argument
		if (strlen(argv[i]) > 0 && argv[i][0] == '-') {

			// double dash argument
			if (strlen(argv[i]) > 1 && argv[i][1] == '-') {

				// print abstract syntax tree
				if (strcmp(argv[i], "--print-ast") == 0) {
					//show_ast = 1;
				}
				else
				// print struct table
				if (strcmp(argv[i], "--print-struct-table") == 0) {
					show_struct_table = 1;
				}
				else
				// compiling for windows
				if (strcmp(argv[i], "--windows") == 0) {
					avdl_platform_set(AVDL_PLATFORM_WINDOWS);
				}
				else
				// compiling for linux
				if (strcmp(argv[i], "--linux") == 0) {
					avdl_platform_set(AVDL_PLATFORM_LINUX);
				}
				else
				// compiling for android
				if (strcmp(argv[i], "--android") == 0) {
					avdl_platform_set(AVDL_PLATFORM_ANDROID);
				}
				else
				// show version number
				if (strcmp(argv[i], "--version") == 0) {
					printf(PKG_NAME " v%s\n", PKG_VERSION);
					return 0;
				}
				else
				// show pkg location
				if (strcmp(argv[i], "--get-pkg-location") == 0) {
					#if AVDL_IS_OS(AVDL_OS_WINDOWS)
					wprintf(L"%lS\n", avdl_getProjectLocation());
					#else
					printf("%s\n", avdl_getProjectLocation());
					#endif
					return 0;
				}
				else
				// grab a copy of the cengine
				if (strcmp(argv[i], "--get-cengine") == 0) {
					getCengine = 1;
				}
				else
				// custom install location
				if (strcmp(argv[i], "--install-loc") == 0) {
					if (argc > i+1) {
						installLocation = argv[i+1];
						i++;
					}
					else {
						printf("avdl error: --install-loc expects a path\n");
						return -1;
					}
				}
				else
				// dynamic install location
				if (strcmp(argv[i], "--install-loc-dynamic") == 0) {
					installLocationDynamic = 1;
				}
				else
				// custom save location
				if (strcmp(argv[i], "--save-loc") == 0) {
					if (argc > i+1) {
						saveLocation = argv[i+1];
						i++;
					}
					else {
						printf("avdl error: %s expects a path\n", argv[i]);
						return -1;
					}
				}
				else
				if (strcmp(argv[i], "--game-version") == 0) {
					if (argc > i+1) {
						gameVersion = argv[i+1];

						// confirm format, only digits and '.' allowed
						for (int j = 0; j < strlen(gameVersion); j++) {
							if ((gameVersion[j] >= '0' && gameVersion[j] <= '9')
							||  gameVersion[j] == '.') {
								continue;
							}

							printf("avdl error: '%s' argument can only contain digits and '.'\n", argv[i]);
							return -1;
						}

						i++;
					}
					else {
						printf("avdl error: '%s' expects a version string\n", argv[i]);
						return -1;
					}
				}
				else
				if (strcmp(argv[i], "--game-version-code") == 0) {
					if (argc > i+1) {
						gameVersionCode = argv[i+1];
						i++;
					}
					else {
						printf("avdl error: '%s' expects a version code string\n", argv[i]);
						return -1;
					}
				}
				else
				if (strcmp(argv[i], "--game-package-name") == 0) {
					if (argc > i+1) {
						gamePackageName = argv[i+1];
						i++;
					}
					else {
						printf("avdl error: '%s' expects a package name string\n", argv[i]);
						return -1;
					}
				}
				else
				if (strcmp(argv[i], "--game-icon") == 0) {
					if (argc > i+1) {
						gameIconFlat = argv[i+1];
						i++;
					}
					else {
						printf("avdl error: '%s' expects an icon string\n", argv[i]);
						return -1;
					}
				}
				else
				if (strcmp(argv[i], "--game-icon-foreground") == 0) {
					if (argc > i+1) {
						gameIconForeground = argv[i+1];
						i++;
					}
					else {
						printf("avdl error: '%s' expects an icon string\n", argv[i]);
						return -1;
					}
				}
				else
				if (strcmp(argv[i], "--game-icon-background") == 0) {
					if (argc > i+1) {
						gameIconBackground = argv[i+1];
						i++;
					}
					else {
						printf("avdl error: '%s' expects an icon string\n", argv[i]);
						return -1;
					}
				}
				else
				if (strcmp(argv[i], "--game-revision") == 0) {
					if (argc > i+1) {
						gameRevision = argv[i+1];

						// confirm format, only digits allowed
						for (int j = 0; j < strlen(gameRevision); j++) {
							if (gameRevision[j] >= '0'
							&&  gameRevision[j] <= '9') {
								continue;
							}

							printf("avdl error: '%s' argument can only contain digits\n", argv[i]);
							return -1;
						}

						i++;

					}
					else {
						printf("avdl error: %s expects a revision string\n", argv[i]);
						return -1;
					}
				}
				else
				if (strcmp(argv[i], "--game-name") == 0) {
					if (argc > i+1) {
						gameName = argv[i+1];

						// confirm format, only digits and '.' allowed
						for (int j = 0; j < strlen(gameName); j++) {
							if ((gameName[j] >= '0' && gameName[j] <= '9')
							||  (gameName[j] >= 'a' && gameName[j] <= 'z')
							||  (gameName[j] >= 'A' && gameName[j] <= 'Z')
							||   gameName[j] == '_') {
								continue;
							}

							printf("avdl error: '%s' argument can only contain digits, a-z, A-Z and '_'\n",
								argv[i]
							);
							return -1;
						}

						i++;
					}
					else {
						printf("avdl error: '%s' expects a game name string\n", argv[i]);
						return -1;
					}
				}
				else
				if (strcmp(argv[i], "--steam") == 0) {
					avdlSteamMode = 1;
				}
				else
				if (strcmp(argv[i], "--standalone") == 0) {
					avdlStandalone = 1;
				}
				// unknown double dash argument
				else {
					printf("avdl error: cannot understand double dash argument '%s'\n", argv[i]);
					return -1;
				}

			}
			else
			/* phase arguments
			 * -t: translate only
			 * -c: skip linking
			 */
			if (strcmp(argv[i], "-t") == 0) {
				compile = 0;
				link = 0;
			}
			else
			if (strcmp(argv[i], "-c") == 0) {
				link = 0;
			}
			else
			// output filename
			if (strcmp(argv[i], "-o") == 0) {
				if (argc > i+1) {
					outname = argv[i+1];
					i++;
				}
				else {
					printf("avdl error: name is expected after `-o`\n");
					return -1;
				}
			}
			else
			// add include path
			if (strcmp(argv[i], "-I") == 0) {
				if (argc > i+1) {
					includePath = argv[i+1];
					i++;
				}
				else {
					printf("avdl error: include path is expected after `-I`\n");
					return -1;
				}
			}
			else
			// add extra include paths
			if (strcmp(argv[i], "-i") == 0) {
				if (argc > i+1) {
					if (totalIncludeDirectories >= 10) {
						printf("avdl error: maximum 10 include directories allowed\n");
						return -1;
					}
					additionalIncludeDirectory[totalIncludeDirectories] = argv[i+1];
					totalIncludeDirectories++;
					i++;
				}
				else {
					printf("avdl error: include path is expected after `-i`\n");
					return -1;
				}
			}
			else
			// add library path
			if (strcmp(argv[i], "-L") == 0) {
				if (argc > i+1) {
					if (totalLibDirectories >= 10) {
						printf("avdl error: maximum 10 library directories allowed\n");
						return -1;
					}
					additionalLibDirectory[totalLibDirectories] = argv[i+1];
					totalLibDirectories++;
					i++;
				}
				else {
					printf("avdl error: library path is expected after `-L`\n");
					return -1;
				}
			}
			else
			// quiet mode
			if (strcmp(argv[i], "-q") == 0) {
				avdlQuietMode = 1;
			}
			// unknown single dash argument
			else {
				printf("avdl error: cannot understand dash argument '%s'\n", argv[i]);
				return -1;
			}
		}
		// non-dash argument - input file?
		else {
			// input file
			if (input_file_total < MAX_INPUT_FILES) {
				strncpy(filename[input_file_total], argv[i], 100);
				filename[input_file_total][99] = '\0';
				input_file_total++;
			}
			// error argument
			else {
				printf("avdl error: '%s': Only %d input files can be provided at a time\n", argv[i], MAX_INPUT_FILES);
				return -1;
			}
		}
	}

	// grab a copy of the cengine
	if (getCengine) {
		dir_create("cengine");
		char tempBuffer[DD_BUFFER_SIZE];
		for (int i = 0; i < cengine_headers_total; i++) {
			strcpy(buffer, avdl_getProjectLocation());
			strcat(buffer, "/include/");
			strcat(buffer, cengine_headers[i]);
			strcpy(tempBuffer, "cengine/");
			strcat(tempBuffer, cengine_headers[i]);
			file_copy(buffer, tempBuffer, 0);
		}
		for (int i = 0; i < cengine_files_total; i++) {
			strcpy(buffer, avdl_getProjectLocation());
			strcat(buffer, "/share/avdl/cengine/");
			if (avdlSteamMode && cengine_files[i].steam) {
				strcat(buffer, cengine_files[i].steam);
			}
			else
			if (cengine_files[i].main) {
				strcat(buffer, cengine_files[i].main);
			}
			else {
				continue;
			}
			strcpy(tempBuffer, "cengine/");
			if (avdlSteamMode && cengine_files[i].steam) {
				strcat(tempBuffer, cengine_files[i].steam);
			}
			else
			if (cengine_files[i].main) {
				strcat(tempBuffer, cengine_files[i].main);
			}
			else {
				continue;
			}
			file_copy(buffer, tempBuffer, 0);
		}
		return 0;
	}

	// make sure the minimum to parse exists
	if (input_file_total <= 0) {
		printf("avdl error: No filename given\n");
		return -1;
	}

	if (outname && !link && input_file_total > 1) {
		printf("avdl error: cannot supply `-o` with `-c` and multiple input files\n");
		return -1;
	}

	if (translate) {
		//printf("~~~ translation phase ~~~\n");
	}
	/*
	 * translate all given `avdl` files to `.c` files
	 */
	for (int i = 0; i < input_file_total && translate; i++) {

		// check if file is meant to be compiled, or is already compiled
		if (strcmp(filename[i] +strlen(filename[i]) -3, ".dd") != 0) {
			continue;
		}

		included_files_num = 0;

		// initialise the parent node

		game_node = ast_create(AST_GAME);
		if (semanticAnalyser_convertToAst(game_node, filename[i]) != 0) {
			return -1;
		}

		/*
		 * if only transpiling, check output file
		 */
		//printf("transpiling: %s", filename[i]);
		// parse resulting ast tree to a file
		if (!compile && !link && outname) {
			strcpy(buffer, outname);
		}
		else {
			// given an outname, compile the .c file in the same directory
			if (outname && !link) {
				strcpy(buffer, outname);
				buffer[strlen(buffer) -1] = 'c';
				strcpy(filename[i], buffer);
			}
			// by default, put .c file in the same directory as source one
			else {
				filename[i][strlen(filename[i]) -2] = 'c';
				filename[i][strlen(filename[i]) -1] = '\0';
				strcpy(buffer, filename[i]);
			}
		}
		//printf(" to %s\n", buffer);
		if (transpile_cglut(buffer, game_node) != 0) {
			printf("avdl: transpilation failed to file: %s\n", buffer);
			return -1;
		}

		// print debug data
		/*
		if (show_ast) {
			ast_print(game_node);
		}
		*/

		if (show_struct_table) {
			struct_table_print();
		}
	}

	if (compile) {
		//printf("~~~ compilation phase ~~~\n");
	}
	/*
	 * compile all given `.c` files to `.o` files
	 */
	for (int i = 0; i < input_file_total && compile; i++) {

		// check if file is meant to be compiled, or is already compiled
		if (strcmp(filename[i] +strlen(filename[i]) -2, ".c") != 0) {
			continue;
		}

		//printf("compiling: %s\n", filename[i]);

		/*
		 * on android, object files are the same as `.c` source files
		 * for now
		 */
		if (avdl_platform_get() == AVDL_PLATFORM_ANDROID) {
			strcpy(buffer, filename[i]);
			filename[i][strlen(filename[i]) -1] = 'o';
			file_copy(buffer, filename[i], 0);
			continue;
		}

		// compile
		strcpy(buffer, "gcc -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DAVDL_GAME_VERSION=\"\\\"");
		strcat(buffer, gameVersion);
		strcat(buffer, "\\\"\" -DAVDL_GAME_REVISION=\"\\\"");
		strcat(buffer, gameRevision);
		strcat(buffer, "\\\"\" -c -w ");
		strcat(buffer, filename[i]);
		strcat(buffer, " -o ");
		if (outname && !link) {
			strcat(buffer, outname);
		}
		else {
			filename[i][strlen(filename[i]) -1] = 'o';
			strcat(buffer, filename[i]);
		}

		//strcat(buffer, " -O3 -lGL -lGLEW -lavdl-cengine -lm -w -lSDL2 -lSDL2_mixer");
		strcat(buffer, " -O3 -w");
		for (int i = 0; i < totalIncludeDirectories; i++) {
			strcat(buffer, " -I ");
			strcat(buffer, additionalIncludeDirectory[i]);
		}
		if (includePath) {
			strcat(buffer, " -I ");
			strcat(buffer, includePath);
		}
		//printf("command: %s\n", buffer);
		if (system(buffer)) {
			printf("avdl: error compiling file: %s\n", filename[i]);
			exit(-1);
		}
	}

	if (1) {
		//printf("~~~ asset phase ~~~\n");
	}
	// compile all given asset files
	for (int i = 0; i < input_file_total && compile; i++) {

		// check if file is asset
		if (strcmp(filename[i] +strlen(filename[i]) -4, ".ply") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".bmp") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".png") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".wav") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".ogg") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".mp3") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -5, ".opus") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -5, ".json") != 0) {
			continue;
		}

		// on android, put assets in a specific directory
		if (avdl_platform_get() == AVDL_PLATFORM_ANDROID) {
			char *out;
			if (outname) {
				out = outname;
			}
			else {
				out = "android";
			}

			if (create_android_directory(out) < 0) {
				printf("avdl: error while compiling asset '%s'\n", filename[i]);
				return -1;
			}

			char *assetDir;

			if (strcmp(filename[i] +strlen(filename[i]) -4, ".ply") == 0
			||  strcmp(filename[i] +strlen(filename[i]) -4, ".ogg") == 0
			||  strcmp(filename[i] +strlen(filename[i]) -5, ".opus") == 0
			||  strcmp(filename[i] +strlen(filename[i]) -4, ".wav") == 0) {
				assetDir = "raw";
			}
			else
			if (strcmp(filename[i] +strlen(filename[i]) -4, ".bmp") == 0
			||  strcmp(filename[i] +strlen(filename[i]) -4, ".png") == 0) {
				assetDir = "drawable";
			}
			else {
				assetDir = "raw";
			}

			strcpy(buffer, out);
			strcat(buffer, "/app/src/main/res/");
			strcat(buffer, assetDir);
			dir_create(buffer);
			strcat(buffer, "/");

			char *raw = filename[i];
			char *temp;
			while ((temp = strstr(raw, "/"))) {
				raw = temp+1;
			}
			strcat(buffer, raw);
		}
		else {
			if (outname) {
				strcpy(buffer, outname);
				strcat(buffer, "/assets/");

				if (!is_dir(buffer)) {
					dir_create(buffer);
				}

				int character = 0;
				int lastSlash = -1;
				int lastDot = -1;
				while (filename[i][character] != '\0') {
					if (filename[i][character] == '/') {
						lastSlash = character;
					}

					if (filename[i][character] == '.') {
						lastDot = character;
					}

					character++;
				}

				// temporarily keep file endings the same
				lastDot = -1;

				// has slash ("dir/*")
				if (lastSlash >= 0) {
					lastSlash++;

					// has file extention ("dir/file.ext")
					if (lastDot >= 0) {
						int length = lastDot -lastSlash;
						strncat(buffer, filename[i] +lastSlash, length);
					}
					// no file extention ("dir/file")
					else {
						int length = strlen(filename[i] +lastSlash);
						strncat(buffer, filename[i] +lastSlash, length);
					}
				}
				// no slash ("*")
				else {
					// has file extention ("file.ext")
					if (lastDot >= 0) {
						int length = lastDot;
						strncat(buffer, filename[i], length);
					}
					// no file extention ("file")
					else {
						strcat(buffer, filename[i]);
					}
				}
				//strcat(buffer, ".asset");
			}
			else {
				strcpy(buffer, filename[i]);
				//buffer[strlen(buffer) -4] = '\0';
				//strcat(buffer, ".asset");
			}
		}

		//printf("compiling asset: %s to %s\n", filename[i], buffer);

		file_copy(filename[i], buffer, 0);
	}

	// compile the final executable
	if (link) {
		//printf("~~~ link phase ~~~\n");

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
		else {

			char *outdir;
			if (outname) {
				outdir = outname;
			}
			else {
				outdir = "game";
			}

			/*
			 * if not available, compile `cengine` and cache it
			 */
			strcpy(buffer, outdir);
			strcat(buffer, "_cache");
			if (!is_dir(buffer)) {

				if (!avdlQuietMode) {
					printf("avdl: compiling cengine\n");
				}
				dir_create(buffer);

				char compile_command[DD_BUFFER_SIZE];
				for (int i = 0; i < cengine_files_total; i++) {
					if (avdlSteamMode && cengine_files[i].steam) {
						strcpy(compile_command, "g++ -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DPKG_LOCATION=\\\"");
					}
					else
					if (cengine_files[i].main) {
						strcpy(compile_command, "gcc -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DPKG_LOCATION=\\\"");
					}
					else {
						continue;
					}
					strcat(compile_command, installLocation);
					strcat(compile_command, "\\\" ");
					strcat(compile_command, avdl_getProjectLocation());
					strcat(compile_command, "/share/avdl/cengine/");
					if (avdlSteamMode && cengine_files[i].steam) {
						strcat(compile_command, cengine_files[i].steam);
					}
					else {
						strcat(compile_command, cengine_files[i].main);
					}
					if (avdlSteamMode) {
						strcat(compile_command, " -DAVDL_STEAM ");
					}
					strcat(compile_command, " -o ");
					strcat(compile_command, buffer);
					strcat(compile_command, "/");
					if (avdlSteamMode && cengine_files[i].steam) {
						strcat(compile_command, cengine_files[i].steam);
						compile_command[strlen(compile_command)-3] = 'o';
						compile_command[strlen(compile_command)-2] = '\0';
					}
					else {
						strcat(compile_command, cengine_files[i].main);
						compile_command[strlen(compile_command)-1] = 'o';
					}
					strcat(compile_command, " -I");
					strcat(compile_command, cengine_path);
					strcat(compile_command, "/include");
					for (int i = 0; i < totalIncludeDirectories; i++) {
						strcat(compile_command, " -I ");
						strcat(compile_command, additionalIncludeDirectory[i]);
					}
					//printf("cengine compile command: %s\n", compile_command);
					if (system(compile_command) != 0) {
						printf("error compiling cengine\n");
						exit(-1);
					}
					if (!avdlQuietMode) {
						printf("avdl: %d%%\n", (int)((float) (i+1)/cengine_files_total *100));
					}
				}
				if (!avdlQuietMode) {
					printf("avdl: compiling cengine done\n");
				}
			}

			// prepare link command
			if (avdlSteamMode) {
				strcpy(buffer, "g++ -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU ");
			}
			else {
				strcpy(buffer, "gcc -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU ");
			}

			// add game files to link
			for (int i = 0; i < input_file_total; i++) {
				strcat(buffer, filename[i]);
				strcat(buffer, " ");
			}

			// add cengine files to link
			char tempDir[DD_BUFFER_SIZE];
			strcpy(tempDir, outdir);
			strcat(tempDir, "_cache");
			for (int i = 0; i < cengine_files_total; i++) {
				if (avdlSteamMode && cengine_files[i].steam) {
				}
				else
				if (cengine_files[i].main) {
				}
				else {
					continue;
				}
				strcat(buffer, tempDir);
				strcat(buffer, "/");
				if (avdlSteamMode && cengine_files[i].steam) {
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

			// make sure output directory exists
			if (!is_dir(outdir)) {
				dir_create(outdir);
			}

			// output file
			strcat(buffer, "-o ");
			strcat(buffer, outdir);
			strcat(buffer, "/");
			strcat(buffer, gameName);

			for (int i = 0; i < totalLibDirectories; i++) {
				strcat(buffer, " -L ");
				strcat(buffer, additionalLibDirectory[i]);
			}

			if (avdlStandalone) {
				strcat(buffer, " -O3 -lm -l:libogg.so.0 -l:libopus.so.0 -l:libopusfile.so.0 -l:libpng16.so.16 -w -l:libSDL2-2.0.so.0 -l:libSDL2_mixer-2.0.so.0 -lpthread -lGL -l:libGLEW.so.2.2");
			}
			else {
				strcat(buffer, " -O3 -lm -logg -lopus -lopusfile -lpng -w -lSDL2 -lSDL2_mixer -lpthread -lGL -lGLEW");
			}

			if (avdlSteamMode) {
				strcat(buffer, " -lsteam_api ");
			}
			printf("link command: %s\n", buffer);
			if (system(buffer)) {
				printf("avdl: error linking files\n");
				exit(-1);
			}
		}
	}

	avdl_cleanProjectLocation();

	// success!
	return 0;
}

int create_android_directory(const char *androidDirName) {
	#if AVDL_IS_OS(AVDL_OS_WINDOWS)
	#else
	int isDir = is_dir(androidDirName);
	if (isDir == 0) {
		dir_create(androidDirName);
		dir_copy_recursive(0, PKG_LOCATION "/share/avdl/android", 0, androidDirName);
	}
	else
	if (isDir < 0) {
		printf("avdl error: file '%s' not a directory\n", androidDirName);
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
	strcpy(buffer, dirname);
	strcat(buffer, filename);

	// dst file full path
	char buffer2[1024];
	strcpy(buffer2, cache_dir);
	strcat(buffer2, filename);
	strcat(buffer2, ".c");

	// check file type
	struct stat statbuf;
	if (stat(buffer, &statbuf) != 0) {
		printf("avdl error: Unable to stat file '%s': %s\n", buffer, strerror(errno));
		return -1;
	}

	// is directory - skip - maybe recursive compilation at some point?
	if (Avdl_FileOp_IsDirStat(statbuf)) {
		printf("avdl skipping directory: %s\n", buffer);
		return 0;
	}
	else
	// is regular file - do nothing
	if (Avdl_FileOp_IsRegStat(statbuf)) {
	}
	// not supporting other file types - skip
	else {
		printf("avdl error: Unsupported file type '%s' - skip\n", buffer);
		return 0;
	}

	// skip files already transpiled (check last modified)
	if ( Avdl_FileOp_DoesFileExist(buffer2) ) {
		struct stat statbuf2;
		if (stat(buffer2, &statbuf2) != 0) {
			printf("avdl error: Unable to stat file '%s': %s\n", buffer2, strerror(errno));
			return -1;
		}

		// transpiled file is same or newer (?) - skip transpilation
		if (difftime(statbuf2.st_mtime, statbuf.st_mtime) >= 0) {
			//printf("avdl src file not modified, skipping transpilation of '%s' -> '%s'\n", buffer, buffer2);
			return 0;
		}

	}
	//printf("transpiling %s\n", buffer);

	included_files_num = 0;

	// initialise the parent node
	game_node = ast_create(AST_GAME);
	if (semanticAnalyser_convertToAst(game_node, buffer) != 0) {
		printf("avdl failed to do semantic analysis\n");
		return -1;
	}

	// write results to destination file
	if (transpile_cglut(buffer2, game_node) != 0) {
		printf("avdl: transpilation failed: %s -> %s\n", buffer, buffer2);
		return -1;
	}
	printf("avdl: transpiling - " YEL "%d%%" RESET "\r", (int)((float) (fileIndex)/filesTotal *100));

	return 0;
}

int avdl_transpile(struct AvdlSettings *avdl_settings) {

	// TODO: Handle this
	includePath = "include/";

	printf("avdl: transpiling - " RED "0%%" RESET "\r");
	fflush(stdout);

	Avdl_FileOp_ForFileInDirectory(avdl_settings->src_dir, transpile_file);

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
	strcpy(buffer, dirname);
	strcat(buffer, filename);

	// dst file full path
	char buffer2[1024];
	strcpy(buffer2, buffer);
	strcat(buffer2, ".o");

	// skip non-regular files (like directories)
	struct stat statbuf;
	if ( stat(buffer, &statbuf) != 0 ) {
		printf("avdl error: Unable to stat file '%s': %s\n", buffer, strerror(errno));
		return -1;
	}

	// is regular file - do nothing
	if (Avdl_FileOp_IsRegStat(statbuf)) {
	}
	// not supporting other file types - skip
	else {
		//printf("avdl error: Unsupported file type '%s' - skip\n", dir->d_name);
		return 0;
	}

	// skip non-`.c` files
	if (strcmp(buffer +strlen(buffer) -2, ".c") != 0) {
		return 0;
	}

	// skip files already compiled (check last modified)
	if ( Avdl_FileOp_DoesFileExist(buffer2) ) {
		struct stat statbuf2;
		if (stat(buffer2, &statbuf2) != 0) {
			printf("avdl error: Unable to stat file '%s': %s\n", buffer2, strerror(errno));
			return -1;
		}

		// compiled file is same or newer (?) - skip compilation
		if (difftime(statbuf2.st_mtime, statbuf.st_mtime) >= 0) {
			//printf("avdl src file not modified, skipping compilation of '%s'\n", dir->d_name);
			return 0;
		}
	}

	//printf("compiling %s\n", dir->d_name);

	char buffer3[1024];
	// compile
	strcpy(buffer3, "gcc -O3 -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DAVDL_GAME_VERSION=\"\\\"");
	//strcat(buffer, gameVersion);
	strcat(buffer3, "0.0.0");
	strcat(buffer3, "\\\"\" -DAVDL_GAME_REVISION=\"\\\"");
	//strcat(buffer, gameRevision);
	strcat(buffer3, "0");
	strcat(buffer3, "\\\"\" -c -w ");
	// cengine headers
	strcat(buffer3, " -I ");
	strcat(buffer3, cengine_path);
	strcat(buffer3, "/include ");
	//strcat(buffer, filename[i]);
	strcat(buffer3, buffer);
	strcat(buffer3, " -o ");
	strcat(buffer3, buffer2);
	for (int i = 0; i < totalIncludeDirectories; i++) {
		strcat(buffer3, " -I ");
		strcat(buffer3, additionalIncludeDirectory[i]);
	}
	strcat(buffer3, " -I include ");
//	if (includePath) {
//		strcat(buffer, " -I ");
//		strcat(buffer, includePath);
//	}
	printf("avdl compile command: %s\n", buffer3);
	if (system(buffer3)) {
		printf("avdl: error compiling file: %s\n", buffer3);
		return -1;
	}

	printf("avdl: compiling - " YEL "%d%%" RESET "\r", (int)((float) (fileIndex)/filesTotal *100));
	fflush(stdout);

	return 0;
}

int avdl_compile(struct AvdlSettings *avdl_settings) {

	printf("avdl: compiling - " RED "0%%" RESET "\r");
	fflush(stdout);

	Avdl_FileOp_ForFileInDirectory(cache_dir, compile_file);

	printf("avdl: compiling - " GRN "100%%" RESET "\n");
	fflush(stdout);

	return 0;
}

int avdl_compile_cengine(struct AvdlSettings *avdl_settings) {

	/*
	 * if not available, compile `cengine` and cache it
	 */
	char *outdir = ".avdl_cache/";
	strcpy(buffer, outdir);
	strcat(buffer, "cengine/");
	if (!is_dir(buffer)) {
		dir_create(buffer);
	}

	printf("avdl: compiling cengine - " RED "0%%" RESET "\r");
	fflush(stdout);
	char compile_command[DD_BUFFER_SIZE];
	for (int i = 0; i < cengine_files_total; i++) {

		strcpy(compile_command, "gcc -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DPKG_LOCATION=\\\"");
//		if (avdlSteamMode && cengine_files[i].steam) {
//			printf("cengine steam file: %s\n", cengine_files[i].steam);
//			strcpy(compile_command, "g++ -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DPKG_LOCATION=\\\"");
//		}
//		else
		if (cengine_files[i].main) {
			//printf("cengine file: %s\n", cengine_files[i].main);
			//strcpy(compile_command, "gcc -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU -DPKG_LOCATION=\\\"");
			strcpy(compile_command, "gcc -w -c -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU ");
		}
		// no file available for current settings
		else {
			continue;
		}

//		strcat(compile_command, installLocation);
//		strcat(compile_command, "\\\" ");

		// include the source file
		strcat(compile_command, cengine_path);
//		if (avdlSteamMode && cengine_files[i].steam) {
//			strcat(compile_command, cengine_files[i].steam);
//		}
//		else
		if (cengine_files[i].main) {
			strcat(compile_command, cengine_files[i].main);
		}
		else {
			continue;
		}
//
//		if (avdlSteamMode) {
//			strcat(compile_command, " -DAVDL_STEAM ");
//		}
		strcat(compile_command, " -o ");
		//strcat(compile_command, buffer);
		//strcat(compile_command, "/");
//		if (avdlSteamMode && cengine_files[i].steam) {
//			strcat(compile_command, cengine_files[i].steam);
//			compile_command[strlen(compile_command)-3] = 'o';
//			compile_command[strlen(compile_command)-2] = '\0';
//		}
//		else {
			strcpy(buffer, outdir);
			strcat(buffer, "cengine/");
			strcat(buffer, cengine_files[i].main);
			//strcat(compile_command, cengine_files[i].main);
			buffer[strlen(buffer)-1] = 'o';
			//compile_command[strlen(compile_command)-1] = 'o';
//		}
		strcat(compile_command, buffer);

		// cengine headers
		strcat(compile_command, " -I");
		strcat(compile_command, cengine_path);
		strcat(compile_command, "/include");

		// should cengine allow the inclusion of extra directories?
//		for (int i = 0; i < totalIncludeDirectories; i++) {
//			strcat(compile_command, " -I ");
//			strcat(compile_command, additionalIncludeDirectory[i]);
//		}
//
		// skip files already compiled
		if ( Avdl_FileOp_DoesFileExist(buffer) ) {
			//printf("skipping: %s\n", buffer);
			printf("avdl: compiling cengine - " YEL "%d%%" RESET "\r", (int)((float) (i+1)/cengine_files_total *100));
			fflush(stdout);
			continue;
		}

		printf("cengine compile command: %s\n", compile_command);
		if (system(compile_command) != 0) {
			printf("error compiling cengine\n");
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

	return 0;
}

static int create_executable_file(const char *filename, const char *content) {

	// make a `.sh` file to link executable with dependencies
	int fd = open(filename, O_RDWR | O_CREAT, 0777);
	if (fd == -1) {
		printf("avdl error: Unable to open '%s': %s\n", filename, strerror(errno));
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

	char buffer2[1024];
	strcpy(buffer2, dirname);
	strcat(buffer2, filename);

	// skip non-regular files (like directories)
	struct stat statbuf;
	if (stat(buffer2, &statbuf) != 0) {
		printf("avdl error: Unable to stat file '%s': %s\n", buffer2, strerror(errno));
		return -1;
	}

	// is regular file - add to link command
	if (Avdl_FileOp_IsRegStat(statbuf)) {
		strcat(buffer, ".avdl_cache/");
		strcat(buffer, filename);
		strcat(buffer, ".c.o ");
	}
	// not supporting other file types - skip
	else {
		//printf("avdl error: Unsupported file type '%s' - skip\n", dir->d_name);
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
////	if (avdlSteamMode) {
////		strcpy(buffer, "g++ -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU ");
////	}
////	else {
		strcpy(buffer, "gcc -DDD_PLATFORM_NATIVE -DGLEW_NO_GLU ");
////	}

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
////		if (avdlSteamMode && cengine_files[i].steam) {
////		}
////		else
		if (cengine_files[i].main) {
		}
		else {
			continue;
		}
		strcat(buffer, tempDir);
		strcat(buffer, "/");
////		if (avdlSteamMode && cengine_files[i].steam) {
////			strcat(buffer, cengine_files[i].steam);
////			buffer[strlen(buffer)-3] = 'o';
////			buffer[strlen(buffer)-2] = '\0';
////		}
////		else {
			strcat(buffer, cengine_files[i].main);
			buffer[strlen(buffer)-1] = 'o';
////		}
		strcat(buffer, " ");
	}

	// output file
	strcat(buffer, "-o ");
	strcat(buffer, outdir);
	strcat(buffer, "/");
	//strcat(buffer, gameName);
	strcat(buffer, "avdl_game");

////	for (int i = 0; i < totalLibDirectories; i++) {
////		strcat(buffer, " -L ");
////		strcat(buffer, additionalLibDirectory[i]);
////	}

////	if (avdlStandalone) {
////		strcat(buffer, " -O3 -lm -l:libogg.so.0 -l:libopus.so.0 -l:libopusfile.so.0 -l:libpng16.so.16 -w -l:libSDL2-2.0.so.0 -l:libSDL2_mixer-2.0.so.0 -lpthread -lGL -l:libGLEW.so.2.2");
////	}
////	else {
		strcat(buffer, " -O3 -lm -logg -lopus -lopusfile -lpng -w -lSDL2 -lSDL2_mixer -lpthread -lGL -lGLEW");
////	}

////	if (avdlSteamMode) {
////		strcat(buffer, " -lsteam_api ");
////	}
	//printf("link command: %s\n", buffer);
	if (system(buffer)) {
		printf("avdl: error linking files\n");
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

	create_executable_file("./avdl_build/avdl_game.sh",
			"LD_LIBRARY_PATH=./dependencies/ ./bin/avdl_game");
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

	// src file
	strcpy(buffer, dirname);
	strcat(buffer, filename);

	char *outdir = "avdl_build/assets/";

	// dst file
	char buffer2[1024];
	strcpy(buffer2, outdir);
	strcat(buffer2, filename);

	// skip non-regular files (like directories)
	struct stat statbuf;
	if (stat(buffer, &statbuf) != 0) {
		printf("avdl error: Unable to stat file '%s': %s\n", buffer, strerror(errno));
		return -1;
	}

	// is directory - skip - maybe recursive compilation at some point?
	if (Avdl_FileOp_IsDirStat(statbuf)) {
		//printf("avdl skipping directory: %s\n", dir->d_name);
		return 0;
	}
	else
	// is regular file - do nothing
	if (Avdl_FileOp_IsRegStat(statbuf)) {
	}
	// not supporting other file types - skip
	else {
		//printf("avdl error: Unsupported file type '%s' - skip\n", dir->d_name);
		return 0;
	}

	// skip files already transpiled (check last modified)
	if ( Avdl_FileOp_DoesFileExist(buffer2) ) {
		struct stat statbuf2;
		if (stat(buffer2, &statbuf2) != 0) {
			printf("avdl error: Unable to stat file '%s': %s\n", buffer2, strerror(errno));
			return -1;
		}

		// transpiled file is same or newer (?) - skip transpilation
		if (difftime(statbuf2.st_mtime, statbuf.st_mtime) >= 0) {
			//printf("avdl asset file not modified, skipping handling of '%s'\n", dir->d_name);
			return 0;
		}
	}
	//printf("handling %s\n", dir->d_name);

	/*
	 * Currently assets are just copy-pasted,
	 * however on a future version there will be more fine
	 * control of editing files to supported formats
	 * and throwing errors on unsupported formats.
	 */
	file_copy(buffer, buffer2, 0);

	printf("avdl: assets - " YEL "%d%%" RESET "\r", (int)((float) (fileIndex)/filesTotal *100));
	fflush(stdout);
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
