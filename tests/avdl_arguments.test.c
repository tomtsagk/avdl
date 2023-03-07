#include "avdl_arguments.h"
#include "avdl_settings.h"
#include <assert.h>
#include <string.h>

int main(int argc, char *argv[]) {

	struct AvdlSettings avdl_settings;
	AvdlSettings_Create(&avdl_settings);

	// check platform - linux
	int platLinuxArgumentsC = 2;
	char *platLinuxArgumentsV[] = {"avdl", "--linux"};
	assert(avdl_arguments_handle(&avdl_settings, platLinuxArgumentsC, platLinuxArgumentsV) == 0);
	assert(avdl_settings.target_platform == AVDL_PLATFORM_LINUX);

	// check platform - windows
	int platWindowsArgumentsC = 2;
	char *platWindowsArgumentsV[] = {"avdl", "--windows"};
	assert(avdl_arguments_handle(&avdl_settings, platWindowsArgumentsC, platWindowsArgumentsV) == 0);
	assert(avdl_settings.target_platform == AVDL_PLATFORM_WINDOWS);

	// check platform - android
	int platAndroidArgumentsC = 2;
	char *platAndroidArgumentsV[] = {"avdl", "--android"};
	assert(avdl_arguments_handle(&avdl_settings, platAndroidArgumentsC, platAndroidArgumentsV) == 0);
	assert(avdl_settings.target_platform == AVDL_PLATFORM_ANDROID);

	// check version
	int versArgumentsC = 2;
	char *versArgumentsV[] = {"avdl", "--version"};
	assert(avdl_arguments_handle(&avdl_settings, versArgumentsC, versArgumentsV) > 0);

	// pkg location
	int pkgLocArgumentsC = 2;
	char *pkgLocArgumentsV[] = {"avdl", "--get-pkg-location"};
	assert(avdl_arguments_handle(&avdl_settings, pkgLocArgumentsC, pkgLocArgumentsV) > 0);

	// set save location
	int saveLocArgumentsC = 3;
	char *saveLocArgumentsV[] = {"avdl", "--save-loc", "/some/random/location"};
	assert(avdl_arguments_handle(&avdl_settings, saveLocArgumentsC, saveLocArgumentsV) == 0);

	// save location - not enough arguments
	int saveLocErrorArgumentsC = 2;
	char *saveLocErrorArgumentsV[] = {"avdl", "--save-loc"};
	assert(avdl_arguments_handle(&avdl_settings, saveLocErrorArgumentsC, saveLocErrorArgumentsV) < 0);

	// save location - too long argument
	int saveLocError2ArgumentsC = 3;
	char *saveLocError2ArgumentsV[] = {"avdl", "--save-loc", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};
	assert(avdl_arguments_handle(&avdl_settings, saveLocError2ArgumentsC, saveLocError2ArgumentsV) < 0);

	// steam mode
	int steamArgumentsC = 2;
	char *steamArgumentsV[] = {"avdl", "--steam"};
	assert(avdl_arguments_handle(&avdl_settings, steamArgumentsC, steamArgumentsV) == 0);

	// standalone
	int standaloneArgumentsC = 2;
	char *standaloneArgumentsV[] = {"avdl", "--standalone"};
	assert(avdl_arguments_handle(&avdl_settings, standaloneArgumentsC, standaloneArgumentsV) == 0);

	// asset location
	int assetLocArgumentsC = 3;
	char *assetLocArgumentsV[] = {"avdl", "--asset-loc", "/some/random/location"};
	assert(avdl_arguments_handle(&avdl_settings, assetLocArgumentsC, assetLocArgumentsV) == 0);

	// asset location - not enough arguments
	int assetLocErrorArgumentsC = 2;
	char *assetLocErrorArgumentsV[] = {"avdl", "--asset-loc"};
	assert(avdl_arguments_handle(&avdl_settings, assetLocErrorArgumentsC, assetLocErrorArgumentsV) < 0);

	// asset location - too long argument
	int assetLocError2ArgumentsC = 3;
	char *assetLocError2ArgumentsV[] = {"avdl", "--asset-loc", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};
	assert(avdl_arguments_handle(&avdl_settings, assetLocError2ArgumentsC, assetLocError2ArgumentsV) < 0);

	// translate only
	int translateOnlyArgumentsC = 2;
	char *translateOnlyArgumentsV[] = {"avdl", "-t"};
	assert(avdl_arguments_handle(&avdl_settings, translateOnlyArgumentsC, translateOnlyArgumentsV) == 0);

	// quiet mode
	int quietModeArgumentsC = 2;
	char *quietModeArgumentsV[] = {"avdl", "-q"};
	assert(avdl_arguments_handle(&avdl_settings, quietModeArgumentsC, quietModeArgumentsV) == 0);

	// include directories
	int includeDirsArgumentsC = 3;
	char *includeDirsArgumentsV[] = {"avdl", "-i", "some/path"};
	assert(avdl_arguments_handle(&avdl_settings, includeDirsArgumentsC, includeDirsArgumentsV) == 0);

	// include directories - error
	int includeDirsErrorArgumentsC = 2;
	char *includeDirsErrorArgumentsV[] = {"avdl", "-i"};
	assert(avdl_arguments_handle(&avdl_settings, includeDirsErrorArgumentsC, includeDirsErrorArgumentsV) < 0);

	// lib directories
	int libDirsArgumentsC = 3;
	char *libDirsArgumentsV[] = {"avdl", "-L", "some/path"};
	assert(avdl_arguments_handle(&avdl_settings, libDirsArgumentsC, libDirsArgumentsV) == 0);

	// lib directories - error
	int libDirsErrorArgumentsC = 2;
	char *libDirsErrorArgumentsV[] = {"avdl", "-L"};
	assert(avdl_arguments_handle(&avdl_settings, libDirsErrorArgumentsC, libDirsErrorArgumentsV) < 0);

	// error double dash
	int errorDblDashArgumentsC = 2;
	char *errorDblDashArgumentsV[] = {"avdl", "--randomargument"};
	assert(avdl_arguments_handle(&avdl_settings, errorDblDashArgumentsC, errorDblDashArgumentsV) < 0);

	// error single dash
	int errorSglDashArgumentsC = 2;
	char *errorSglDashArgumentsV[] = {"avdl", "-randomargument"};
	assert(avdl_arguments_handle(&avdl_settings, errorSglDashArgumentsC, errorSglDashArgumentsV) < 0);

	// error non dash argument
	int errorNullDashArgumentsC = 2;
	char *errorNullDashArgumentsV[] = {"avdl", "randomargument"};
	assert(avdl_arguments_handle(&avdl_settings, errorNullDashArgumentsC, errorNullDashArgumentsV) < 0);

	return 0;
}
