#ifndef AVDL_PKG_H
#define AVDL_PKG_H

/*
 * Holds information about the avdl package itself,
 * like where it's installed and how to access
 * its own files.
 */

/*
 * Two types of package location:
 *
 * Dynamic: The project lives in a directory,
 * which might be moved around.
 *
 * Fixed: The project is permanently in a directory,
 * most likely because it was installed on a system.
 */
enum AVDL_PKG_LOCATION_TYPE {
	AVDL_PKG_LOCATION_TYPE_DYNAMIC,
	AVDL_PKG_LOCATION_TYPE_FIXED,
};

/*
 * Returns the path of the `cengine` directory,
 * to compile the engine for a project.
 */
const char *avdl_pkg_GetCenginePath();

#endif
