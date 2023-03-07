#include "avdl_settings.h"
#include <assert.h>
#include <string.h>

int main(int argc, char *argv[]) {

	struct AvdlSettings avdl_settings;
	AvdlSettings_Create(&avdl_settings);
	assert(AvdlSettings_SetFromFile(&avdl_settings, "samples/app.avdl") == 0);
	assert(AvdlSettings_SetFromFile(&avdl_settings, "tests/errorapp.avdl") != 0);

	return 0;
}
