#include "dd_world.h"
#include "avdl_mesh.h"
#include "dd_vec3.h"

struct avdl_editor {
	struct dd_world parent;

	// view camera
	struct dd_vec3 camera_position;

	// default grid
	struct avdl_mesh grid;
};

void avdl_editor_create(struct avdl_editor *);
void avdl_editor_clean(struct avdl_editor *);

void avdl_editor_key_input(struct avdl_editor *, int key);
void avdl_editor_draw(struct avdl_editor *);

void avdl_OpenEditor();
