#include <stdio.h>
#include <stdlib.h>
#include "avdl_mesh.h"
#include <string.h>
#include "avdl_assetManager.h"
#include "shared/avdl_log.h"
#include <stdlib.h>
#include "avdl_graphics.h"
#include "dd_math.h"
#include <errno.h>

#define CGLTF_IMPLEMENTATION
#include "avdl_cgltf.h"

#ifndef AVDL_DIRECT3D11
extern GLuint defaultProgram;
extern GLuint currentProgram;
#endif

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
extern AAssetManager *aassetManager;
#endif

static const char *skip_whitespace(const char *str);
static const char *skip_to_whitespace(const char *str);
static int filetomesh(struct avdl_mesh_data *m, const char *asset);
static struct avdl_mesh_data *CreateMeshData();

// cache
static struct avdl_dynamic_array meshCache;

// global data
void avdl_mesh_InitGlobalData() {
	avdl_da_init(&meshCache, sizeof(struct avdl_mesh_data *));
}

void avdl_mesh_DeinitGlobalData() {
	if (avdl_da_count(&meshCache) > 0) {
		avdl_log("%d mesh object(s) were not cleaned", avdl_da_count(&meshCache));
		for (int i = 0; i < avdl_da_count(&meshCache); i++) {
			struct avdl_mesh_data *t = avdl_da_getDeref(&meshCache, i);
			avdl_log("    mesh: %s", avdl_string_toCharPtr(&t->filename));
		}
	}
	avdl_da_free(&meshCache);
}

static float shape_triangle[] = {
	0, 0.5, 0,
	-0.5, -0.5, 0,
	0.5, -0.5, 0,
};
static float shape_triangle_lines_loop[] = {
	0, 0.5, 0,
	-0.5, -0.5, 0,
	0.5, -0.5, 0,
};

static float shape_rectangle_fan[] = {
	-0.5,  0.5, 0,
	-0.5, -0.5, 0,
	 0.5, -0.5, 0,
	 0.5,  0.5, 0,
};
static float shape_rectangle_lines_loop[] = {
	-0.5,  0.5, 0,
	-0.5, -0.5, 0,
	 0.5, -0.5, 0,
	 0.5,  0.5, 0,
};

static float shape_box_strip[] = {
	// front side
	-0.5, -0.5, 0.5,
	 0.5, -0.5, 0.5,
	-0.5,  0.5, 0.5,
	 0.5,  0.5, 0.5,

	// top
	 0.5,  0.5, -0.5,

	// right
	 0.5, -0.5,  0.5,
	 0.5, -0.5, -0.5,

	// bottom
	-0.5, -0.5,  0.5,
	-0.5, -0.5, -0.5,

	// left
	-0.5,  0.5,  0.5,
	-0.5,  0.5, -0.5,

	// top 2
	 0.5,  0.5, -0.5,

	// back
	-0.5, -0.5, -0.5,
	 0.5, -0.5, -0.5,
};

static float shape_box_corners[] = {
	// front top right
	0.5, 0.5, 0.5,
	0.5, 0.2, 0.5,

	0.5, 0.5, 0.5,
	0.2, 0.5, 0.5,

	0.5, 0.5, 0.5,
	0.5, 0.5, 0.2,

	// front top left
	-0.5, 0.5, 0.5,
	-0.5, 0.2, 0.5,

	-0.5, 0.5, 0.5,
	-0.2, 0.5, 0.5,

	-0.5, 0.5, 0.5,
	-0.5, 0.5, 0.2,

	// front bottom left
	-0.5, -0.5, 0.5,
	-0.5, -0.2, 0.5,

	-0.5, -0.5, 0.5,
	-0.2, -0.5, 0.5,

	-0.5, -0.5, 0.5,
	-0.5, -0.5, 0.2,

	// front bottom right
	0.5, -0.5, 0.5,
	0.5, -0.2, 0.5,

	0.5, -0.5, 0.5,
	0.2, -0.5, 0.5,

	0.5, -0.5, 0.5,
	0.5, -0.5, 0.2,

	// back top right
	0.5, 0.5, -0.5,
	0.5, 0.2, -0.5,

	0.5, 0.5, -0.5,
	0.2, 0.5, -0.5,

	0.5, 0.5, -0.5,
	0.5, 0.5, -0.2,

	// back top left
	-0.5, 0.5, -0.5,
	-0.5, 0.2, -0.5,

	-0.5, 0.5, -0.5,
	-0.2, 0.5, -0.5,

	-0.5, 0.5, -0.5,
	-0.5, 0.5, -0.2,

	// back bottom left
	-0.5, -0.5, -0.5,
	-0.5, -0.2, -0.5,

	-0.5, -0.5, -0.5,
	-0.2, -0.5, -0.5,

	-0.5, -0.5, -0.5,
	-0.5, -0.5, -0.2,

	// back bottom right
	0.5, -0.5, -0.5,
	0.5, -0.2, -0.5,

	0.5, -0.5, -0.5,
	0.2, -0.5, -0.5,

	0.5, -0.5, -0.5,
	0.5, -0.5, -0.2,

};

static float shape_box_flipped[] = {
	// front side
	-0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,

	0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,

	// back side
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,

	0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,

	// top side
	-0.5,  0.5, -0.5,
	 0.5,  0.5, -0.5,
	-0.5,  0.5,  0.5,

	 0.5,  0.5, -0.5,
	 0.5,  0.5,  0.5,
	-0.5,  0.5,  0.5,

	// bottom side
	-0.5, -0.5, -0.5,
	-0.5, -0.5,  0.5,
	 0.5, -0.5, -0.5,

	 0.5, -0.5, -0.5,
	-0.5, -0.5,  0.5,
	 0.5, -0.5,  0.5,

	// left side
	-0.5, -0.5, -0.5,
	-0.5,  0.5, -0.5,
	-0.5, -0.5,  0.5,

	-0.5,  0.5, -0.5,
	-0.5,  0.5,  0.5,
	-0.5, -0.5,  0.5,

	// right side
	 0.5, -0.5, -0.5,
	 0.5, -0.5,  0.5,
	 0.5,  0.5, -0.5,

	 0.5,  0.5, -0.5,
	 0.5, -0.5,  0.5,
	 0.5,  0.5,  0.5,
};

static float shape_line[] = {
	-0.5, 0.0, 0.0,
	 0.5, 0.0, 0.0,
};

static float shape_pyramid[] = {
	// front
	 0.0,  0.5, 0.0,
	-0.5, -0.5, 0.5,
	 0.5, -0.5, 0.5,

	// right
	 0.0,  0.5,  0.0,
	 0.5, -0.5, -0.5,
	 0.5, -0.5,  0.5,

	// left
	 0.0,  0.5,  0.0,
	-0.5, -0.5, -0.5,
	-0.5, -0.5,  0.5,

	// back
	 0.0,  0.5,  0.0,
	-0.5, -0.5, -0.5,
	 0.5, -0.5, -0.5,

	// bottom 1
	-0.5, -0.5, -0.5,
	 0.5, -0.5, -0.5,
	-0.5, -0.5,  0.5,

	// bottom 2
	 0.5, -0.5, -0.5,
	 0.5, -0.5,  0.5,
	-0.5, -0.5,  0.5,

};

/*
static void clean_position(struct avdl_mesh *m) {
	if (m->v && m->dirtyVertices) {
		free(m->v);
		m->dirtyVertices = 0;
	}
	m->v = 0;
}

static void clean_colour(struct avdl_mesh *m) {
	if (m->c && m->dirtyColours) {
		free(m->c);
		m->dirtyColours = 0;
	}
	m->c = 0;
}

static void clean_textures(struct avdl_mesh *m) {
	if (m->t && m->dirtyTextures) {
		free(m->t);
		m->dirtyTextures = 0;
	}
	m->t = 0;
}

static void clean_normals(struct avdl_mesh *m) {
	if (m->n && m->dirtyNormals) {
		free(m->n);
		m->dirtyNormals = 0;
	}
	m->n = 0;
}

static void clean_tan(struct avdl_mesh *m) {
	if (m->tan && m->dirtyTan) {
		free(m->tan);
		m->dirtyTan = 0;
	}
	m->tan = 0;
}
static void clean_bitan(struct avdl_mesh *m) {
	if (m->bitan && m->dirtyBitan) {
		free(m->bitan);
		m->dirtyTan = 0;
	}
	m->bitan = 0;
}
*/

// constructor
void avdl_mesh_create(struct avdl_mesh *m) {

	m->data = 0;

	// textures
	m->img = 0;
	m->img_normal = 0;
	for (int i = 0; i < TEXTURES_COUNT; i++) {
		m->img_extra[i] = 0;
	}
	m->hasTransparency = 0;

	m->clean = avdl_mesh_clean;

	m->lineWidth = 1.0;

}

void avdl_mesh_set_primitive(struct avdl_mesh *m, enum avdl_primitives shape) {

	// clean previous vertices
	//clean_position(m);

	// set mesh shape based on given value
	switch (shape) {
		case AVDL_PRIMITIVE_TRIANGLE:
			if (!m->data) {
				m->data = CreateMeshData();
			}
			m->data->v = shape_triangle;
			m->data->dirtyVertices = 0;
			m->data->verticesType = AVDL_GRAPHICS_VTYPE_TRIANGLES;
			m->data->vcount = sizeof(shape_triangle) /sizeof(float) /3;
			break;

		case AVDL_PRIMITIVE_TRIANGLE_WIREFRAME:
			if (!m->data) {
				m->data = CreateMeshData();
			}
			m->data->v = shape_triangle_lines_loop;
			m->data->dirtyVertices = 0;
			m->data->verticesType = AVDL_GRAPHICS_VTYPE_LINE_LOOP;
			m->data->vcount = sizeof(shape_triangle_lines_loop) /sizeof(float) /3;
			break;

		case AVDL_PRIMITIVE_RECTANGLE:
			if (!m->data) {
				m->data = CreateMeshData();
			}
			m->data->v = shape_rectangle_fan;
			m->data->dirtyVertices = 0;
			m->data->verticesType = AVDL_GRAPHICS_VTYPE_TRIANGLE_FAN;
			m->data->vcount = sizeof(shape_rectangle_fan) /sizeof(float) /3;
			break;
		case AVDL_PRIMITIVE_RECTANGLE_WIREFRAME:
			if (!m->data) {
				m->data = CreateMeshData();
			}
			m->data->v = shape_rectangle_lines_loop;
			m->data->dirtyVertices = 0;
			m->data->verticesType = AVDL_GRAPHICS_VTYPE_LINE_LOOP;
			m->data->vcount = sizeof(shape_rectangle_lines_loop) /sizeof(float) /3;
			break;

		case AVDL_PRIMITIVE_BOX:
			if (!m->data) {
				m->data = CreateMeshData();
			}
			m->data->v = shape_box_strip;
			m->data->dirtyVertices = 0;
			m->data->verticesType = AVDL_GRAPHICS_VTYPE_TRIANGLE_STRIP;
			m->data->vcount = sizeof(shape_box_strip) /sizeof(float) /3;
			break;

		case AVDL_PRIMITIVE_BOX_FLIP:
			if (!m->data) {
				m->data = CreateMeshData();
			}
			m->data->v = shape_box_flipped;
			m->data->dirtyVertices = 0;
			m->data->verticesType = AVDL_GRAPHICS_VTYPE_TRIANGLES;
			m->data->vcount = sizeof(shape_box_flipped) /sizeof(float) /3;
			break;

		case AVDL_PRIMITIVE_LINE:
			if (!m->data) {
				m->data = CreateMeshData();
			}
			m->data->v = shape_line;
			m->data->dirtyVertices = 0;
			m->data->verticesType = AVDL_GRAPHICS_VTYPE_LINES;
			m->data->vcount = sizeof(shape_line) /sizeof(float) /3;
			break;

		case AVDL_PRIMITIVE_BOX_CORNERS:
			if (!m->data) {
				m->data = CreateMeshData();
			}
			m->data->v = shape_box_corners;
			m->data->dirtyVertices = 0;
			m->data->verticesType = AVDL_GRAPHICS_VTYPE_LINES;
			m->data->vcount = sizeof(shape_box_corners) /sizeof(float) /3;
			break;
		case AVDL_PRIMITIVE_PYRAMID:
			if (!m->data) {
				m->data = CreateMeshData();
			}
			m->data->v = shape_pyramid;
			m->data->dirtyVertices = 0;
			m->data->verticesType = AVDL_GRAPHICS_VTYPE_TRIANGLES;
			m->data->vcount = sizeof(shape_pyramid) /sizeof(float) /3;
			break;
	}

	if (m->data) {
		avdl_vec3_Setf(&m->data->boundsCenter,
			0,
			0,
			0
		);
		avdl_vec3_Setf(&m->data->boundsExtend,
			0.5,
			0.5,
			0.5
		);
	}

}

static void CleanData(struct avdl_mesh *m) {
	if (!m->data) {
		return;
	}

	// cached - reduce cache uses
	if (m->data->uses > 0) {
		m->data->uses--;

		// mesh data still used by something else, just detach it from this mesh
		if (m->data->uses > 0) {
			m->data = 0;
			return;
		}
	}

	struct avdl_mesh_data *data = m->data;

	// position
	if (data->v && data->dirtyVertices) {
		free(data->v);
		data->dirtyVertices = 0;
	}
	data->v = 0;

	// colours
	if (data->c) {
		free(data->c);
	}
	data->c = 0;

	// tex
	if (data->t) {
		free(data->t);
	}
	data->t = 0;

	// normals
	if (data->n) {
		free(data->n);
	}
	data->n = 0;

	// tan
	if (data->tan) {
		free(data->tan);
	}
	data->tan = 0;

	// bitan
	if (data->bitan) {
		free(data->bitan);
	}
	data->bitan = 0;

	// indices
	if (data->indices) {
		free(data->indices);
	}
	data->indices = 0;
	data->indicesCount = 0;

	// boneIds
	if (data->boneIds) {
		free(data->boneIds);
	}
	data->boneIds = 0;

	// weights
	if (data->weights) {
		free(data->weights);
	}
	data->weights = 0;

	avdl_string_clean(&data->filename);

	data->vcount = 0;

	/* animation
	int *boneIds;
	float *weights;
	// animations
	int boneCount;
	struct dd_matrix *inverseBindMatrices;
	struct dd_animation *animations;
	int animationsCount;
	int rootIndex;
	int **children_indices;
	int *children_indices_count;
	struct dd_matrix rootMatrix;
	*/

	if (data->id > 0) {
		GL(glDeleteVertexArrays(1, &data->id));
	}
	data->id = 0;

	if (data->buffer > 0) {
		GL(glDeleteBuffers(1, &data->buffer));
	}
	data->buffer = 0;

	if (data->bufferIndices > 0) {
		GL(glDeleteBuffers(1, &data->bufferIndices));
	}
	data->bufferIndices = 0;

	// remove from cache if active
	for (int i = 0; i < avdl_da_count(&meshCache); i++) {
		struct avdl_mesh_data *temp = avdl_da_getDeref(&meshCache, i);
		if (temp == m->data) {
			avdl_da_remove(&meshCache, 1, i);
			break;
		}
	}

	free(m->data);
	m->data = 0;

}

/* Free mesh from allocated memory
 * the mesh is left in an undefined state.
 * It should either get a new state with a
 * load function or not used anymore.
 */
void avdl_mesh_clean(struct avdl_mesh *m) {
	/*
	clean_position(m);
	clean_colour(m);
	clean_textures(m);
	clean_normals(m);
	clean_tan(m);
	clean_bitan(m);
	*/

	CleanData(m);

}

extern struct dd_matrix matPerspective;
extern struct dd_matrix matView;
extern struct dd_matrix matModel[];
extern int matModel_index;

void avdl_mesh_draw2(struct avdl_mesh *m) {
}

/* draw the mesh itself
 */
void avdl_mesh_draw(struct avdl_mesh *m) {
	if (!m->data) {
		return;
	}

	if (m->data->hasError || m->data->vcount == 0) {
		return;
	}

	if (m->data->id == 0 || m->data->graphicsContextId != avdl_graphics_getContextId()) {

		// keep graphics context up to date
                m->data->graphicsContextId = avdl_graphics_getContextId();

		size_t totalSize = 0;

		// vertex positions
		size_t posOffset = 0;
		size_t posSize = sizeof(float) *3 *m->data->vcount;
		totalSize += posSize;

		// vertex colours
		size_t colOffset = posOffset +posSize;
		size_t colSize = 0;
		if (m->data->c) {
			colSize = sizeof(float) *3 *m->data->vcount;
		}
		totalSize += colSize;

		// texture coordinates
		size_t texOffset = colOffset +colSize;
		size_t texSize = 0;
		if (m->data->t) {
			texSize = sizeof(float) *2 *m->data->vcount;
		}
		totalSize += texSize;

		// normals
		size_t norOffset = texOffset +texSize;
		size_t norSize = 0;
		if (m->data->n) {
			norSize = sizeof(float) *3 *m->data->vcount;
		}
		totalSize += norSize;

		// tan
		size_t tanOffset = norOffset +norSize;
		size_t tanSize = 0;
		if (m->data->tan) {
			tanSize = sizeof(float) *3 *m->data->vcount;
		}
		totalSize += tanSize;

		// bitan
		size_t bitanOffset = tanOffset +tanSize;
		size_t bitanSize = 0;
		if (m->data->bitan) {
			bitanSize = sizeof(float) *3 *m->data->vcount;
		}
		totalSize += bitanSize;

		// bone ids
		size_t boneIdsOffset = bitanOffset +bitanSize;
		size_t boneIdsSize = 0;
		if (m->data->boneIds) {
			boneIdsSize = sizeof(int) *4 *m->data->vcount;
		}
		totalSize += boneIdsSize;

		// weights
		size_t weightsOffset = boneIdsOffset +boneIdsSize;
		size_t weightsSize = 0;
		if (m->data->weights) {
			weightsSize = sizeof(float) *4 *m->data->vcount;
		}
		totalSize += weightsSize;

		// create array as one unit
		void *verticesCol = malloc( totalSize );
		memcpy(((char *)verticesCol) +posOffset, m->data->v, posSize);
		if (m->data->dirtyVertices) {
			free(m->data->v);
			m->data->v = 0;
		}
		if (m->data->c) {
			memcpy(((char *)verticesCol) +colOffset, m->data->c, colSize);
			free(m->data->c);
			m->data->c = 0;
		}
		if (m->data->t) {
			memcpy(((char *)verticesCol) +texOffset, m->data->t, texSize);
			free(m->data->t);
			m->data->t = 0;
		}
		if (m->data->n) {
			memcpy(((char *)verticesCol) +norOffset, m->data->n, norSize);
			free(m->data->n);
			m->data->n = 0;
		}
		if (m->data->tan) {
			memcpy(((char *)verticesCol) +tanOffset, m->data->tan, tanSize);
			free(m->data->tan);
			m->data->tan = 0;
		}
		if (m->data->bitan) {
			memcpy(((char *)verticesCol) +bitanOffset, m->data->bitan, bitanSize);
			free(m->data->bitan);
			m->data->bitan = 0;
		}
		if (m->data->boneIds) {
			memcpy(((char *)verticesCol) +boneIdsOffset, m->data->boneIds, boneIdsSize);
			free(m->data->boneIds);
			m->data->boneIds = 0;
		}
		if (m->data->weights) {
			memcpy(((char *)verticesCol) +weightsOffset, m->data->weights, weightsSize);
			free(m->data->weights);
			m->data->weights = 0;
		}

		// generate array object
		GL(glGenVertexArrays(1, &m->data->id));
		GL(glBindVertexArray(m->data->id));
	
		// generate buffer attached to array
		GL(glGenBuffers(1, &m->data->buffer));
		GL(glBindBuffer(GL_ARRAY_BUFFER, m->data->buffer));

		// give data to buffer
		if (GL(glBufferData(GL_ARRAY_BUFFER, totalSize, verticesCol, GL_STATIC_DRAW)) != 0) {
			avdl_log("error allocating buffer data to draw mesh");
			GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
			GL(glDeleteBuffers(1, &m->data->buffer));

			GL(glBindVertexArray(0));
			GL(glDeleteVertexArrays(1, &m->data->id));
			m->data->hasError = 1;

			free(verticesCol);
			return;
		}
		free(verticesCol);

		// attach vertex positions to current program
		int pos = glGetAttribLocation(currentProgram, "position");
		// program has `position`
		if (pos != -1) {
			GL(glVertexAttribPointer(pos, 3, GL_FLOAT, 0, 0, (void *) posOffset));
			GL(glEnableVertexAttribArray(pos));
		}

		// attach vertex colours
		if (colSize > 0) {
			int col = glGetAttribLocation(currentProgram, "colour");
			// program has colours
			if (col != -1) {
				GL(glVertexAttribPointer(col, 3, GL_FLOAT, 0, 0, (void *) colOffset));
				GL(glEnableVertexAttribArray(col));
			}
		}

		// attach texture coordinates
		if (texSize > 0) {
			int tex = glGetAttribLocation(currentProgram, "texCoord");
			// program has texCoord
			if (tex != -1) {
				GL(glVertexAttribPointer(tex, 2, GL_FLOAT, 0, 0, (void *) texOffset));
				GL(glEnableVertexAttribArray(tex));
			}
		}

		// attach normal
		if (norSize > 0) {
			int nor = glGetAttribLocation(currentProgram, "normal");
			// program has normal
			if (nor != -1) {
				GL(glVertexAttribPointer(nor, 3, GL_FLOAT, 0, 0, (void *) norOffset));
				GL(glEnableVertexAttribArray(nor));
			}
		}

		// attach tan
		if (tanSize > 0) {
			int tanLoc = glGetAttribLocation(currentProgram, "tangent");
			// program has tan
			if (tanLoc != -1) {
				GL(glVertexAttribPointer(tanLoc, 3, GL_FLOAT, 0, 0, (void *) tanOffset));
				GL(glEnableVertexAttribArray(tanLoc));
			}
		}

		// attach bitan
		if (bitanSize > 0) {
			int bitanLoc = glGetAttribLocation(currentProgram, "bitangent");
			// program has bitan
			if (bitanLoc != -1) {
				GL(glVertexAttribPointer(bitanLoc, 3, GL_FLOAT, 0, 0, (void *) bitanOffset));
				GL(glEnableVertexAttribArray(bitanLoc));
			}
		}

		// attach bone ids
		if (boneIdsSize > 0) {
			int boneIdsLoc = glGetAttribLocation(currentProgram, "boneIds");
			// program has boneIds
			if (boneIdsLoc != -1) {
				GL(glVertexAttribIPointer(boneIdsLoc, 4, GL_INT, 0, (void *) boneIdsOffset));
				GL(glEnableVertexAttribArray(boneIdsLoc));
			}
		}

		// attach weights
		if (weightsSize > 0) {
			int weightsLoc = glGetAttribLocation(currentProgram, "weights");
			// program has weights
			if (weightsLoc != -1) {
				GL(glVertexAttribPointer(weightsLoc, 4, GL_FLOAT, 0, 0, (void *) weightsOffset));
				GL(glEnableVertexAttribArray(weightsLoc));
			}
		}

		size_t indicesSize = 0;
		if (m->data->indicesType == AVDL_GRAPHICS_INDICETYPE_UBYTE) {
			indicesSize = sizeof(GLubyte) *m->data->indicesCount;
		}
		else
		if (m->data->indicesType == AVDL_GRAPHICS_INDICETYPE_USHORT) {
			indicesSize = sizeof(GLushort) *m->data->indicesCount;
		}
		else {
			avdl_log("indice type is wrong ?");
		}

		if (indicesSize > 0) {

			// generate buffer for indices
			GL(glGenBuffers(1, &m->data->bufferIndices));
			GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->data->bufferIndices));

			if (GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, m->data->indices, GL_STATIC_DRAW)) == 0) {
				free(m->data->indices);
				m->data->indices = 0;
			}
			else {
				GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
				GL(glDeleteBuffers(1, &m->data->bufferIndices));
				m->data->bufferIndices = 0;
			}
		}

	}

	if (m->hasTransparency) {
		avdl_graphics_EnableBlend();
	}

	if (m->img) {
		avdl_texture_bindIndex(m->img, 0);
		GLuint loc = glGetUniformLocation(currentProgram, "image");
		if (loc != -1) {
			GL(glUniform1i(loc, 0));
		}
	}
	if (m->img_normal) {
		avdl_texture_bindIndex(m->img_normal, 1);
		GLuint loc = glGetUniformLocation(currentProgram, "image_normal");
		if (loc != -1) {
			GL(glUniform1i(loc, 1));
		}
	}
	for (int i = 0; i < TEXTURES_COUNT; i++) {
		if (!m->img_extra[i]) {
			continue;
		}
		avdl_texture_bindIndex(m->img_extra[i], 2 +i);
		char shadername[20] = "image_extra_X";
		shadername[12] = '0' +i;
		GLuint loc = -1;
		loc = glGetUniformLocation(currentProgram, shadername);
		if (loc != -1) {
			GL(glUniform1i(loc, 2 +i));
		}
	}

	GL(glBindVertexArray(m->data->id));

	#if defined(AVDL_QUEST2)
	int MatrixID = avdl_graphics_GetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		//avdl_log("avdl: avdl_mesh: location of `matrix` not found in current program");
	}
	else {
		GL(glUniformMatrix4fv(
			MatrixID,
			1,
			GL_TRUE,
			(float *)dd_matrix_globalGet()
		));
	}
	#else
	int MatrixID = avdl_graphics_GetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		//avdl_log("avdl: avdl_mesh: location of `matrix` not found in current program");
	}
	else {
		avdl_graphics_SetUniformMatrix4f(MatrixID, (float *)dd_matrix_globalGet());
	}
	int MatrixIDProjection = avdl_graphics_GetUniformLocation(currentProgram, "matrix_projection");
	if (MatrixIDProjection >= 0) {
		avdl_graphics_SetUniformMatrix4f(MatrixIDProjection, (float *)&matPerspective);
	}
	int MatrixIDView = avdl_graphics_GetUniformLocation(currentProgram, "matrix_view");
	if (MatrixIDView >= 0) {
		avdl_graphics_SetUniformMatrix4f(MatrixIDView, (float *)&matView);
	}
	int MatrixIDModel = avdl_graphics_GetUniformLocation(currentProgram, "matrix_model");
	if (MatrixIDModel >= 0) {
		avdl_graphics_SetUniformMatrix4f(MatrixIDModel, (float *)&matModel[matModel_index]);
	}
	#endif

	if (m->data->verticesType == AVDL_GRAPHICS_VTYPE_LINES
	||  m->data->verticesType == AVDL_GRAPHICS_VTYPE_LINE_STRIP
	||  m->data->verticesType == AVDL_GRAPHICS_VTYPE_LINE_LOOP) {
		// not possible on OpenGL ES
		//GL(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
		GL(glLineWidth(m->lineWidth));
	}

	// actual draw call
	if (m->data->indices || m->data->bufferIndices > 0) {
		GL(glDrawElements(m->data->verticesType, m->data->indicesCount, m->data->indicesType, m->data->indices));
	}
	else {
		GL(glDrawArrays(m->data->verticesType, 0, m->data->vcount));
	}
	GL(glBindVertexArray(0));

	if (m->img) {
		avdl_texture_unbindIndex(m->img, 0);
	}

	if (m->img_normal) {
		avdl_texture_unbindIndex(m->img_normal, 1);
	}

	for (int i = 0; i < TEXTURES_COUNT; i++) {
		if (!m->img_extra[i]) {
			continue;
		}
		avdl_texture_unbindIndex(m->img_extra[i], 2 +i);
	}

	if (m->hasTransparency) {
		avdl_graphics_DisableBlend();
	}
	#ifdef AVDL_DIRECT3D11
	/*
	if (!m->vertexBuffer && m->v) {
		avdl_graphics_direct3d11_setVertexBufferMesh(m);
	}
	avdl_graphics_direct3d11_drawMeshMesh(m, dd_matrix_globalGet());
	if (!m->parent.vertexBuffer && m->c) {
		avdl_graphics_direct3d11_setVertexBuffer(m);
	}
	avdl_graphics_direct3d11_drawMesh(m, dd_matrix_globalGet());
	if (!m->parent.parent.vertexBuffer && m->t) {
		avdl_graphics_direct3d11_setVertexBufferTexture(m);
	}
	avdl_graphics_direct3d11_drawMeshTexture(m, dd_matrix_globalGet());
	*/
	#endif
}

static struct avdl_mesh_data *CreateMeshData() {
	struct avdl_mesh_data *data = malloc(sizeof(struct avdl_mesh_data));
	data->graphicsContextId = -1;
	data->vcount = 0;
	data->verticesType = AVDL_GRAPHICS_VTYPE_TRIANGLES;
	data->v = 0;
	data->dirtyVertices = 0;
	data->c = 0;
	data->t = 0;
	data->n = 0;
	data->tan = 0;
	data->bitan = 0;
	data->boneIds = 0;
	data->weights = 0;
	data->indices = 0;
	data->indicesType = AVDL_GRAPHICS_INDICETYPE_UBYTE;
	data->indicesCount = 0;

	data->boneCount = 0;
	data->inverseBindMatrices = 0;
	data->animations = 0;
	data->animationsCount = 0;
	data->rootIndex = -1;
	data->children_indices = 0;
	data->children_indices_count = 0;

	avdl_vec3_create(&data->boundsCenter);
	avdl_vec3_Setf(&data->boundsCenter, 0, 0, 0);
	avdl_vec3_create(&data->boundsExtend);
	avdl_vec3_Setf(&data->boundsExtend, 0, 0, 0);

	data->id = 0;
	data->buffer = 0;
	data->bufferIndices = 0;

	avdl_string_create(&data->filename);
	avdl_string_SetMaxCharacters(&data->filename, 1024);

	// cache
	data->uses = 0;

	data->hasError = 0;
	return data;
}

static struct avdl_mesh_data *GetDataFromFile(const char *filename) {

	// check cache here
	for (int i = 0; i < avdl_da_count(&meshCache); i++) {
		struct avdl_mesh_data *t = avdl_da_getDeref(&meshCache, i);
		if (strcmp(avdl_string_toCharPtr(&t->filename), filename) == 0) {
			if (t->boneCount != 0) break; // for now don't cache animation files
			return t;
		}
	}

	struct avdl_mesh_data *data = CreateMeshData();
	filetomesh(data, filename);
	avdl_da_push(&meshCache, &data);
	return data;
}

static int SetData(struct avdl_mesh *o, struct avdl_mesh_data *data) {
	CleanData(o);
	data->uses++;
	o->data = data;
	return 0;
}

/*
 * add the mesh to be loaded from the asset manager
 */
void avdl_mesh_load(struct avdl_mesh *m, const char *asset) {

	// clean the mesh, if was dirty
	avdl_mesh_clean(m);

	avdl_assetManager_AddLoadOperation(m, asset, GetDataFromFile, SetData);

}

void avdl_mesh_loadLocal(struct avdl_mesh *m, const char *asset) {

	// clean the mesh, if was dirty
	avdl_mesh_clean(m);

	// mark to be loaded
	avdl_assetManager_AddLoadOperationLocal(m, asset, GetDataFromFile, SetData);

}

void avdl_mesh_copy(struct avdl_mesh *dest, struct avdl_mesh *src) {
	/*
	avdl_mesh_clean(dest);
	dest->vcount = src->vcount;
	dest->v = malloc(src->vcount *sizeof(float) *3);
	memcpy(dest->v, src->v, sizeof(float) *src->vcount *3);
	dest->dirtyVertices = 1;

	// optional colours
	if (src->c) {
		dest->c = malloc(sizeof(float) *src->vcount *3);
		memcpy(dest->c, src->c, sizeof(float) *src->vcount *3);
		dest->dirtyColours = 1;
	}

	// optional texture coordinates
	if (src->t) {
		dest->t = malloc(sizeof(float) *(dest->vcount*2));
		memcpy(dest->t, src->t, sizeof(float) *(dest->vcount*2));
		dest->dirtyTextures = 1;
	}
	*/
}

void avdl_mesh_combine(struct avdl_mesh *dst, struct avdl_mesh *src, float offsetX, float offsetY, float offsetZ) {
	/*
	dst->v = realloc(dst->v, (dst->vcount +src->vcount) *sizeof(float) *3);
	dst->dirtyVertices = 1;
	for (int i = dst->vcount *3; i < (dst->vcount +src->vcount) *3; i += 3) {
		dst->v[i+0] = src->v[(i+0) -(dst->vcount *3)] +offsetX;
		dst->v[i+1] = src->v[(i+1) -(dst->vcount *3)] +offsetY;
		dst->v[i+2] = src->v[(i+2) -(dst->vcount *3)] +offsetZ;
	}
	dst->vcount += src->vcount;

	if ((!dst->c && src->c) || dst->c) {
		dst->c = realloc(dst->c, dst->vcount *sizeof(float) *4);
		dst->dirtyColours = 1;
		int oldVertices = dst->vcount -src->vcount;
		for (int i = oldVertices *4; i < dst->vcount *4; i += 4) {
			// get new mesh's colour
			if (src->c) {
				dst->c[i+0] = src->c[(i+0) -(oldVertices *4)];
				dst->c[i+1] = src->c[(i+1) -(oldVertices *4)];
				dst->c[i+2] = src->c[(i+2) -(oldVertices *4)];
				dst->c[i+3] = src->c[(i+3) -(oldVertices *4)];
			}
			// new mesh has no colour - add default
			else {
				dst->c[i+0] = 0;
				dst->c[i+1] = 0;
				dst->c[i+2] = 0;
				dst->c[i+3] = 0;
			}
		}
	}

	if ((!dst->t && src->t) || dst->t) {
		dst->t = realloc(dst->t, dst->vcount *sizeof(float) *2);
		dst->dirtyTextures = 1;
		int oldVertices = dst->vcount -src->vcount;
		for (int i = oldVertices *2; i < dst->vcount *2; i += 2) {
			if (src->t) {
				dst->t[i+0] = src->t[(i+0) -(oldVertices *2)];
				dst->t[i+1] = src->t[(i+1) -(oldVertices *2)];
			}
			else {
				dst->t[i+0] = 0;
				dst->t[i+1] = 0;
			}
		}
	}
	*/
}

void avdl_mesh_translatef(struct avdl_mesh *o, float x, float y, float z) {
	/*
	if (o->v && !o->dirtyVertices) {
		float *p = malloc(sizeof(float) *o->vcount *3);
		memcpy(p, o->v, sizeof(float) *o->vcount *3);
		o->v = p;
		o->dirtyVertices = 1;
	}
	for (int i = 0; i < o->vcount; i++) {
		o->v[i*3 +0] += x;
		o->v[i*3 +1] += y;
		o->v[i*3 +2] += z;
	}
	*/
}

void avdl_mesh_scalef(struct avdl_mesh *o, float x, float y, float z) {
	/*
	if (o->v && !o->dirtyVertices) {
		float *p = malloc(sizeof(float) *o->vcount *3);
		memcpy(p, o->v, sizeof(float) *o->vcount *3);
		o->v = p;
		o->dirtyVertices = 1;
	}
	for (int i = 0; i < o->vcount; i++) {
		o->v[i*3 +0] *= x;
		o->v[i*3 +1] *= y;
		o->v[i*3 +2] *= z;
	}
	*/
}

void avdl_mesh_set_colour(struct avdl_mesh *m, float r, float g, float b) {
	//clean_colour(m);
//	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
//	m->c = malloc(m->vcount *sizeof(float) *4);
//	m->dirtyColours = 1;
//	for (int i = 0; i < m->vcount *4; i += 4) {
//		m->c[i+0] = r;
//		m->c[i+1] = g;
//		m->c[i+2] = b;
//		m->c[i+3] = 0;
//	}
//	#else

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	r = dd_math_pow(r, 2.2);
	g = dd_math_pow(g, 2.2);
	b = dd_math_pow(b, 2.2);
	#endif

	/*
	m->c = malloc(m->vcount *sizeof(float) *3);
	m->dirtyColours = 1;
	for (int i = 0; i < m->vcount *3; i += 3) {
		m->c[i+0] = r;
		m->c[i+1] = g;
		m->c[i+2] = b;
	}
	*/
//	#endif

	if (m->data) {
		m->data->c = malloc(m->data->vcount *sizeof(float) *3);
		//m->dirtyColours = 1;

		for (int i = 0; i < m->data->vcount *3; i += 3) {
			m->data->c[i+0] = r;
			m->data->c[i+1] = g;
			m->data->c[i+2] = b;
		}
	}
}

void avdl_mesh_set_primitive_texcoords(struct avdl_mesh *m, float offsetX, float offsetY, float sizeX, float sizeY) {
	/*
	for (int i = 0; i < m->vcount*2; i += 2) {
		m->t[i+0] *= sizeX;
		m->t[i+0] += offsetX;

		m->t[i+1] *= sizeY;
		m->t[i+1] += offsetY;
	}
	*/
}

void avdl_mesh_setTransparency(struct avdl_mesh *o, int transparency) {
	o->hasTransparency = transparency;
}

void avdl_mesh_setTexture(struct avdl_mesh *o, struct avdl_texture *tex) {
	o->img = tex;
}

void avdl_mesh_setTextureNormal(struct avdl_mesh *o, struct avdl_texture *tex) {
	o->img_normal = tex;
}

void avdl_mesh_setTextureIndex(struct avdl_mesh *o, struct avdl_texture *tex, int index) {
	if (index < 0 || index >= TEXTURES_COUNT) {
		avdl_log("avdl_mesh: texture index out of bounds: %d / %d", index, TEXTURES_COUNT);
		return;
	}
	o->img_extra[index] = tex;
}

int avdl_mesh_hasTexture(struct avdl_mesh *o) {
	return o->img != 0;
}

void avdl_mesh_setWireframe(struct avdl_mesh *o) {
}

void avdl_mesh_setSolid(struct avdl_mesh *o) {
}

void avdl_mesh_SetTypeLine(struct avdl_mesh *o, float lineWidth) {
	o->lineWidth = lineWidth;
}

struct avdl_vec3 *avdl_mesh_GetBoundsCenter(struct avdl_mesh *o) {
	if (o->data) {
		return &o->data->boundsCenter;
	}
	return 0;
}
struct avdl_vec3 *avdl_mesh_GetBoundsExtend(struct avdl_mesh *o) {
	if (o->data) {
		return &o->data->boundsExtend;
	}
	return 0;
}

static int load_ply(struct avdl_mesh_data *data, const char *path);
static int load_gltf(struct avdl_mesh_data *data, const char *path);
static int load_ply_string(struct avdl_mesh_data *m, const char *string);
static int load_gltf_internal(struct avdl_mesh_data *m, cgltf_options *options, cgltf_data *data);

/* Select the right function depending on file_type */
static int filetomesh(struct avdl_mesh_data *m, const char *asset) {

	const char *cend = asset;
	cend += strlen(asset);

	avdl_string_empty(&m->filename);
	avdl_string_cat(&m->filename, asset);

	if (strcmp(cend -strlen(".ply"), ".ply") == 0) {
		return load_ply(m, asset);
	}
	else
	if (strcmp(cend -strlen(".glb"), ".glb") == 0) {
		return load_gltf(m, asset);
	}

	avdl_log("avdl_mesh: filetomesh: unsupported file format: %s", asset);

	return -1;
}

static int load_ply(struct avdl_mesh_data *data, const char *path) {

	#if defined( AVDL_DIRECT3D11 )
	return 0;
	#else

	/*
	struct avdl_time t;
	avdl_time_start(&t);
	*/
	//avdl_log("file: %s", path);

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	//Open file and check error
	AAsset *f = AAssetManager_open(aassetManager, path, AASSET_MODE_UNKNOWN);
	if (!f)
	{
		avdl_log("load_ply: error opening file: %s: %s", path, "unknown error");
		return -1;
	}

	char *fc = AAsset_getBuffer(f);
	if ( load_ply_string(data, fc) != 0) {
		avdl_log("avdl: avdl_load_ply_string: failed to load asset: %s", path);
		AAsset_close(f);
		return -1;
	}

	AAsset_close(f);
	#else

	//Open file and check error
	FILE *f;
	#if defined( AVDL_DIRECT3D11 )
	f = avdl_filetomesh_openFile(path);
	#else
	f = fopen(path, "r");
	#endif

	if (!f)
	{
		#if defined( AVDL_DIRECT3D11 )
		avdl_log("load_ply: error opening file: %s", path);
		#else
		avdl_log("avdl: avdl_load_ply: error opening file: %s: %s", path, strerror(errno));
		#endif
		return -1;
	}

	// put file into memory
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *fstr = malloc(fsize + 1);
	fread(fstr, fsize, 1, f);
	fclose(f);

	if ( load_ply_string(data, fstr) != 0) {
		avdl_log("avdl: avdl_load_ply_string: failed to load asset: %s", path);
		free(fstr);
		return -1;
	}

	// clean
	free(fstr);

	/*
	avdl_time_end(&t);
	avdl_log("loading took %ld for size %ld", avdl_time_getTimeDouble(&t), fsize);
	*/

	#endif

	#endif
	return 0;
}

#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
static int load_gltf_string(struct avdl_mesh_data *m, void *file_data, off_t size) {

	// initialise cgltf
	cgltf_options options = {0};
	cgltf_data *data = 0;
	cgltf_result result = cgltf_parse(&options, file_data, size, &data);
	if (result != cgltf_result_success) {
		avdl_log("avdl: %s: error loading gtlf file in string mode");
		return -1;
	}

	// load cgltf buffers
	result = cgltf_load_buffers(&options, data, 0);
	if (result != cgltf_result_success) {
		avdl_log("avdl %s: error loading gtlf buffers in string mode");
		cgltf_free(data);
		return -1;
	}

	if (load_gltf_internal(m, &options, data) != 0) {
		avdl_log("avdl %s: error loading gtlf string internal data");
		cgltf_free(data);
		return -1;
	}

	cgltf_free(data);
	return 0;
}
#endif

static int load_gltf(struct avdl_mesh_data *data, const char *path) {

	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	//Open file and check error
	AAsset *f = AAssetManager_open(aassetManager, path, AASSET_MODE_UNKNOWN);
	if (!f)
	{
		avdl_log("load_gltf: error opening file: %s: %s", path, "unknown error");
		return -1;
	}

	void *fc = AAsset_getBuffer(f);
	off_t fs = AAsset_getLength(f);
	if ( load_gltf_string(data, fc, fs) != 0) {
		avdl_log("avdl: load_gltf_string: failed to load asset: %s", path);
		AAsset_close(f);
		return -1;
	}

	AAsset_close(f);
	#else

	//avdl_log("path: %s", path);
	// initialise cgltf
	cgltf_options options = {0};
	cgltf_data *cgltfdata = 0;
	cgltf_result result = cgltf_parse_file(&options, path, &cgltfdata);
	if (result != cgltf_result_success) {
		avdl_log("avdl: %s: error loading gltf file", path);
		return -1;
	}

	// load cgltf buffers
	result = cgltf_load_buffers(&options, cgltfdata, 0);
	if (result != cgltf_result_success) {
		avdl_log("avdl %s: error loading gltf buffers", path);
		cgltf_free(data);
		return -1;
	}

	if (load_gltf_internal(data, &options, cgltfdata) != 0) {
		avdl_log("avdl %s: error loading gltf internal data", path);
		cgltf_free(data);
		return -1;
	}
	cgltf_free(cgltfdata);
	#endif
	return 0;
}

static int load_ply_string(struct avdl_mesh_data *m, const char *string) {

	#if defined( AVDL_DIRECT3D11 )
	return 0;
	#else

	// main pointer
	const char *p = string;

	// verify ply signature
	if ( strncmp(p, "ply", strlen("ply")) != 0) {
		avdl_log("avdl: avdl_load_ply_string: ply signature not found");
		return -1;
	}
	p += 3;

	// ply structures
	enum ply_format {
		PLY_FORMAT_CHAR,
		PLY_FORMAT_UCHAR,
		PLY_FORMAT_SHORT,
		PLY_FORMAT_USHORT,
		PLY_FORMAT_INT,
		PLY_FORMAT_UINT,
		PLY_FORMAT_FLOAT,
		PLY_FORMAT_DOUBLE,
		PLY_FORMAT_NONE,
	};

	struct avdl_ply_property {
		char name[100];
		enum ply_format format;
		enum ply_format list_format;
	};

	struct avdl_ply_element {
		char name[100];
		struct avdl_ply_property p[100];
		int propertyTotal;
		int amount;
	};

	// maximum number of elements reduced as it was causing memory issues on android
	struct avdl_ply_element elements[10];
	int elementTotal = 0;

	int vertex_count = 0;

	// ready ply commands
	while ( strncmp(p, "end_header", strlen("end_header")) != 0 ) {

		p = skip_whitespace(p);

		// format
		if ( strncmp(p, "format", strlen("format")) == 0 ) {
			p += strlen("format");
			p = skip_whitespace(p);
			if ( strncmp(p, "ascii 1.0", strlen("ascii 1.0")) != 0) {
				avdl_log("avdl: avdl_load_ply_string: unsupported ply format, only ascii supported");
				return -1;
			}
			while (p[0] != '\n') {
				p++;
			}
			p++;
		}
		else
		// comments
		if ( strncmp(p, "comment", strlen("comment")) == 0 ) {
			p += strlen("comment");
			while (p[0] != '\n') {
				p++;
			}
			p++;
		}
		else
		// elements
		if ( strncmp(p, "element", strlen("element")) == 0 ) {
			p += strlen("element");

			// element limit
			if (elementTotal >= 10) {
				avdl_log("avdl: avdl_load_ply_string: too many elements, max is 10");
				return -1;
			}

			// init element
			elements[elementTotal].propertyTotal = 0;

			// get element name
			p = skip_whitespace(p);
			const char *p2 = p;
			p2 = skip_to_whitespace(p2);
			if (p2 -p >= 100) {
				avdl_log("avdl: avdl_load_ply_string: element name has to be smaller than 100 characters");
				return -1;
			}

			// attach it to element
			strncpy(elements[elementTotal].name, p, p2 -p);
			elements[elementTotal].name[p2 -p] = '\0';
			p = p2;

			// get element amount
			p = skip_whitespace(p);
			elements[elementTotal].amount = atoi(p);
			p = skip_to_whitespace(p);

			if ( strncmp( elements[elementTotal].name, "vertex", strlen("vertex") ) == 0 ) {
				vertex_count = elements[elementTotal].amount;
			}

			//avdl_log("element: %s %d - %d", elements[elementTotal].name, elements[elementTotal].amount, elementTotal);

			// advance element
			elementTotal++;

			// temp skip
			while (p[0] != '\n') {
				p++;
			}
			p++;
		}
		else
		// property
		if ( strncmp(p, "property", strlen("property")) == 0 ) {
			p += strlen("property");
			p = skip_whitespace(p);

			struct avdl_ply_property *property = &elements[elementTotal-1].p[elements[elementTotal-1].propertyTotal];

			// check element is valid
			if (elementTotal == 0) {
				avdl_log("avdl: avdl_load_ply_string: property found before element");
				return -1;
			}

			property->list_format = PLY_FORMAT_NONE;
			// check if list
			if ( strncmp(p, "list", strlen("list")) == 0 ) {
				p += strlen("list");
				p = skip_whitespace(p);

				// get list type
				const char *p2 = p;
				p2 = skip_to_whitespace(p2);
				if (p2 -p >= 100) {
					avdl_log("avdl: avdl_load_ply_string: property list type has to be smaller than 100 characters");
					return -1;
				}

				// detect format
				if ( strncmp(p, "uchar", strlen("uchar")) == 0 ) {
					property->list_format = PLY_FORMAT_UCHAR;
				}
				else {
					avdl_log("avdl: avdl_load_ply_string: unsupported list type");
					return -1;
				}
				p = p2;
				p = skip_whitespace(p);
			}

			// get property type
			const char *p2 = p;
			p2 = skip_to_whitespace(p2);
			if (p2 -p >= 100) {
				avdl_log("avdl: avdl_load_ply_string: property type has to be smaller than 100 characters");
				return -1;
			}

			// detect format
			if ( strncmp(p, "float", strlen("float")) == 0 ) {
				property->format = PLY_FORMAT_FLOAT;
			}
			else
			if ( strncmp(p, "uchar", strlen("uchar")) == 0 ) {
				property->format = PLY_FORMAT_UCHAR;
			}
			else
			if ( strncmp(p, "uint", strlen("uint")) == 0 ) {
				property->format = PLY_FORMAT_UINT;
			}
			else {
				property->format = PLY_FORMAT_NONE;
			}
			p = p2;

			// property name
			p = skip_whitespace(p);
			p2 = p;
			p2 = skip_to_whitespace(p2);
			if (p2 -p >= 100) {
				avdl_log("avdl: avdl_load_ply_string: property name has to be smaller than 100 characters");
				return -1;
			}
			strncpy(property->name, p, p2 -p);
			property->name[p2 -p] = '\0';
			p = p2;
			elements[elementTotal-1].propertyTotal++;

			//avdl_log("property: %s %d - %d %d", property->name, property->format, elementTotal-1, elements[elementTotal-1].propertyTotal);

		}
		// unrecognised word
		else {
			const char *p2 = p;
			p2 = skip_to_whitespace(p2);

			char temp[100];
			strncpy(temp, p, dd_math_min(p2-p, 99));
			temp[dd_math_min(p2-p, 99)] = '\0';
			//avdl_log("unrecognised word: %s", temp);
			return -1;
		}

		p = skip_whitespace(p);
	}
	p = skip_whitespace(p);
	p += strlen("end_header");
	p = skip_whitespace(p);

	// check if mesh is valid
	int has_positions = 0;
	int has_colours = 0;
	int has_texcoord = 0;
	int has_normals = 0;

	for (int i = 0; i < elementTotal; i++) {
		struct avdl_ply_element *element = &elements[i];

		// currently only check vertex attributes
		if ( strncmp(element->name, "vertex", strlen("vertex")) != 0) {
			continue;
		}

		// pos
		int has_x = 0;
		int has_y = 0;
		int has_z = 0;

		// col
		int has_red = 0;
		int has_green = 0;
		int has_blue = 0;

		// normals
		int has_nx = 0;
		int has_ny = 0;
		int has_nz = 0;

		// tex
		int has_s = 0;
		int has_t = 0;

		for (int j = 0; j < elements[i].propertyTotal; j++) {
			struct avdl_ply_property *property = &elements[i].p[j];

			// pos
			if ( strncmp(property->name, "x", strlen("x")) == 0 ) {
				has_x = 1;
			}
			else
			if ( strncmp(property->name, "y", strlen("y")) == 0 ) {
				has_y = 1;
			}
			else
			if ( strncmp(property->name, "z", strlen("z")) == 0 ) {
				has_z = 1;
			}
			else
			// col
			if ( strncmp(property->name, "red", strlen("red")) == 0 ) {
				has_red = 1;
			}
			else
			if ( strncmp(property->name, "green", strlen("green")) == 0 ) {
				has_green = 1;
			}
			else
			if ( strncmp(property->name, "blue", strlen("blue")) == 0 ) {
				has_blue = 1;
			}
			else
			// normals
			if ( strncmp(property->name, "nx", strlen("nx")) == 0 ) {
				has_nx = 1;
			}
			else
			if ( strncmp(property->name, "ny", strlen("ny")) == 0 ) {
				has_ny = 1;
			}
			else
			if ( strncmp(property->name, "nz", strlen("nz")) == 0 ) {
				has_nz = 1;
			}
			else
			// tex
			if ( strncmp(property->name, "s", strlen("s")) == 0 ) {
				has_s = 1;
			}
			else
			if ( strncmp(property->name, "t", strlen("t")) == 0 ) {
				has_t = 1;
			}
		}

		// pos
		if (has_x && has_y && has_z) {
			has_positions = 1;
		}

		// col
		if (has_red && has_green && has_blue) {
			has_colours = 1;
		}

		// normals
		if (has_nx && has_ny && has_nz) {
			has_normals = 1;
		}

		// tex
		if (has_s && has_t) {
			has_texcoord = 1;
		}
	}

	// error!
	if (!has_positions) {
		avdl_log("avdl: avdl_load_ply_string: asset has no vertex positions!");
		return -1;
	}

	// vertex attributes v2
	float *array_vertex_pos = malloc(sizeof(float) *vertex_count *3);
	float *array_vertex_col = 0;
	float *array_vertex_st = 0;
	float *array_vertex_nor = 0;
	if (has_colours) {
		array_vertex_col = malloc(sizeof(float) *vertex_count *3);
	}
	if (has_texcoord) {
		array_vertex_st = malloc(sizeof(float) *vertex_count *2);
	}
	if (has_normals) {
		array_vertex_nor = malloc(sizeof(float) *vertex_count *3);
	}
	struct avdl_dynamic_array array_vertex_indices;
	avdl_da_init(&array_vertex_indices, sizeof(unsigned int));

	// bounds
	struct avdl_vec3 boundsMin;
	struct avdl_vec3 boundsMax;

	int useIndices = 1;
	if (vertex_count < 250) {
		m->indicesType = AVDL_GRAPHICS_INDICETYPE_UBYTE;
	}
	else
	if (vertex_count < 65000) {
		m->indicesType = AVDL_GRAPHICS_INDICETYPE_USHORT;
	}
	// fallback to not using indices
	else {
		useIndices = 0;
	}

	// for each element
	for (int i = 0; i < elementTotal; i++) {
		struct avdl_ply_element *element = &elements[i];

		int is_vertex = 0;
		int is_face_indices = 0;

		if ( strncmp(element->name, "vertex", strlen("vertex")) == 0 ) {
			is_vertex = 1;
		}
		if ( strncmp(element->name, "face", strlen("face")) == 0 ) {
			is_face_indices = 1;
		}

		//avdl_log("element %d %s properties %d", i, element->name, element->propertyTotal);
		for (int j = 0; j < element->amount; j++) {

			for (int z = 0; z < element->propertyTotal; z++) {
				struct avdl_ply_property *property = &element->p[z];
				//avdl_log("property %s %d %d", property->name, z, property->format);

				p = skip_whitespace(p);

				int values = 1;

				// is list
				if (property->list_format == PLY_FORMAT_UCHAR) {
					p = skip_whitespace(p);
					values = atoi(p);
					p = skip_to_whitespace(p);
					//avdl_log("is list with values %d", values);
				}

				// read property value(s)
				if (property->format == PLY_FORMAT_FLOAT) {
					for (int list_i = 0; list_i < values; list_i++) {
						p = skip_whitespace(p);
						float f = atof(p);
						p = skip_to_whitespace(p);

						if ( is_vertex && strncmp( property->name, "x", strlen("x") ) == 0) {
							array_vertex_pos[j*3 +0] = f;
							if (j == 0 || avdl_vec3_X(&boundsMin) > array_vertex_pos[j*3 +0]) {
								avdl_vec3_SetX(&boundsMin, array_vertex_pos[j*3 +0]);
							}
							if (j == 0 || avdl_vec3_X(&boundsMax) < array_vertex_pos[j*3 +0]) {
								avdl_vec3_SetX(&boundsMax, array_vertex_pos[j*3 +0]);
							}
						}
						else
						if ( is_vertex && strncmp( property->name, "y", strlen("y") ) == 0) {
							array_vertex_pos[j*3 +1] = f;
							if (j == 0 || avdl_vec3_Y(&boundsMin) > array_vertex_pos[j*3 +1]) {
								avdl_vec3_SetY(&boundsMin, array_vertex_pos[j*3 +1]);
							}
							if (j == 0 || avdl_vec3_Y(&boundsMax) < array_vertex_pos[j*3 +1]) {
								avdl_vec3_SetY(&boundsMax, array_vertex_pos[j*3 +1]);
							}
						}
						else
						if ( is_vertex && strncmp( property->name, "z", strlen("z") ) == 0) {
							array_vertex_pos[j*3 +2] = f;
							if (j == 0 || avdl_vec3_Z(&boundsMin) > array_vertex_pos[j*3 +2]) {
								avdl_vec3_SetZ(&boundsMin, array_vertex_pos[j*3 +2]);
							}
							if (j == 0 || avdl_vec3_Z(&boundsMax) < array_vertex_pos[j*3 +2]) {
								avdl_vec3_SetZ(&boundsMax, array_vertex_pos[j*3 +2]);
							}
						}
						else
						if ( is_vertex && strncmp( property->name, "nx", strlen("nx") ) == 0 && has_normals) {
							array_vertex_nor[j*3 +0] = f;
						}
						else
						if ( is_vertex && strncmp( property->name, "ny", strlen("ny") ) == 0 && has_normals) {
							array_vertex_nor[j*3 +1] = f;
						}
						else
						if ( is_vertex && strncmp( property->name, "nz", strlen("nz") ) == 0 && has_normals) {
							array_vertex_nor[j*3 +2] = f;
						}
						else
						if ( is_vertex && strncmp( property->name, "s", strlen("s") ) == 0 && has_texcoord) {
							array_vertex_st[j*2 +0] = f;
						}
						else
						if ( is_vertex && strncmp( property->name, "t", strlen("t") ) == 0 && has_texcoord) {
							array_vertex_st[j*2 +1] = f;
						}
						//avdl_log("\tfloat: %f", f);
					}
				}
				else
				if (property->format == PLY_FORMAT_UCHAR) {
					for (int list_i = 0; list_i < values; list_i++) {
						p = skip_whitespace(p);
						int integer = atoi(p);
						p = skip_to_whitespace(p);

						if ( is_vertex && strncmp( property->name, "red", strlen("red") ) == 0 && has_colours) {
							#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
							array_vertex_col[j*3 +0] = integer /255.0;
							#else
							array_vertex_col[j*3 +0] = dd_math_pow(integer /255.0, 2.2);
							#endif
						}
						else
						if ( is_vertex && strncmp( property->name, "green", strlen("green") ) == 0 && has_colours) {
							#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
							array_vertex_col[j*3 +1] = integer /255.0;
							#else
							array_vertex_col[j*3 +1] = dd_math_pow(integer /255.0, 2.2);
							#endif
						}
						else
						if ( is_vertex && strncmp( property->name, "blue", strlen("blue") ) == 0 && has_colours) {
							#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
							array_vertex_col[j*3 +2] = integer /255.0;
							#else
							array_vertex_col[j*3 +2] = dd_math_pow(integer /255.0, 2.2);
							#endif
						}
						else
						// for the time being no alpha on vertex colours
						if ( is_vertex && strncmp( property->name, "alpha", strlen("alpha") ) == 0 && has_colours) {
							//avdl_da_push(&array_vertex_alpha, &integer);
						}
						//avdl_log("\tuchar: %d", integer);
					}
				}
				else
				if (property->format == PLY_FORMAT_UINT) {
					for (int list_i = 0; list_i < values; list_i++) {
						p = skip_whitespace(p);
						int integer = atoi(p);
						p = skip_to_whitespace(p);

						// no negative indices
						if (integer < 0) {
							avdl_log("vertex index is shouldn't be negative");
							return -1;
						}

						if ( is_face_indices && strncmp( property->name, "vertex_indices", strlen("vertex_indices") ) == 0) {
							if (list_i >= 3) {
								// do not insert parts of the array in itself, extract numbers first
								unsigned int i1 = ((int*)avdl_da_get(&array_vertex_indices, -3 +((list_i -3) *3)))[0];
								unsigned int i2 = ((int*)avdl_da_get(&array_vertex_indices, -1))[0];
								avdl_da_push(&array_vertex_indices, &i1);
								avdl_da_push(&array_vertex_indices, &i2);
							}
							avdl_da_push(&array_vertex_indices, &integer);
						}

						//avdl_log("\tuint: %d", integer);
					}
				}
			}
		}
	}

	// init loaded mesh

	m->verticesType = AVDL_GRAPHICS_VTYPE_TRIANGLES;
	m->c = 0;
	m->t = 0;
	m->n = 0;
	m->tan = 0;
	m->bitan = 0;
	m->boneIds = 0;
	m->weights = 0;

	if (useIndices) {
		// indices way
		m->vcount = vertex_count;
		m->indicesCount = array_vertex_indices.elements;
		if (m->indicesType == AVDL_GRAPHICS_INDICETYPE_UBYTE) {
			m->indices = malloc(sizeof(GLubyte) *m->indicesCount);
		}
		else
		if (m->indicesType == AVDL_GRAPHICS_INDICETYPE_USHORT) {
			m->indices = malloc(sizeof(GLushort) *m->indicesCount);
		}
		m->v = array_vertex_pos;
		m->dirtyVertices = 1;
		m->c = array_vertex_col;
		m->t = array_vertex_st;
		m->n = array_vertex_nor;

		for (unsigned int i = 0; i < m->indicesCount; i++) {
			int *index = avdl_da_get(&array_vertex_indices, i);
			if (m->indicesType == AVDL_GRAPHICS_INDICETYPE_UBYTE) {
				GLubyte *a = m->indices;
				a[i] = index[0];
			}
			else
			if (m->indicesType == AVDL_GRAPHICS_INDICETYPE_USHORT) {
				GLushort *a = m->indices;
				a[i] = index[0];
			}
		}
	}
	// old way of drawing arrays (fallback)
	else {
		m->vcount = array_vertex_indices.elements;

		m->v = malloc(sizeof(float) *m->vcount *3);
		m->dirtyVertices = 1;
		if (has_colours) {
			m->c = malloc(sizeof(float) *m->vcount *3);
		}
		if (has_texcoord) {
			m->t = malloc(sizeof(float) *m->vcount *2);
			m->tan = malloc(sizeof(float) *m->vcount *3);
			m->bitan = malloc(sizeof(float) *m->vcount *3);
		}
		if (has_normals) {
			m->n = malloc(sizeof(float) *m->vcount *3);
		}
		for (unsigned int i = 0; i < m->vcount; i++) {
			int *index = avdl_da_get(&array_vertex_indices, i);
			m->v[i*3 +0] = array_vertex_pos[index[0]*3 +0];
			m->v[i*3 +1] = array_vertex_pos[index[0]*3 +1];
			m->v[i*3 +2] = array_vertex_pos[index[0]*3 +2];

			if (has_colours) {
				#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
				m->c[i*3 +0] = array_vertex_col[index[0]*3 +0];
				m->c[i*3 +1] = array_vertex_col[index[0]*3 +1];
				m->c[i*3 +2] = array_vertex_col[index[0]*3 +2];
				#else
				m->c[i*3 +0] = dd_math_pow(array_vertex_col[index[0]*3 +0], 2.2);
				m->c[i*3 +1] = dd_math_pow(array_vertex_col[index[0]*3 +1], 2.2);
				m->c[i*3 +2] = dd_math_pow(array_vertex_col[index[0]*3 +2], 2.2);
				#endif
			}

			if (has_texcoord) {
				m->t[i*2 +0] = array_vertex_st[index[0]*2 +0];
				m->t[i*2 +1] = array_vertex_st[index[0]*2 +1];
			}

			if (has_normals) {
				m->n[i*3 +0] = array_vertex_nor[index[0]*3 +0];
				m->n[i*3 +1] = array_vertex_nor[index[0]*3 +1];
				m->n[i*3 +2] = array_vertex_nor[index[0]*3 +2];
			}
		}

		if (m->t) {
		for (int i = 0; i < m->vcount; i += 3) {

			struct avdl_vec3 deltaPos1;
			avdl_vec3_Setf(&deltaPos1,
				m->v[(i+1)*3 +0] -m->v[(i+0)*3 +0],
				m->v[(i+1)*3 +1] -m->v[(i+0)*3 +1],
				m->v[(i+1)*3 +2] -m->v[(i+0)*3 +2]
			);
			struct avdl_vec3 deltaPos2;
			avdl_vec3_Setf(&deltaPos2,
				m->v[(i+2)*3 +0] -m->v[(i+0)*3 +0],
				m->v[(i+2)*3 +1] -m->v[(i+0)*3 +1],
				m->v[(i+2)*3 +2] -m->v[(i+0)*3 +2]
			);

			struct avdl_vec3 deltaUV1;
			avdl_vec3_Setf(&deltaUV1,
				m->t[(i+1)*2 +0] -m->t[(i+0)*2 +0],
				m->t[(i+1)*2 +1] -m->t[(i+0)*2 +1],
				0
			);
			struct avdl_vec3 deltaUV2;
			avdl_vec3_Setf(&deltaUV2,
				m->t[(i+2)*2 +0] -m->t[(i+0)*2 +0],
				m->t[(i+2)*2 +1] -m->t[(i+0)*2 +1],
				0
			);

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			struct avdl_vec3 tangent;
			avdl_vec3_Setf(&tangent,
				(deltaPos1.x *deltaUV2.y -deltaPos2.x *deltaUV1.y) *r,
				(deltaPos1.y *deltaUV2.y -deltaPos2.y *deltaUV1.y) *r,
				(deltaPos1.z *deltaUV2.y -deltaPos2.z *deltaUV1.y) *r
			);
			struct avdl_vec3 bitangent;
			avdl_vec3_Setf(&bitangent,
				(deltaPos2.x *deltaUV1.x -deltaPos1.x *deltaUV2.x) *r,
				(deltaPos2.y *deltaUV1.x -deltaPos1.y *deltaUV2.x) *r,
				(deltaPos2.z *deltaUV1.x -deltaPos1.z *deltaUV2.x) *r
			);

			avdl_vec3_Normalise(&tangent);
			avdl_vec3_Normalise(&bitangent);

			// tans
			m->tan[(i+0)*3 +0] = tangent.x;
			m->tan[(i+0)*3 +1] = tangent.y;
			m->tan[(i+0)*3 +2] = tangent.z;

			m->tan[(i+1)*3 +0] = tangent.x;
			m->tan[(i+1)*3 +1] = tangent.y;
			m->tan[(i+1)*3 +2] = tangent.z;

			m->tan[(i+2)*3 +0] = tangent.x;
			m->tan[(i+2)*3 +1] = tangent.y;
			m->tan[(i+2)*3 +2] = tangent.z;

			// bitans
			m->bitan[(i+0)*3 +0] = bitangent.x;
			m->bitan[(i+0)*3 +1] = bitangent.y;
			m->bitan[(i+0)*3 +2] = bitangent.z;

			m->bitan[(i+1)*3 +0] = bitangent.x;
			m->bitan[(i+1)*3 +1] = bitangent.y;
			m->bitan[(i+1)*3 +2] = bitangent.z;

			m->bitan[(i+2)*3 +0] = bitangent.x;
			m->bitan[(i+2)*3 +1] = bitangent.y;
			m->bitan[(i+2)*3 +2] = bitangent.z;
		}
		}

		// cleanup
		if (array_vertex_pos) {
			free(array_vertex_pos);
			array_vertex_pos = 0;
		}
		if (array_vertex_col) {
			free(array_vertex_col);
			array_vertex_col = 0;
		}
		if (array_vertex_st) {
			free(array_vertex_st);
			array_vertex_st = 0;
		}
		if (array_vertex_nor) {
			free(array_vertex_nor);
			array_vertex_nor = 0;
		}
	}

	avdl_vec3_Setf(&m->boundsCenter,
		avdl_vec3_X(&boundsMin) +(avdl_vec3_X(&boundsMax) -avdl_vec3_X(&boundsMin))/2,
		avdl_vec3_Y(&boundsMin) +(avdl_vec3_Y(&boundsMax) -avdl_vec3_Y(&boundsMin))/2,
		avdl_vec3_Z(&boundsMin) +(avdl_vec3_Z(&boundsMax) -avdl_vec3_Z(&boundsMin))/2
	);
	avdl_vec3_Setf(&m->boundsExtend,
		(avdl_vec3_X(&boundsMax) -avdl_vec3_X(&boundsMin))/2,
		(avdl_vec3_Y(&boundsMax) -avdl_vec3_Y(&boundsMin))/2,
		(avdl_vec3_Z(&boundsMax) -avdl_vec3_Z(&boundsMin))/2
	);
	
	avdl_da_free(&array_vertex_indices);

	return 0;

	#endif
}

static const char *skip_whitespace(const char *str) {
	while (str[0] == ' '
	||     str[0] == '\t'
	||     str[0] == '\n'
	||     str[0] == '\r') {
		str++;
	}

	return str;
}

static const char *skip_to_whitespace(const char *str) {
	while (str[0] != ' '
	&&     str[0] != '\t'
	&&     str[0] != '\n'
	&&     str[0] != '\r'
	&&     str[0] != '\0') {
		str++;
	}

	return str;
}

static int load_gltf_internal(struct avdl_mesh_data *m, cgltf_options *options, cgltf_data *data) {

	m->vcount = 0;
	m->v = 0;
	m->c = 0;
	m->t = 0;
	m->n = 0;
	m->tan = 0;
	m->bitan = 0;
	m->boneIds = 0;
	m->weights = 0;
	m->verticesType = AVDL_GRAPHICS_VTYPE_TRIANGLES;

	if (data->meshes_count > 1) {
		avdl_log("avdl_load_gltf_internal: can only load one mesh at a time, found %d meshes", data->meshes_count);
	}
	//avdl_log("meshes: %d", data->meshes_count);
	for (int i = 0; i < data->meshes_count; i++) {
		cgltf_mesh *mesh = &data->meshes[i];
		if (mesh->primitives_count > 1) {
			avdl_log("avdl_load_gltf_internal: only supporting one mesh primitive for now, found %d", mesh->primitives_count);
		}
		for (int j = 0; j < mesh->primitives_count; j++) {
			cgltf_primitive *primitive = &mesh->primitives[j];
			unsigned int *indices = 0;
			int indices_count = 0;
			if (primitive->type != cgltf_primitive_type_triangles) {
				avdl_log("avdl_load_gltf_internal: primitives can only contain triangles for now, not %d", primitive->type);
				cgltf_free(data);
				return -1;
			}
			//avdl_log("    primitive compression: %d", primitive->has_draco_mesh_compression);
			if (primitive->indices) {
				cgltf_accessor *indice_data = primitive->indices;
				if (indice_data->type != cgltf_type_scalar) {
					avdl_log("avdl_load_gltf_internal: only supporting scalar for indices for now, not %d", indice_data->type);
					cgltf_free(data);
					return -1;
				}
				size_t attribute_value_dimension_count = cgltf_num_components(indice_data->type);
				if (attribute_value_dimension_count != 1) {
					avdl_log("avdl_load_gltf_internal: indices should only have 1 dimension, they instead have %d", attribute_value_dimension_count);
					cgltf_free(data);
					return -1;
				}
				indices_count = primitive->indices->count;
				indices = malloc(sizeof(unsigned int) *primitive->indices->count);
				m->vcount = indices_count;
				cgltf_accessor_unpack_indices(primitive->indices, indices, sizeof(unsigned int), primitive->indices->count);
			}
			else {
				avdl_log("avdl_load_gltf_internal: only supporting meshes with indices for now");
				cgltf_free(data);
				return -1;
			}
			for (int z = 0; z < primitive->attributes_count; z++) {
				cgltf_attribute *attribute = &primitive->attributes[z];
				if (attribute->data) {
					cgltf_accessor *attribute_data = attribute->data;
					size_t attribute_value_dimension_count = cgltf_num_components(attribute_data->type);
					if (attribute->type == cgltf_attribute_type_position) {
						if (attribute_data->type != cgltf_type_vec3) {
							avdl_log("avdl_load_gltf_internal: only supporting vec3 for vertex positions for now");
							cgltf_free(data);
							return -1;
						}
						if (attribute_value_dimension_count != 3) {
							avdl_log("avdl_load_gltf_internal: vertex position should only have 3 dimensions, they instead have %d", attribute_value_dimension_count);
							cgltf_free(data);
							return -1;
						}
						/* cgltf parses other formats to floats
						if (attribute_data->component_type != cgltf_component_type_r_32f) {
							avdl_log("avdl_load_gltf_internal: vertex position should only be float, it instead is %d", attribute_data->component_type);
							cgltf_free(data);
							return -1;
						}
						*/
						float *vertices = malloc(sizeof(float) *attribute_data->count *3);
						for (int ind = 0; ind < attribute_data->count; ind++) {
							cgltf_accessor_read_float(attribute_data, ind, &vertices[ind *3], 3);
						}
						if (indices) {
							m->v = malloc(sizeof(float) *m->vcount *3);
							m->dirtyVertices = 1;
							for (int ind = 0; ind < m->vcount; ind++) {
								m->v[ind*3 +0] = vertices[indices[ind]*3 +0];
								m->v[ind*3 +1] = vertices[indices[ind]*3 +1];
								m->v[ind*3 +2] = vertices[indices[ind]*3 +2];
							}
						}
						free(vertices);
					}
					else
					if (attribute->type == cgltf_attribute_type_color) {
						if (attribute_data->type != cgltf_type_vec4) {
							avdl_log("avdl_load_gltf_internal: only supporting vec4 for vertex colours for now");
							cgltf_free(data);
							return -1;
						}
						if (attribute_value_dimension_count != 4) {
							avdl_log("avdl_load_gltf_internal: vertex colours should only have 4 dimensions, they instead have %d", attribute_value_dimension_count);
							cgltf_free(data);
							return -1;
						}
						/* cgltf parses other formats to floats
						if (attribute_data->component_type != cgltf_component_type_r_32f) {
							avdl_log("avdl_load_gltf_internal: vertex colours should only be float, it instead is %d", attribute_data->component_type);
							cgltf_free(data);
							return -1;
						}
						*/
						float *colours = malloc(sizeof(float) *attribute_data->count *4);
						for (int ind = 0; ind < attribute_data->count; ind++) {
							cgltf_accessor_read_float(attribute_data, ind, &colours[ind *attribute_value_dimension_count], 4);
						}
						if (indices) {
							m->c = malloc(sizeof(float) *m->vcount *4);
							for (int ind = 0; ind < indices_count; ind++) {
								m->c[ind*3 +0] = colours[indices[ind]*4 +0];
								m->c[ind*3 +1] = colours[indices[ind]*4 +1];
								m->c[ind*3 +2] = colours[indices[ind]*4 +2];
							}
						}
						free(colours);
					}
					else
					if (attribute->type == cgltf_attribute_type_normal) {
						if (attribute_data->type != cgltf_type_vec3) {
							avdl_log("avdl_load_gltf_internal: only supporting vec3 for vertex normals for now");
							cgltf_free(data);
							return -1;
						}
						if (attribute_value_dimension_count != 3) {
							avdl_log("avdl_load_gltf_internal: vertex normals should only have 3 dimensions, they instead have %d", attribute_value_dimension_count);
							cgltf_free(data);
							return -1;
						}
						float *normals = malloc(sizeof(float) *attribute_data->count *3);
						for (int ind = 0; ind < attribute_data->count; ind++) {
							cgltf_accessor_read_float(attribute_data, ind, &normals[ind *attribute_value_dimension_count], 3);
						}
						if (indices) {
							m->n = malloc(sizeof(float) *m->vcount *3);
							for (int ind = 0; ind < indices_count; ind++) {
								m->n[ind*3 +0] = normals[indices[ind]*3 +0];
								m->n[ind*3 +1] = normals[indices[ind]*3 +1];
								m->n[ind*3 +2] = normals[indices[ind]*3 +2];
							}
						}
						free(normals);
					}
					else
					if (attribute->type == cgltf_attribute_type_texcoord) {
						if (attribute_data->type != cgltf_type_vec2) {
							avdl_log("avdl_load_gltf_internal: only supporting vec2 for texture coordinates for now");
							cgltf_free(data);
							return -1;
						}
						if (attribute_value_dimension_count != 2) {
							avdl_log("avdl_load_gltf_internal: texture coordinates should only have 2 dimensions, they instead have %d", attribute_value_dimension_count);
							cgltf_free(data);
							return -1;
						}
						/* cgltf parses other formats to floats
						if (attribute_data->component_type != cgltf_component_type_r_32f) {
							avdl_log("avdl_load_gltf_internal: texture coordinates should only be float, it instead is %d", attribute_data->component_type);
							cgltf_free(data);
							return -1;
						}
						*/
						float *texcoord = malloc(sizeof(float) *attribute_data->count *2);
						for (int ind = 0; ind < attribute_data->count; ind++) {
							cgltf_accessor_read_float(attribute_data, ind, &texcoord[ind *2], 2);
						}
						if (indices) {
							m->t = malloc(sizeof(float) *m->vcount *2);
							for (int ind = 0; ind < m->vcount; ind++) {
								m->t[ind*2 +0] = texcoord[indices[ind]*2 +0];
								m->t[ind*2 +1] = 1 -texcoord[indices[ind]*2 +1];
							}
						}
						free(texcoord);
					}
					else
					if (attribute->type == cgltf_attribute_type_joints) {
						if (attribute_data->type != cgltf_type_vec4) {
							avdl_log("avdl_load_gltf_internal: only supporting vec4 for vertex joints");
							cgltf_free(data);
							return -1;
						}
						if (attribute_value_dimension_count != 4) {
							avdl_log("avdl_load_gltf_internal: vertex joints should only have 4 dimensions, they instead have %d", attribute_value_dimension_count);
							cgltf_free(data);
							return -1;
						}
						/* cgltf parses other formats to floats
						if (attribute_data->component_type != cgltf_component_type_r_32u) {
							avdl_log("avdl_load_gltf_internal: vertex joints should only be unsigned int, it instead is %d", attribute_data->component_type);
							cgltf_free(data);
							return -1;
						}
						*/
						unsigned int *bones = malloc(sizeof(unsigned int) *attribute_data->count *4);
						for (int ind = 0; ind < attribute_data->count; ind++) {
							cgltf_accessor_read_uint(attribute_data, ind, &bones[ind *attribute_value_dimension_count], 4);
						}
						if (indices) {
							m->boneIds = malloc(sizeof(int) *m->vcount *4);
							for (int ind = 0; ind < indices_count; ind++) {
								m->boneIds[ind*4 +0] = bones[indices[ind]*4 +0];
								m->boneIds[ind*4 +1] = bones[indices[ind]*4 +1];
								m->boneIds[ind*4 +2] = bones[indices[ind]*4 +2];
								m->boneIds[ind*4 +3] = bones[indices[ind]*4 +3];
							}
						}
						free(bones);
					}
					else
					if (attribute->type == cgltf_attribute_type_weights) {
						if (attribute_data->type != cgltf_type_vec4) {
							avdl_log("avdl_load_gltf_internal: only supporting vec4 for vertex weights");
							cgltf_free(data);
							return -1;
						}
						if (attribute_value_dimension_count != 4) {
							avdl_log("avdl_load_gltf_internal: vertex weights should only have 4 dimensions, they instead have %d", attribute_value_dimension_count);
							cgltf_free(data);
							return -1;
						}
						/* cgltf parses other formats to floats
						if (attribute_data->component_type != cgltf_component_type_r_32f) {
							avdl_log("avdl_load_gltf_internal: vertex weights should only be float, it instead is %d", attribute_data->component_type);
							cgltf_free(data);
							return -1;
						}
						*/
						float *weights = malloc(sizeof(float) *attribute_data->count *4);
						for (int ind = 0; ind < attribute_data->count; ind++) {
							cgltf_accessor_read_float(attribute_data, ind, &weights[ind *attribute_value_dimension_count], 4);
						}
						if (indices) {
							m->weights = malloc(sizeof(float) *m->vcount *4);
							for (int ind = 0; ind < indices_count; ind++) {
								m->weights[ind*4 +0] = weights[indices[ind]*4 +0];
								m->weights[ind*4 +1] = weights[indices[ind]*4 +1];
								m->weights[ind*4 +2] = weights[indices[ind]*4 +2];
								m->weights[ind*4 +3] = weights[indices[ind]*4 +3];
							}
						}
						free(weights);
					}
					else {
						//avdl_log("avdl: avdl_load_gltf_internal: vertex attribute not handled: %s", attribute->name);
					}
				}
			}
			if (indices) {
				free(indices);
			}
			break; // only one mesh primitive for now
		}
		break; // only one mesh for now
	}
	cgltf_node **joints = 0;
	if (data->skins_count > 1) {
		avdl_log("avdl_load_gltf_internal: only supporting one skin at a time for now");
	}
	for (int i = 0; i < data->skins_count; i++) {
		cgltf_skin *skin = &data->skins[i];
		//avdl_log("	skin name: %s", skin->name);
		//avdl_log("	skin joints: %d", skin->joints_count);
		if (skin->joints_count < 1) {
			avdl_log("avdl_load_gltf_internal: skin should have at least one joint");
			cgltf_free(data);
			return -1;
		}
		joints = skin->joints;
		m->boneCount = skin->joints_count;
		m->inverseBindMatrices = malloc(sizeof(struct dd_matrix) *skin->joints_count);
		m->rootIndex = -1;
		m->children_indices = malloc(sizeof(int *) *skin->joints_count);
		m->children_indices_count = malloc(sizeof(int) *skin->joints_count);
		dd_matrix_create(&m->rootMatrix);
		dd_matrix_identity(&m->rootMatrix);
		for (int j = 0; j < skin->joints_count; j++) {
			struct dd_matrix *inverse_matrix = &m->inverseBindMatrices[j];
			dd_matrix_create(inverse_matrix);
			dd_matrix_identity(inverse_matrix);
			cgltf_node *joint = skin->joints[j];
			//avdl_log("	    joint i: %d", j);
			//avdl_log("	    joint name: %s", joint->name);
			m->children_indices[j] = 0;
			m->children_indices_count[j] = 0;
			if (joint->children && joint->children_count > 0) {
				//avdl_log("	    joint has children: %d", joint->children_count);
				m->children_indices_count[j] = joint->children_count;
				m->children_indices[j] = malloc(sizeof(int) *joint->children_count);
				for (int zz = 0; zz < joint->children_count; zz++) {
					m->children_indices[j][zz] = -1;
					for (int z = 0; z < skin->joints_count; z++) {
						if (skin->joints[z] == joint->children[zz]) {
							m->children_indices[j][zz] = z;
							break;
						}
					}
					if (m->children_indices[j][zz] != -1) {
						//avdl_log("	    joint child: %d", m->children_indices[j][zz]);
					}
					else {
						avdl_log("avdl_load_gltf_internal: joint child: COULD NOT FIND CHILD INDEX");
						cgltf_free(data);
						return -1;
					}
				}
			}
			if (joint->parent) {
				//avdl_log("	    joint parent: %s", joint->parent->name);
				int isRoot = 1;
				for (int z = 0; z < skin->joints_count; z++) {
					if (skin->joints[z] == joint->parent) {
						isRoot = 0;
						break;
					}
				}
				if (isRoot) {
					//avdl_log("	    joint root: IS ROOT");
					if (m->rootIndex != -1) {
						avdl_log("avdl_load_gltf_internal: multiple root joints detected - confused");
						cgltf_free(data);
						return -1;
					}
					m->rootIndex = j;
					cgltf_node *parentJoint = joint->parent;
					//avdl_log("parent node name: %s", parentJoint->name);
					if (parentJoint->has_translation) {
						/*
						avdl_log("	    parentJoint translation: %f %f %f",
							parentJoint->translation[0],
							parentJoint->translation[1],
							parentJoint->translation[2]
						);
						*/
						dd_matrix_translate(&m->rootMatrix, parentJoint->translation[0], parentJoint->translation[1], parentJoint->translation[2]);
					}
					else {
						//avdl_log("	    parentJoint translation: no translation");
					}
					if (parentJoint->has_rotation) {
						/*
						avdl_log("	    parentJoint rotation: %f %f %f %f",
							parentJoint->rotation[0],
							parentJoint->rotation[1],
							parentJoint->rotation[2],
							parentJoint->rotation[3]
						);
						*/
						struct dd_matrix mat;
						dd_matrix_quaternion_to_rotation_matrix(parentJoint->rotation, &mat);
						dd_matrix_mult(&m->rootMatrix, &mat);
					}
					else {
						//avdl_log("	    parentJoint rotation: no rotation");
					}
					if (parentJoint->has_scale) {
						/*
						avdl_log("	    parentJoint scale: %f %f %f",
							parentJoint->scale[0],
							parentJoint->scale[1],
							parentJoint->scale[2]
						);
						*/
						dd_matrix_scale(&m->rootMatrix, parentJoint->scale[0], parentJoint->scale[1], parentJoint->scale[2]);
					}
					else {
						//avdl_log("	    parentJoint scale: no scale");
					}
					/*
					*/
					if (parentJoint->has_matrix) {
						/*
						avdl_log("	    parentJoint matrix:\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f",
							parentJoint->matrix[0],
							parentJoint->matrix[1],
							parentJoint->matrix[2],
							parentJoint->matrix[3],

							parentJoint->matrix[4],
							parentJoint->matrix[5],
							parentJoint->matrix[6],
							parentJoint->matrix[7],

							parentJoint->matrix[8],
							parentJoint->matrix[9],
							parentJoint->matrix[10],
							parentJoint->matrix[11],

							parentJoint->matrix[12],
							parentJoint->matrix[13],
							parentJoint->matrix[14],
							parentJoint->matrix[15]
						);
						*/
					}
				}
				/*
				else {
					avdl_log("	    joint root: NOT ROOT");
				}
				*/
			}
			else {
				// doesn't happen
				//avdl_log("	    joint parent: NO PARENT");
			}
			if (joint->has_translation) {
				/*
				avdl_log("	    joint translation: %f %f %f",
					joint->translation[0],
					joint->translation[1],
					joint->translation[2]
				);
				*/
			}
			else {
				//avdl_log("	    joint translation: no translation");
			}
			if (joint->has_rotation) {
				/*
				avdl_log("	    joint rotation: %f %f %f %f",
					joint->rotation[0],
					joint->rotation[1],
					joint->rotation[2],
					joint->rotation[3]
				);
				*/
			}
			else {
				//avdl_log("	    joint rotation: no rotation");
			}
			if (joint->has_scale) {
				/*
				avdl_log("	    joint scale: %f %f %f",
					joint->scale[0],
					joint->scale[1],
					joint->scale[2]
				);
				*/
			}
			else {
				//avdl_log("	    joint scale: no scale");
			}
			if (joint->has_matrix) {
				/*
				avdl_log("	    joint matrix:\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f",
					joint->matrix[0],
					joint->matrix[1],
					joint->matrix[2],
					joint->matrix[3],

					joint->matrix[4],
					joint->matrix[5],
					joint->matrix[6],
					joint->matrix[7],

					joint->matrix[8],
					joint->matrix[9],
					joint->matrix[10],
					joint->matrix[11],

					joint->matrix[12],
					joint->matrix[13],
					joint->matrix[14],
					joint->matrix[15]
				);
				*/
			}
			/*
			else {
				avdl_log("	    joint matrix: no matrix");
			}
			*/
			if (skin->inverse_bind_matrices) {
				cgltf_accessor *inverse = skin->inverse_bind_matrices;
				size_t attribute_value_dimension_count = cgltf_num_components(inverse->type);
				//avdl_log("	    joint inverse bind matrix components: %d", attribute_value_dimension_count);
				//avdl_log("	    joint inverse bind matrix count: %d", inverse->count);
				if (inverse->type != cgltf_type_mat4) {
					avdl_log("avdl_load_gltf_internal: inverse bind matrix can only be mat4");
					cgltf_free(data);
					return -1;
				}
				if (attribute_value_dimension_count != 16) {
					avdl_log("avdl_load_gltf_internal: inverse bind matrix can have 16 components, found %d", attribute_value_dimension_count);
					cgltf_free(data);
					return -1;
				}
				cgltf_accessor_read_float(inverse, j, inverse_matrix, 16);
				/*
				avdl_log("	    joint inverse bind matrix:\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f",
					inverse_matrix->cell[0],
					inverse_matrix->cell[1],
					inverse_matrix->cell[2],
					inverse_matrix->cell[3],

					inverse_matrix->cell[4],
					inverse_matrix->cell[5],
					inverse_matrix->cell[6],
					inverse_matrix->cell[7],

					inverse_matrix->cell[8],
					inverse_matrix->cell[9],
					inverse_matrix->cell[10],
					inverse_matrix->cell[11],

					inverse_matrix->cell[12],
					inverse_matrix->cell[13],
					inverse_matrix->cell[14],
					inverse_matrix->cell[15]
				);
				*/
			}
			else {
				// only does local transform
				// inverse_matrix = parent transform + matrix, then inverse
				//dd_matrix_copy(inverse_matrix, matrix);
				//dd_matrix_inverse(inverse_matrix);
				avdl_log("avdl_load_gltf_internal: does not support files without inverse matrix for now");
			}
			//avdl_log("matrix:");
			//dd_matrix_print(matrix);
			//dd_matrix_print(inverse_matrix);
		}
		/*
		if (skin->inverse_bind_matrices) {
			cgltf_accessor *accessor = skin->inverse_bind_matrices;
			avdl_log("	skin inverse bind matrices: %d", accessor->count);
			//float *matrices = malloc(sizeof(float) *16 *skin->inverse_bind_matrices);
			size_t attribute_value_dimension_count = cgltf_num_components(accessor->type);
			//cgltf_accessor_read_float(accessor, ind, &weights[ind *attribute_value_dimension_count], attribute_value_dimension_count);
			avdl_log("        num of components: %d", attribute_value_dimension_count);
		}
		*/
		/*
		if (skin->skeleton) {
			avdl_log("	skin skeleton: %s", skin->skeleton->name);
		}
		else {
			avdl_log("	skin skeleton: NO SKELETON");
		}
		*/
		break;
	}

	if (data->animations_count == 0) {
		avdl_log("avdl_load_gltf_internal: not sure how to handle file without animations");
		cgltf_free(data);
		return -1;
	}
	// animations
	m->animationsCount = data->animations_count;
	m->animations = malloc(sizeof(struct dd_animation) *data->animations_count);
	for (int i = 0; i < data->animations_count; i++) {
		cgltf_animation *animation = &data->animations[i];
		struct dd_animation *anim = &m->animations[i];

		// animation name
		anim->name = malloc(sizeof(char) *strlen(animation->name) +1);
		strcpy(anim->name, animation->name);

		// animation bones
		anim->animatedBones = malloc(sizeof(struct dd_animated_bone) *m->boneCount);
		anim->animatedBonesCount = m->boneCount;
		for (int j = 0; j < m->boneCount; j++) {
			struct dd_animated_bone *animBone = &anim->animatedBones[j];
			avdl_da_init(&animBone->keyframes_position, sizeof(struct dd_keyframe_vec3));
			avdl_da_init(&animBone->keyframes_rotation, sizeof(struct dd_keyframe_vec4));
			avdl_da_init(&animBone->keyframes_scale, sizeof(struct dd_keyframe_vec3));
		}

		if (animation->channels_count <= 0) {
			avdl_log("avdl_load_gltf_internal: animation has no channel, not supported");
			cgltf_free(data);
			return -1;
		}
		for (int j = 0; j < animation->channels_count; j++) {
			cgltf_animation_channel *channel = &animation->channels[j];

			// find target bone
			struct dd_animated_bone *animBone = 0;
			for (int node_index = 0; node_index < m->boneCount; node_index++) {
				if (joints[node_index] == channel->target_node) {
					animBone = &anim->animatedBones[node_index];
					break;
				}
			}

			// didn't find target
			if (!animBone) {
				avdl_log("avdl_load_gltf_internal: bone targeted by channel cannot be found");
				cgltf_free(data);
				return -1;
			}

			if (channel->target_path == cgltf_animation_path_type_translation) {

				if (!channel->sampler) {
					avdl_log("avdl_load_gltf_internal: channel without sampler - not supported");
					cgltf_free(data);
					return -1;
				}

				// add new position keyframe into `animBone`
				cgltf_animation_sampler *sampler = channel->sampler;
				cgltf_accessor* input = sampler->input;
				cgltf_accessor* output = sampler->output;
				size_t input_dimensions = cgltf_num_components(input->type);
				if (input_dimensions != 1) {
					avdl_log("avdl_load_gltf_internal: channel input position should have 1 dimension, it instead has %d", input_dimensions);
					cgltf_free(data);
					return -1;
				}
				size_t output_dimensions = cgltf_num_components(output->type);
				if (output_dimensions != 3) {
					avdl_log("avdl_load_gltf_internal: channel output position should have 3 dimensions, it instead has %d", output_dimensions);
					cgltf_free(data);
					return -1;
				}
				if (input->count != output->count) {
					avdl_log("avdl_load_gltf_internal: channel input and output position should have the same number of values %d/%d", input->count, output->count);
					cgltf_free(data);
					return -1;
				}
				for (int z = 0; z < input->count; z++) {
					struct dd_keyframe_vec3 keyframe;
					cgltf_accessor_read_float(input, z, &keyframe.time, 1);
					cgltf_accessor_read_float(output, z, &keyframe.value, 3);
					avdl_da_push(&animBone->keyframes_position, &keyframe);
				}
			}
			else
			if (channel->target_path == cgltf_animation_path_type_rotation) {

				if (!channel->sampler) {
					avdl_log("avdl_load_gltf_internal: channel without sampler - not supported");
					cgltf_free(data);
					return -1;
				}

				// add new rotation keyframe into `animBone`
				cgltf_animation_sampler *sampler = channel->sampler;
				cgltf_accessor* input = sampler->input;
				cgltf_accessor* output = sampler->output;
				size_t input_dimensions = cgltf_num_components(input->type);
				if (input_dimensions != 1) {
					avdl_log("avdl_load_gltf_internal: channel input rotation should have 1 dimension, it instead has %d", input_dimensions);
					cgltf_free(data);
					return -1;
				}
				size_t output_dimensions = cgltf_num_components(output->type);
				if (output_dimensions != 4) {
					avdl_log("avdl_load_gltf_internal: channel output rotation should have 4 dimensions, it instead has %d", output_dimensions);
					cgltf_free(data);
					return -1;
				}
				if (input->count != output->count) {
					avdl_log("avdl_load_gltf_internal: channel input and output rotation should have the same number of values %d/%d", input->count, output->count);
					cgltf_free(data);
					return -1;
				}
				for (int z = 0; z < input->count; z++) {
					struct dd_keyframe_vec4 keyframe;
					cgltf_accessor_read_float(input, z, &keyframe.time, 1);
					cgltf_accessor_read_float(output, z, &keyframe.value, 4);
					avdl_da_push(&animBone->keyframes_rotation, &keyframe);
				}

			}
			else
			if (channel->target_path == cgltf_animation_path_type_scale) {
				//avdl_log("avdl_load_gltf_internal: unsupported animation type: scale");
			}
			else
			if (channel->target_path == cgltf_animation_path_type_weights) {
				//avdl_log("avdl_load_gltf_internal: unsupported animation type: weights");
			}
			else
			if (channel->target_path == cgltf_animation_path_type_max_enum) {
				//avdl_log("avdl_load_gltf_internal: unsupported animation type: max enum");
			}
			else
			if (channel->target_path == cgltf_animation_path_type_invalid) {
				//avdl_log("avdl_load_gltf_internal: invalid animation type");
			}
		}
	}
	return 0;
}

int avdl_mesh_SetCustomData(struct avdl_mesh *m, int vcount, float *pos, float *col, float *tex) {

	if (m->data) {
		avdl_log("avdl_mesh_SetCustomData: Currently not supporting setting custom data when other data is available");
		return -1;
	}

	m->data = CreateMeshData();

	m->data->vcount = vcount;
	m->data->v = pos;
	m->data->dirtyVertices = 1;
	m->data->c = col;
	m->data->t = tex;

	m->data->verticesType = AVDL_GRAPHICS_VTYPE_TRIANGLES;
	return 0;
}
