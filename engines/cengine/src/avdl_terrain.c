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

	o->setTextureIndex = avdl_terrain_setTextureIndex;

	avdl_mesh_create(&o->mesh);
	avdl_texture_create(&o->img);

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

	avdl_texture_clean(&o->img);
	avdl_mesh_clean(&o->mesh);

	o->loaded = 0;
}

void avdl_terrain_load(struct avdl_terrain *o, const char *filename) {
	//avdl_texture_load_png(&o->img, filename);
	avdl_texture_set(&o->img, filename, AVDL_IMAGETYPE_PNG);
}

void avdl_terrain_loadLocal(struct avdl_terrain *o, const char *filename) {
	avdl_texture_setLocal(&o->img, filename, AVDL_IMAGETYPE_PNG);
	//avdl_texture_load_png(&o->img, filename);
}

void avdl_terrain_draw(struct avdl_terrain *o) {
	if (o->img.isLoaded(&o->img)) {
		if (avdl_texture_GetWidth(&o->img) == 0 || avdl_texture_GetHeight(&o->img) == 0) {
			avdl_log("image doesn't have width or height: %dx%d", avdl_texture_GetWidth(&o->img), avdl_texture_GetHeight(&o->img));
			//avdl_texture_clean(&o->img);
			return;
		}

		if (avdl_texture_GetPixelFormat(&o->img) != GL_RGB) {
			avdl_log("image has wrong format (non RGB): %d", avdl_texture_GetPixelFormat(&o->img));
			avdl_log("image width height: %dx%d", avdl_texture_GetWidth(&o->img), avdl_texture_GetHeight(&o->img));
			//avdl_texture_clean(&o->img);
			return;
		}
		int pixelStride = 3;

		o->loaded = 1;
		o->width = avdl_texture_GetWidth(&o->img);
		o->height = avdl_texture_GetHeight(&o->img);
		o->heights = malloc(sizeof(float) *o->width *o->height);
		float *pixels = avdl_texture_GetPixels(&o->img);
		if (!pixels) {
			avdl_log("terrain heightmap texture could not get pixels");
			return;
		}
		for (int i = 0; i < o->width *o->height; i++) {
			o->heights[i] = pixels[i*pixelStride] *o->scaleZ;
		}

		o->mesh.vcount = ((o->width -1) *(o->height -1)) *6;
		o->mesh.v = malloc(sizeof(float) *o->mesh.vcount *3);
		o->mesh.dirtyVertices = 1;
		o->mesh.c = malloc(sizeof(float) *o->mesh.vcount *3);
		o->mesh.dirtyColours = 1;
		o->mesh.t = malloc(sizeof(float) *o->mesh.vcount *2);
		o->mesh.dirtyTextures = 1;

		for (int x = 0; x < o->width -1; x++)
		for (int y = 0; y < o->height-1; y++) {

			int index = ((y *(o->width-1)) +x) *18;
			int pixelIndex = ((y *(o->width-0) *3) +(x *3));
			//int pixelIndex = (x *3);
			int pixelIndexRight = pixelIndex +3;
			int pixelIndexTop = pixelIndex +((o->width-0)*3);
			int pixelIndexTopRight = pixelIndexTop +3;
			int indexT = ((y *(o->width-1)) +x) *12;

			int invertTX = (y%5 +y*3%3 +x%3) %2;
			int invertTY = (y%2 +y*4%6 +x*2%5) %2;
			float fromTX = (x+0) *1.0;
			float toTX   = (x+1) *1.0;
			float fromTY = 0;
			float toTY   = 1;

			// rotate
			float cornersX[4];
			float cornersY[4];
			/*
			cornersX[0] = dd_math_randf(0.4);
			cornersY[0] = dd_math_randf(0.4);

			cornersX[2] = cornersX[0] +0.25 +dd_math_randf(0.25);
			cornersY[2] = cornersY[0] +0.25 +dd_math_randf(0.25);
			*/

			cornersX[0] = (float) x /o->width ;
			cornersY[0] = (float) y /o->height;
			//cornersX[0] = 0.0;
			//cornersY[0] = 0.0;

			cornersX[2] = (x+1.0) /o->width ;
			cornersY[2] = (y+1.0) /o->height;
			//cornersX[2] = 1.0;
			//cornersY[2] = 1.0;

			cornersX[1] = cornersX[0];
			cornersY[1] = cornersY[2];

			cornersX[3] = cornersX[2];
			cornersY[3] = cornersY[0];

			/*
			int rotations = (y +x) %4;
			for (int i = 0; i < rotations; i++) {
				float tempX = cornersX[0];
				float tempY = cornersY[0];
				for (int j = 0; j < 3; j++) {
					cornersX[j] = cornersX[j+1];
					cornersY[j] = cornersY[j+1];
				}
				cornersX[3] = tempX;
				cornersY[3] = tempY;
			}
			*/

			// triangle 1

			// vertex 1 - bottom left
			o->mesh.v[index +0] = x *1;
			o->mesh.v[index +1] = pixels[pixelIndex] *o->scaleZ;
			o->mesh.v[index +2] = y *-1;
			o->mesh.c[index +0] = 0;
			o->mesh.c[index +1] = 0;
			o->mesh.c[index +2] = 0;
			//o->mesh.t[indexT +0] = invertTX ? 1 : 0;
			//o->mesh.t[indexT +1] = invertTY ? 1 : 0;
			//o->mesh.t[indexT +0] = fromTX;
			//o->mesh.t[indexT +1] = fromTY;
			o->mesh.t[indexT +0] = cornersX[0];
			o->mesh.t[indexT +1] = cornersY[0];

			// vertex 2
			o->mesh.v[index +3] = x *1 +1;
			o->mesh.v[index +4] = pixels[pixelIndexTopRight] *o->scaleZ;
			o->mesh.v[index +5] = y *-1 -1;
			o->mesh.c[index +3] = 0;
			o->mesh.c[index +4] = 0;
			o->mesh.c[index +5] = 0;
			//o->mesh.t[indexT +2] = invertTX ? 0 : 1;
			//o->mesh.t[indexT +3] = invertTY ? 0 : 1;
			//o->mesh.t[indexT +2] = toTX;
			//o->mesh.t[indexT +3] = toTY;
			o->mesh.t[indexT +2] = cornersX[2];
			o->mesh.t[indexT +3] = cornersY[2];

			// vertex 3
			o->mesh.v[index +6] = x *1;
			o->mesh.v[index +7] = pixels[pixelIndexTop] *o->scaleZ;
			o->mesh.v[index +8] = y *-1 -1;
			o->mesh.c[index +6] = 0;
			o->mesh.c[index +7] = 0;
			o->mesh.c[index +8] = 0;
			//o->mesh.t[indexT +4] = invertTX ? 1 : 0;
			//o->mesh.t[indexT +5] = invertTY ? 0 : 1;
			//o->mesh.t[indexT +4] = fromTX;
			//o->mesh.t[indexT +5] = toTY;
			o->mesh.t[indexT +4] = cornersX[1];
			o->mesh.t[indexT +5] = cornersY[1];

			// triangle 2

			// vertex 1
			o->mesh.v[index +9] = x *1;
			o->mesh.v[index +10] = pixels[pixelIndex] *o->scaleZ;
			o->mesh.v[index +11] = y *-1;
			o->mesh.c[index +9] = 0;
			o->mesh.c[index +10] = 0;
			o->mesh.c[index +11] = 0;
			//o->mesh.t[indexT +6] = invertTX ? 1 : 0;
			//o->mesh.t[indexT +7] = invertTY ? 1 : 0;
			//o->mesh.t[indexT +6] = fromTX;
			//o->mesh.t[indexT +7] = fromTY;
			o->mesh.t[indexT +6] = cornersX[0];
			o->mesh.t[indexT +7] = cornersY[0];

			// vertex 2
			o->mesh.v[index +12] = x *1 +1;
			o->mesh.v[index +13] = pixels[pixelIndexRight] *o->scaleZ;
			o->mesh.v[index +14] = y *-1;
			o->mesh.c[index +12] = 0;
			o->mesh.c[index +13] = 0;
			o->mesh.c[index +14] = 0;
			//o->mesh.t[indexT +8] = invertTX ? 0 : 1;
			//o->mesh.t[indexT +9] = invertTY ? 1 : 0;
			//o->mesh.t[indexT +8] = toTX;
			//o->mesh.t[indexT +9] = fromTY;
			o->mesh.t[indexT +8] = cornersX[3];
			o->mesh.t[indexT +9] = cornersY[3];

			// vertex 3
			o->mesh.v[index +15] = x *1 +1;
			o->mesh.v[index +16] = pixels[pixelIndexTopRight] *o->scaleZ;
			o->mesh.v[index +17] = y *-1 -1;
			o->mesh.c[index +15] = 0;
			o->mesh.c[index +16] = 0;
			o->mesh.c[index +17] = 0;
			//o->mesh.t[indexT +10] = invertTX ? 0 : 1;
			//o->mesh.t[indexT +11] = invertTY ? 0 : 1;
			//o->mesh.t[indexT +10] = toTX;
			//o->mesh.t[indexT +11] = toTY;
			o->mesh.t[indexT +10] = cornersX[2];
			o->mesh.t[indexT +11] = cornersY[2];

		}

		avdl_texture_clean(&o->img);
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

int avdl_terrain_setTextureIndex(struct avdl_terrain *o, struct avdl_texture *img, int index) {
	if (index == 0) {
		o->mesh.setTexture(&o->mesh, img);
	}
	else {
		o->mesh.setTextureIndex(&o->mesh, img, index-1);
	}
}
