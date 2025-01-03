#include "avdl_terrain.h"
#include "avdl_log.h"
#include "dd_math.h"

void avdl_terrain_create(struct avdl_terrain *o) {
	o->load = avdl_terrain_load;
	o->draw = avdl_terrain_draw;
	o->getSpot = avdl_terrain_getSpot;
	o->isOnTerrain = avdl_terrain_isOnTerrain;

	o->getWidth = avdl_terrain_getWidth;
	o->getHeight = avdl_terrain_getHeight;
	o->isLoaded = avdl_terrain_isLoaded;

	o->setScaleZ = avdl_terrain_setScaleZ;

	avdl_mesh_create(&o->mesh);
	dd_image_create(&o->img);

	o->heights = 0;
	o->width = 0;
	o->height = 0;
	o->loaded = 0;

	o->scaleZ = 1;
}

void avdl_terrain_clean(struct avdl_terrain *o) {

	if (o->heights) {
		free(o->heights);
		o->heights = 0;
	}

	dd_image_clean(&o->img);
	avdl_mesh_clean(&o->mesh);

	o->loaded = 0;
}

void avdl_terrain_load(struct avdl_terrain *o, const char *filename) {
	dd_image_load_png(&o->img, filename);
}

void avdl_terrain_draw(struct avdl_terrain *o) {
	if (o->img.isLoaded(&o->img)) {
		if (o->img.width == 0 || o->img.height == 0) {
			dd_image_clean(&o->img);
			return;
		}

		if (o->img.pixelFormat != GL_RGB) {
			dd_image_clean(&o->img);
			return;
		}

		o->loaded = 1;
		o->width = o->img.width;
		o->height = o->img.height;
		o->heights = malloc(sizeof(float) *o->img.width *o->img.height);
		for (int i = 0; i < o->img.width *o->img.height; i++) {
			o->heights[i] = o->img.pixels[i*3] *o->scaleZ;
		}

		o->mesh.vcount = ((o->img.width -1) *(o->img.height -1)) *6;
		o->mesh.v = malloc(sizeof(float) *o->mesh.vcount *3);
		o->mesh.dirtyVertices = 1;
		o->mesh.c = malloc(sizeof(float) *o->mesh.vcount *3);
		o->mesh.dirtyColours = 1;

		for (int x = 0; x < o->img.width -1; x++)
		for (int y = 0; y < o->img.height-1; y++) {

			int index = ((y *(o->img.width-1)) +x) *18;
			int pixelIndex = ((y *(o->img.width-0) *3) +(x *3));
			//int pixelIndex = (x *3);
			int pixelIndexRight = pixelIndex +3;
			int pixelIndexTop = pixelIndex +((o->img.width-0)*3);
			int pixelIndexTopRight = pixelIndexTop +3;

			// triangle 1

			// vertex 1
			o->mesh.v[index +0] = x *1;
			o->mesh.v[index +1] = o->img.pixels[pixelIndex] *o->scaleZ;
			o->mesh.v[index +2] = y *-1;
			o->mesh.c[index +0] = 0;
			o->mesh.c[index +1] = 0;
			o->mesh.c[index +2] = 0;

			// vertex 2
			o->mesh.v[index +3] = x *1 +1;
			o->mesh.v[index +4] = o->img.pixels[pixelIndexTopRight] *o->scaleZ;
			o->mesh.v[index +5] = y *-1 -1;
			o->mesh.c[index +3] = 1;
			o->mesh.c[index +4] = 0;
			o->mesh.c[index +5] = 1;

			// vertex 3
			o->mesh.v[index +6] = x *1;
			o->mesh.v[index +7] = o->img.pixels[pixelIndexTop] *o->scaleZ;
			o->mesh.v[index +8] = y *-1 -1;
			o->mesh.c[index +6] = 0;
			o->mesh.c[index +7] = 0;
			o->mesh.c[index +8] = 1;

			// triangle 2

			// vertex 1
			o->mesh.v[index +9] = x *1;
			o->mesh.v[index +10] = o->img.pixels[pixelIndex] *o->scaleZ;
			o->mesh.v[index +11] = y *-1;
			o->mesh.c[index +9] = 0;
			o->mesh.c[index +10] = 0;
			o->mesh.c[index +11] = 0;

			// vertex 2
			o->mesh.v[index +12] = x *1 +1;
			o->mesh.v[index +13] = o->img.pixels[pixelIndexRight] *o->scaleZ;
			o->mesh.v[index +14] = y *-1;
			o->mesh.c[index +12] = 1;
			o->mesh.c[index +13] = 0;
			o->mesh.c[index +14] = 0;

			// vertex 3
			o->mesh.v[index +15] = x *1 +1;
			o->mesh.v[index +16] = o->img.pixels[pixelIndexTopRight] *o->scaleZ;
			o->mesh.v[index +17] = y *-1 -1;
			o->mesh.c[index +15] = 1;
			o->mesh.c[index +16] = 0;
			o->mesh.c[index +17] = 1;

		}

		dd_image_clean(&o->img);
	}

	avdl_mesh_draw(&o->mesh);
}

float avdl_terrain_getSpot(struct avdl_terrain *o, float x, float z) {

	if (!o->loaded) {
		avdl_log("terrain not loaded yet");
		return 0;
	}

	// player's tile
	int tileX = dd_math_min(x, o->width  -2);
	int tileZ = dd_math_min(z, o->height -2);

	// tile index
	int index = ((tileZ *o->width) +tileX);
        int indexRight = index +1;
	int indexTop = index +o->width;
	int indexTopRight = index +o->width +1;

	// interpolate to find terrain's height at given position
        float factorX = x -tileX;
        float factorZ = z -tileZ;

	float h_bottom = o->heights[index] +((o->heights[indexRight] -o->heights[index]) *factorX);
	float h_top = o->heights[indexTop] +((o->heights[indexTopRight] -o->heights[indexTop]) *factorX);

	float h_final = h_bottom +((h_top -h_bottom) *factorZ);
	return h_final;
}

int avdl_terrain_isOnTerrain(struct avdl_terrain *o, float x, float z) {
	if (x < 0 || z < 0) {
		return 0;
	}

	if (x >= o->width -1 || z >= o->height -1) {
		return 0;
	}

	return 1;
}

int avdl_terrain_getWidth(struct avdl_terrain *o) {
	return o->width;
}

int avdl_terrain_getHeight(struct avdl_terrain *o) {
	return o->height;
}

int avdl_terrain_isLoaded(struct avdl_terrain *o) {
	return o->loaded;
}

int avdl_terrain_setScaleZ(struct avdl_terrain *o, float scale) {
	o->scaleZ = scale;
}
