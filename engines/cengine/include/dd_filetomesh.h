#ifndef DD_FILETOMESH_H
#define DD_FILETOMESH_H

/* Needs mesh, to store data into */
#include "dd_mesh.h"

/* Files supported */
#define DD_PLY 0
#define DD_OBJ 1

/* Settings for different things */

/* Mirroring - can be piped together for multiple mirroring */
#define DD_FILETOMESH_SETTINGS_MIRROR_X 1
#define DD_FILETOMESH_SETTINGS_MIRROR_Y 2
#define DD_FILETOMESH_SETTINGS_MIRROR_Z 4

/* Vertex attributes to parse */
#define DD_FILETOMESH_SETTINGS_POSITION 8
#define DD_FILETOMESH_SETTINGS_COLOUR 16
#define DD_FILETOMESH_SETTINGS_TEX_COORD 32

/* a mesh with all the asked data loaded
 */
struct dd_loaded_mesh {
	int vcount;
	float *v;
	float *c;
	float *t;
};

/* Creates a loaded mesh from a file */
int dd_filetomesh(struct dd_loaded_mesh *m, const char *path, int settings, int file_type);

#endif /* DD_FILETOMESH_H */
