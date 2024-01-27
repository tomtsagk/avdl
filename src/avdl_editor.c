#include "avdl_editor.h"
#include "avdl_settings.h"
#include "dd_game.h"
#include "dd_matrix.h"

void avdl_editor_create(struct avdl_editor *o) {
	dd_world_create(o);
	o->parent.clean = avdl_editor_clean;
	o->parent.draw = avdl_editor_draw;
	o->parent.key_input = avdl_editor_key_input;

	// default camera
	dd_vec3_create(&o->camera_position);
	dd_vec3_setf(&o->camera_position, 0, 1, 5);

	// grid
	avdl_mesh_create(&o->grid);
	avdl_mesh_set_primitive(&o->grid, AVDL_PRIMITIVE_RECTANGLE);
	avdl_mesh_set_colour(&o->grid, 0.1, 0.5, 0.1);
}

void avdl_editor_clean(struct avdl_editor *o) {
	dd_world_clean(o);
}

void dd_gameInit() {
	dd_world_set_default(avdl_editor);
	dd_setGameTitle("Avdl Editor v"PKG_VERSION);
}

void avdl_editor_key_input(struct avdl_editor *o, int key) {
	if (key == 'w') {
		dd_vec3_addf(&o->camera_position, 0, 0, -0.1);
	}
	else
	if (key == 's') {
		dd_vec3_addf(&o->camera_position, 0, 0, 0.1);
	}
	else
	if (key == 'a') {
		dd_vec3_addf(&o->camera_position, -0.1, 0, 0);
	}
	else
	if (key == 'd') {
		dd_vec3_addf(&o->camera_position, 0.1, 0, 0);
	}
}

void avdl_editor_draw(struct avdl_editor *o) {

	// default bg
	dd_clearColour(0.5, 0.1, 0.2, 1.0);

	// camera
	dd_translatef(
		-dd_vec3_getX(&o->camera_position),
		-dd_vec3_getY(&o->camera_position),
		-dd_vec3_getZ(&o->camera_position)
	);

	// grid
	dd_matrix_push();
	dd_rotatef(-90, 1, 0, 0);
	avdl_mesh_draw(&o->grid);
	dd_matrix_pop();

	// scene?
}

void avdl_OpenEditor() {
	avdl_log("open gui");

	//dd_world_set_default(avdl_editor);
	//dd_setGameTitle("Avdl Editor");
	dd_main(0, 0);
}
