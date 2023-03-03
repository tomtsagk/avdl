#include "avdl_arguments.h"
#include "avdl_settings.h"
#include <assert.h>
#include <string.h>

int main(int argc, char *argv[]) {

	struct AvdlSettings avdl_settings;
	AvdlSettings_Create(&avdl_settings);

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

	// translate only
	int translateOnlyArgumentsC = 2;
	char *translateOnlyArgumentsV[] = {"avdl", "-t"};
	assert(avdl_arguments_handle(&avdl_settings, translateOnlyArgumentsC, translateOnlyArgumentsV) == 0);

	// quiet mode
	int quietModeArgumentsC = 2;
	char *quietModeArgumentsV[] = {"avdl", "-q"};
	assert(avdl_arguments_handle(&avdl_settings, quietModeArgumentsC, quietModeArgumentsV) == 0);

	// error double dash
	int errorDblDashArgumentsC = 2;
	char *errorDblDashArgumentsV[] = {"avdl", "--randomargument"};
	assert(avdl_arguments_handle(&avdl_settings, errorDblDashArgumentsC, errorDblDashArgumentsV) < 0);

	// error single dash
	int errorSglDashArgumentsC = 2;
	char *errorSglDashArgumentsV[] = {"avdl", "-randomargument"};
	assert(avdl_arguments_handle(&avdl_settings, errorSglDashArgumentsC, errorSglDashArgumentsV) < 0);

	return 0;
}
