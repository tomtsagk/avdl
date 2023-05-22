#include "avdl_arguments.h"

#include "avdl_log.h"
#include <string.h>
#include "avdl_settings.h"

int avdl_arguments_handle(struct AvdlSettings *avdl_settings, int argc, char *argv[]) {

	// parse arguments, ignoring 0 -- this program's name
	for (int i = 1; i < argc; i++) {

		// dash argument
		if (strlen(argv[i]) > 0 && argv[i][0] == '-') {

			// double dash argument
			if (strlen(argv[i]) > 1 && argv[i][1] == '-') {

				/*
				// print abstract syntax tree -- used for debugging early on in the project
				// unclear if it's still needed
				if (strcmp(argv[i], "--print-ast") == 0) {
					//show_ast = 1;
				}
				else
				// print struct table -- used for debugging early on in the project
				// unclear if it's still needed
				if (strcmp(argv[i], "--print-struct-table") == 0) {
					//show_struct_table = 1;
				}
				else
				*/
				// compiling for windows
				if (strcmp(argv[i], "--windows") == 0) {
					avdl_settings->target_platform = AVDL_PLATFORM_WINDOWS;
				}
				else
				// compiling for linux
				if (strcmp(argv[i], "--linux") == 0) {
					avdl_settings->target_platform = AVDL_PLATFORM_LINUX;
				}
				else
				// compiling for android
				if (strcmp(argv[i], "--android") == 0) {
					avdl_settings->target_platform = AVDL_PLATFORM_ANDROID;
				}
				else
				// compiling for quest 2
				if (strcmp(argv[i], "--quest2") == 0) {
					avdl_settings->target_platform = AVDL_PLATFORM_QUEST2;
				}
				else
				// makefile generation
				if (strcmp(argv[i], "--makefile") == 0) {
					avdl_settings->makefile_mode = 1;
				}
				else
				// cmake generation
				if (strcmp(argv[i], "--cmake") == 0) {
					avdl_settings->cmake_mode = 1;
				}
				else
				// show version number
				if (strcmp(argv[i], "--version") == 0) {
					avdl_log(PKG_NAME " v%s", PKG_VERSION);
					return 1;
				}
				else
				// show pkg location
				if (strcmp(argv[i], "--get-pkg-location") == 0) {
					/*
					#if AVDL_IS_OS(AVDL_OS_WINDOWS)
					wprintf(L"%lS\n", avdl_getProjectLocation());
					#else
					printf("%s\n", avdl_getProjectLocation());
					#endif
					*/
					avdl_log("%s", avdl_settings->pkg_path);
					return 1;
				}
				else
				// custom save location
				if (strcmp(argv[i], "--save-loc") == 0) {
					if (argc > i+1) {
						if (strlen(argv[i+1]) > 1023) {
							avdl_log_error("save location is too long");
							return -1;
						}
						strcpy(avdl_settings->save_path, argv[i+1]);
						avdl_settings->save_path[1023] = '\0';
						i++;
					}
					else {
						avdl_log_error(BLU "%s" RESET " expects a path", argv[i]);
						return -1;
					}
				}
				else
				if (strcmp(argv[i], "--steam") == 0) {
					avdl_settings->steam_mode = 1;
					avdl_settings->cpp_mode = 1;
				}
				else
				if (strcmp(argv[i], "--standalone") == 0) {
					avdl_settings->standalone = 1;
				}
				else
				if (strcmp(argv[i], "--asset-loc") == 0) {
					if (argc > i+1) {
						if (strlen(argv[i+1]) > 1023) {
							avdl_log_error("asset prefix is too long");
							return -1;
						}
						strcpy(avdl_settings->asset_prefix, argv[i+1]);
						avdl_settings->asset_prefix[1023] = '\0';
						i++;
					}
					else {
						avdl_log_error(BLU "%s" RESET " expects a path", argv[i]);
						return -1;
					}
				}
				// unknown double dash argument
				else {
					avdl_log_error("cannot understand double dash argument " BLU "'%s'" RESET, argv[i]);
					return -1;
				}
			}
			else
			/* phase arguments
			 */
			if (strcmp(argv[i], "-t") == 0) {
				avdl_settings->translate_only = 1;
			}
			else
			// add include path
			if (strcmp(argv[i], "-I") == 0) {
				/*
				if (argc > i+1) {
					includePath = argv[i+1];
					i++;
				}
				else {
					avdl_log_error("include path is expected after " BLU "`-I`" RESET);
					return -1;
				}
				*/
			}
			else
			// add extra include paths
			if (strcmp(argv[i], "-i") == 0) {
				if (argc > i+1) {
					if (avdl_settings->total_include_directories >= 10) {
						avdl_log_error("maximum " BLU "10" RESET " include directories allowed with " BLU "-i" RESET);
						return -1;
					}
					avdl_settings->additional_include_directory
						[avdl_settings->total_include_directories] = argv[i+1];
					avdl_settings->total_include_directories++;
					i++;
				}
				else {
					avdl_log_error("include path is expected after " BLU "`-i`" RESET);
					return -1;
				}
			}
			else
			// add library path
			if (strcmp(argv[i], "-L") == 0) {
				if (argc > i+1) {
					if (avdl_settings->total_lib_directories >= 10) {
						avdl_log_error("maximum " BLU "10" RESET " library directories allowed with " BLU "-L" RESET);
						return -1;
					}
					avdl_settings->additional_lib_directory
						[avdl_settings->total_lib_directories] = argv[i+1];
					avdl_settings->total_lib_directories++;
					i++;
				}
				else {
					avdl_log_error("library path is expected after " BLU "`-L`" RESET);
					return -1;
				}
			}
			else
			// quiet mode
			if (strcmp(argv[i], "-q") == 0) {
				avdl_settings->quiet_mode = 1;
			}
			// unknown single dash argument
			else {
				avdl_log_error("cannot understand dash argument " BLU "'%s'" RESET, argv[i]);
				return -1;
			}
		}
		// non-dash argument - nothing?
		else {
			avdl_log_error("cannot understand argument " BLU "'%s'" RESET, argv[i]);
			return -1;
		}
	}
	return 0;
}
