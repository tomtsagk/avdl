#include <stdio.h>
#include <string.h>
#include <unistd.h>
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
char *saveLocation = "";
char *additionalLibDirectory = 0;

int create_android_directory(const char *androidDirName);

char *cengine_files[] = {
	"avdl_assetManager.c",
	"avdl_data.c",
	"avdl_localisation.c",
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
	"dd_math.c",
	"dd_matrix.c",
	"dd_mesh.c",
	"dd_meshColour.c",
	"dd_meshTexture.c",
	"dd_mouse.c",
	"dd_opengl.c",
	"dd_sound.c",
	"dd_string3d.c",
	"dd_vec3.c",
	"dd_vec4.c",
	"dd_world.c",
	"main.c",
};
unsigned int cengine_files_total = sizeof(cengine_files) /sizeof(char *);

// init data, parse, exit
#ifdef AVDL_UNIT_TEST
int avdl_main(int argc, char *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif

	avdl_platform_initialise();

	/* tweakable data
	 */
	int show_ast = 0;
	int show_struct_table = 0;
	char filename[MAX_INPUT_FILES][100];
	int input_file_total = 0;
	char *outname = 0;
	char *gameName = "game";
	char *gameVersion = "0.0.0";
	char *gameRevision = "0";

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
					show_ast = 1;
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
					return -1;
				}
				else
				// show pkg location
				if (strcmp(argv[i], "--get-pkg-location") == 0) {
					printf("%s\n", PKG_LOCATION);
					return -1;
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
			// add library path
			if (strcmp(argv[i], "-L") == 0) {
				if (argc > i+1) {
					additionalLibDirectory = argv[i+1];
					i++;
				}
				else {
					printf("avdl error: library path is expected after `-L`\n");
					return -1;
				}
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

		game_node = ast_create(AST_GAME, 0);
		semanticAnalyser_convertToAst(game_node, filename[i]);

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
		if (show_ast) {
			ast_print(game_node);
		}

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
		strcpy(buffer, "gcc -DDD_PLATFORM_NATIVE -DAVDL_GAME_VERSION=\"\\\"");
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

		if (additionalLibDirectory) {
			strcat(buffer, " -L ");
			strcat(buffer, additionalLibDirectory);
		}

		strcat(buffer, " -O3 -lGL -lGLU -lGLEW -lavdl-cengine -lm -w -lSDL2 -lSDL2_mixer");
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
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".wav") != 0
		&&  strcmp(filename[i] +strlen(filename[i]) -4, ".ogg") != 0
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
			||  strcmp(filename[i] +strlen(filename[i]) -4, ".wav") == 0) {
				assetDir = "raw";
			}
			else
			if (strcmp(filename[i] +strlen(filename[i]) -4, ".bmp") == 0) {
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
						int length = strlen(filename[i] +lastSlash) -lastSlash;
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
				strcat(buffer, ".asset");
			}
			else {
				strcpy(buffer, filename[i]);
				buffer[strlen(buffer) -4] = '\0';
				strcat(buffer, ".asset");
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

				printf("avdl: compiling cengine\n");
				dir_create(buffer);

				char compile_command[DD_BUFFER_SIZE];
				for (int i = 0; i < cengine_files_total; i++) {
					strcpy(compile_command, "gcc -w -c -DDD_PLATFORM_NATIVE -DPKG_LOCATION=\\\"");
					strcat(compile_command, installLocation);
					strcat(compile_command, "\\\" " PKG_LOCATION "/share/avdl/cengine/");
					strcat(compile_command, cengine_files[i]);
					strcat(compile_command, " -o ");
					strcat(compile_command, buffer);
					strcat(compile_command, "/");
					strcat(compile_command, cengine_files[i]);
					compile_command[strlen(compile_command)-1] = 'o';
					strcat(compile_command, " -I" PKG_LOCATION "/include");
					if (system(compile_command) != 0) {
						printf("error compiling cengine\n");
						exit(-1);
					}
				}
				printf("done\n");
			}

			// prepare link command
			strcpy(buffer, "gcc -DDD_PLATFORM_NATIVE ");

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
				strcat(buffer, tempDir);
				strcat(buffer, "/");
				strcat(buffer, cengine_files[i]);
				buffer[strlen(buffer)-1] = 'o';
				strcat(buffer, " ");
			}

			// output file
			strcat(buffer, "-o ");
			strcat(buffer, outdir);
			strcat(buffer, "/");
			strcat(buffer, gameName);

			if (additionalLibDirectory) {
				strcat(buffer, " -L");
				strcat(buffer, additionalLibDirectory);
			}

			strcat(buffer, " -lGLU -O3 -lm -w -lSDL2 -lSDL2_mixer -lpthread -lGL -lGLEW");
			//printf("link command: %s\n", buffer);
			if (system(buffer)) {
				printf("avdl: error linking files\n");
				exit(-1);
			}
		}
	}

	// success!
	return 0;
}

int create_android_directory(const char *androidDirName) {
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

	return 0;
}
