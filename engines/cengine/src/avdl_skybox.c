#include "avdl_skybox.h"
#include "avdl_graphics.h"
#include "avdl_assetManager.h"
#include "avdl_shaders.h"

extern GLuint skyboxProgram;

void avdl_skybox_create(struct avdl_skybox *o) {
	o->assetName = 0;
	o->assetType = 0;
	o->graphics_contextid = -1;
	o->clean = avdl_skybox_clean;
	o->set = avdl_skybox_set;
	o->bind = avdl_skybox_bind;
	o->unbind = avdl_skybox_unbind;
	o->draw = avdl_skybox_draw;

	// mesh
	avdl_mesh_create(&o->mesh);
	avdl_mesh_set_primitive(&o->mesh, AVDL_PRIMITIVE_BOX_FLIP);
	avdl_mesh_set_colour(&o->mesh, 0, 0, 0);

	for (int i = 0; i < 6; i++) {
		dd_image_create(&o->img[i]);
	}

	// program shader
	avdl_program_create(&o->program);
	o->program.openglContext = avdl_graphics_getContextId();
	o->program.program = skyboxProgram;
}

void avdl_skybox_clean(struct avdl_skybox *o) {
	avdl_program_clean(&o->program);
	avdl_mesh_clean(&o->mesh);

	for (int i = 0; i < 6; i++) {
		dd_image_clean(&o->img[i]);
	}
}

void avdl_skybox_set(struct avdl_skybox *o, const char *assetname[]) {

	// potentially needed if graphics context is lost
	//o->graphics_contextid = avdl_graphics_getContextId();
	//o->assetName = assetname1;
	//o->assetType = 0;

	// load textures
	for (int i = 0; i < 6; i++) {
		o->img[i].set(&o->img[i], assetname[i], 0);
	}

}

void avdl_skybox_unbind(struct avdl_skybox *o) {
	avdl_graphics_BindTextureSkybox(0);
}

void avdl_skybox_bind(struct avdl_skybox *o) {

	// skybox needs to be loaded into the gpu
	if (o->graphics_contextid == -1 || o->graphics_contextid != avdl_graphics_getContextId()) {

		// until all skybox textures are loaded, there's nothing to be done
		for (int i = 0; i < 6; i++) {
			if (!o->img[i].isLoaded(&o->img[i])) {
				return;
			}
		}

		// images loaded - move to gpu
		o->graphics_contextid = avdl_graphics_getContextId();
		void *pixels[] = {
			o->img[0].pixels,
			o->img[1].pixels,
			o->img[2].pixels,
			o->img[3].pixels,
			o->img[4].pixels,
			o->img[5].pixels,
		};
		int pixelFormat[] = {
			o->img[0].pixelFormat,
			o->img[1].pixelFormat,
			o->img[2].pixelFormat,
			o->img[3].pixelFormat,
			o->img[4].pixelFormat,
			o->img[5].pixelFormat,
		};
		int width[] = {
			o->img[0].width,
			o->img[1].width,
			o->img[2].width,
			o->img[3].width,
			o->img[4].width,
			o->img[5].width,
		};
		int height[] = {
			o->img[0].height,
			o->img[1].height,
			o->img[2].height,
			o->img[3].height,
			o->img[4].height,
			o->img[5].height,
		};
		o->tex = avdl_graphics_SkyboxToGpu(pixels, pixelFormat, width, height);
	
		for (int i = 0; i < 6; i++) {
			dd_image_cleanNonGpuData(&o->img[i]);
		}
	}

	avdl_graphics_BindTextureSkybox(o->tex);

//	// texture is valid in this opengl context, bind it
//	if (o->openglContextId == avdl_graphics_getContextId()) {
//		avdl_graphics_BindTextureIndex(o->tex, index);
//	}
//	// texture was in a previous opengl context, reload it
//	else {
//		o->tex = 0;
//		if (o->assetName) {
//			o->set(o, o->assetName, o->assetType);
//		}
//		else {
//			//dd_log("error state?");
//		}
//	}
//	#endif

}

void avdl_skybox_draw(struct avdl_skybox *o) {

	dd_matrix_push();
	dd_scalef(4, 4, 4);

	o->bind(o);
	avdl_useProgram(&o->program);
	o->mesh.draw(&o->mesh);
	avdl_useProgram(0);
	o->unbind(o);

	dd_matrix_pop();
}
