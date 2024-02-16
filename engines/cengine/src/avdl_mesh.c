#include <stdio.h>
#include <stdlib.h>
#include "avdl_mesh.h"
#include "dd_filetomesh.h"
#include <string.h>
#include "avdl_assetManager.h"
#include "dd_log.h"
#include <stdlib.h>
#include "avdl_graphics.h"
#include "dd_math.h"

#ifndef AVDL_DIRECT3D11
extern GLuint defaultProgram;
extern GLuint currentProgram;
#endif

static float shape_triangle[] = {
	0, 0.5, 0,
	-0.5, -0.5, 0,
	0.5, -0.5, 0,
};

static float shape_rectangle[] = {
	-0.5, 0.5, 0,
	-0.5, -0.5, 0,
	0.5, -0.5, 0,

	0.5, -0.5, 0,
	0.5, 0.5, 0,
	-0.5, 0.5, 0,
};

static float shape_box[] = {
	// front side
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,

	0.5, -0.5, 0.5,
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,

	// back side
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,

	0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,

	// top side
	-0.5,  0.5, -0.5,
	-0.5,  0.5,  0.5,
	 0.5,  0.5, -0.5,

	 0.5,  0.5, -0.5,
	-0.5,  0.5,  0.5,
	 0.5,  0.5,  0.5,

	// bottom side
	-0.5, -0.5, -0.5,
	 0.5, -0.5, -0.5,
	-0.5, -0.5,  0.5,

	 0.5, -0.5, -0.5,
	 0.5, -0.5,  0.5,
	-0.5, -0.5,  0.5,

	// left side
	-0.5, -0.5, -0.5,
	-0.5, -0.5,  0.5,
	-0.5,  0.5, -0.5,

	-0.5,  0.5, -0.5,
	-0.5, -0.5,  0.5,
	-0.5,  0.5,  0.5,

	// right side
	 0.5, -0.5, -0.5,
	 0.5,  0.5, -0.5,
	 0.5, -0.5,  0.5,

	 0.5,  0.5, -0.5,
	 0.5,  0.5,  0.5,
	 0.5, -0.5,  0.5,
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
	if (m->bitan && m->dirtyTan) {
		free(m->bitan);
		m->dirtyTan = 0;
	}
	m->bitan = 0;
}

// constructor
void avdl_mesh_create(struct avdl_mesh *m) {

	// num of vertices
	m->vcount = 0;

	// vertex attributes
	m->v = 0;
	m->dirtyVertices = 0;
	m->c = 0;
	m->dirtyColours = 0;
	m->n = 0;
	m->dirtyNormals = 0;

	// bump map
	m->tan = 0;
	m->dirtyTan = 0;
	m->bitan = 0;
	m->dirtyBitan = 0;

	// graphics context
	m->graphicsContextId = -1;

	// draw solid or wireframe
	m->draw_type = 0;

	// array
	m->verticesCol = 0;
	m->dirtyColourArrayObject = 0;

	// textures
	m->dirtyTextures = 0;
	m->t = 0;
	m->img = 0;
	m->img_normal = 0;
	m->hasTransparency = 0;

	m->draw = avdl_mesh_draw;
	m->clean = avdl_mesh_clean;
	m->set_primitive = avdl_mesh_set_primitive;
	m->load = avdl_mesh_load;
	m->copy = avdl_mesh_copy;

	m->set_colour = avdl_mesh_set_colour;

	#if !defined( AVDL_DIRECT3D11 )
	m->buffer = 0;
	m->array = 0;
	#endif

	m->set_primitive_texcoords = avdl_mesh_set_primitive_texcoords;
	m->setTexture = avdl_mesh_setTexture;
	m->setTextureNormal = avdl_mesh_setTextureNormal;
	m->hasTexture = avdl_mesh_hasTexture;
	m->setTransparency = avdl_mesh_setTransparency;

	m->setWireframe = avdl_mesh_setWireframe;
	m->setSolid = avdl_mesh_setSolid;

	m->vertexBuffer = 0;
}

void avdl_mesh_set_primitive(struct avdl_mesh *m, enum avdl_primitives shape) {

	// clean previous vertices
	clean_position(m);

	// set mesh shape based on given value
	switch (shape) {
		case AVDL_PRIMITIVE_TRIANGLE:
			m->v = shape_triangle;
			m->vcount = sizeof(shape_triangle) /sizeof(float) /3;
			break;

		case AVDL_PRIMITIVE_RECTANGLE:
			m->v = shape_rectangle;
			m->vcount = sizeof(shape_rectangle) /sizeof(float) /3;

			// texture coordinates
			m->t = malloc(sizeof(float) *6 *2);
			m->dirtyTextures = 1;
			for (int i = 0; i < 6; i++) {
				m->t[i*2+0] = m->v[i*3+0] +0.5;
				m->t[i*2+1] = m->v[i*3+1] +0.5;
			}
			break;

		case AVDL_PRIMITIVE_BOX:
			m->v = shape_box;
			m->vcount = sizeof(shape_box) /sizeof(float) /3;
			break;

		case AVDL_PRIMITIVE_BOX_FLIP:
			m->v = shape_box_flipped;
			m->vcount = sizeof(shape_box_flipped) /sizeof(float) /3;
			break;
	}

}

/* Free mesh from allocated memory
 * the mesh is left in an undefined state.
 * It should either get a new state with a
 * load function or not used anymore.
 */
void avdl_mesh_clean(struct avdl_mesh *m) {
	clean_position(m);
	clean_colour(m);
	clean_textures(m);
	clean_normals(m);
	clean_tan(m);
	clean_bitan(m);

	#if !defined( AVDL_DIRECT3D11 )
	if (m->array) {
		glDeleteVertexArrays(1, &m->array);
		glDeleteBuffers(1, &m->buffer);
		m->array = 0;
		m->buffer = 0;
	}
	#endif

	if (m->dirtyColourArrayObject) {
		free(m->verticesCol);
		m->verticesCol = 0;
		m->dirtyColourArrayObject = 0;
	}

}

extern struct dd_matrix matPerspective;
extern struct dd_matrix matView;
extern struct dd_matrix matModel[];
extern int matModel_index;

/* draw the mesh itself
 */
void avdl_mesh_draw(struct avdl_mesh *m) {
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
	#else

	if (!m->v) {
		return;
	}

	if (m->array == 0 || m->graphicsContextId != avdl_graphics_getContextId()) {

		// keep graphics context up to date
                m->graphicsContextId = avdl_graphics_getContextId();

		size_t totalSize = 0;

		// vertex positions
		size_t posOffset = 0;
		size_t posSize = sizeof(float) *3 *m->vcount;
		totalSize += posSize;

		// vertex colours
		size_t colOffset = posOffset +posSize;
		size_t colSize = 0;
		if (m->c) {
			//#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
			//colSize = sizeof(float) *4 *m->vcount;
			//#else
			colSize = sizeof(float) *3 *m->vcount;
			//#endif
		}
		totalSize += colSize;

		// texture coordinates
		size_t texOffset = colOffset +colSize;
		size_t texSize = 0;
		if (m->t) {
			texSize = sizeof(float) *2 *m->vcount;
		}
		totalSize += texSize;

		// normals
		size_t norOffset = texOffset +texSize;
		size_t norSize = 0;
		if (m->n) {
			norSize = sizeof(float) *3 *m->vcount;
		}
		totalSize += norSize;

		// tan
		size_t tanOffset = norOffset +norSize;
		size_t tanSize = 0;
		if (m->tan) {
			tanSize = sizeof(float) *3 *m->vcount;
		}
		totalSize += tanSize;

		// bitan
		size_t bitanOffset = tanOffset +tanSize;
		size_t bitanSize = 0;
		if (m->bitan) {
			bitanSize = sizeof(float) *3 *m->vcount;
		}
		totalSize += bitanSize;

		// create array as one unit
		m->verticesCol = malloc( totalSize );
		m->dirtyColourArrayObject = 1;
		memcpy(((char *)m->verticesCol) +posOffset, m->v, posSize);
		if (m->c) {
			memcpy(((char *)m->verticesCol) +colOffset, m->c, colSize);
		}
		if (m->t) {
			memcpy(((char *)m->verticesCol) +texOffset, m->t, texSize);
		}
		if (m->n) {
			memcpy(((char *)m->verticesCol) +norOffset, m->n, norSize);
		}
		if (m->tan) {
			memcpy(((char *)m->verticesCol) +tanOffset, m->tan, tanSize);
		}
		if (m->bitan) {
			memcpy(((char *)m->verticesCol) +bitanOffset, m->bitan, bitanSize);
		}

		// generate array object
		GL(glGenVertexArrays(1, &m->array));
		GL(glBindVertexArray(m->array));
	
		// generate buffer attached to array
		GL(glGenBuffers(1, &m->buffer));
		GL(glBindBuffer(GL_ARRAY_BUFFER, m->buffer));

		// give data to buffer
		GL(glBufferData(GL_ARRAY_BUFFER, totalSize, m->verticesCol, GL_STATIC_DRAW));
	
		// attach vertex positions to current program
		int pos = glGetAttribLocation(currentProgram, "position");
		// program has `position`
		if (pos != -1) {
			GL(glVertexAttribPointer(pos, 3, GL_FLOAT, 0, 0, posOffset));
			GL(glEnableVertexAttribArray(pos));
		}

		// attach vertex colours
		if (m->c) {
			int col = glGetAttribLocation(currentProgram, "colour");
			// program has colours
			if (col != -1) {
				GL(glVertexAttribPointer(col, 3, GL_FLOAT, 0, 0, colOffset));
				GL(glEnableVertexAttribArray(col));
			}
		}

		// attach texture coordinates
		if (m->t) {
			int tex = glGetAttribLocation(currentProgram, "texCoord");
			// program has texCoord
			if (tex != -1) {
				GL(glVertexAttribPointer(tex, 2, GL_FLOAT, 0, 0, texOffset));
				GL(glEnableVertexAttribArray(tex));
			}
		}

		// attach normal
		if (m->n) {
			int nor = glGetAttribLocation(currentProgram, "normal");
			// program has normal
			if (nor != -1) {
				GL(glVertexAttribPointer(nor, 3, GL_FLOAT, 0, 0, norOffset));
				GL(glEnableVertexAttribArray(nor));
			}
		}

		// attach tan
		if (m->tan) {
			int tanLoc = glGetAttribLocation(currentProgram, "tangent");
			// program has tan
			if (tanLoc != -1) {
				GL(glVertexAttribPointer(tanLoc, 3, GL_FLOAT, 0, 0, tanOffset));
				GL(glEnableVertexAttribArray(tanLoc));
			}
		}

		// attach bitan
		if (m->bitan) {
			int bitanLoc = glGetAttribLocation(currentProgram, "bitangent");
			// program has bitan
			if (bitanLoc != -1) {
				GL(glVertexAttribPointer(bitanLoc, 3, GL_FLOAT, 0, 0, bitanOffset));
				GL(glEnableVertexAttribArray(bitanLoc));
			}
		}
	}

	if (m->hasTransparency) {
		avdl_graphics_EnableBlend();
	}

	if (m->img) {
		m->img->bindIndex(m->img, 0);
		GLuint loc = glGetUniformLocation(currentProgram, "image");
		if (loc != -1) {
			GL(glUniform1i(loc, 0));
		}
	}
	if (m->img_normal) {
		m->img_normal->bindIndex(m->img_normal, 1);
		GLuint loc = glGetUniformLocation(currentProgram, "image_normal");
		if (loc != -1) {
			GL(glUniform1i(loc, 1));
		}
	}

	GL(glBindVertexArray(m->array));

	#if defined(AVDL_QUEST2)
	int MatrixID = avdl_graphics_GetUniformLocation(currentProgram, "matrix");
	if (MatrixID < 0) {
		//dd_log("avdl: avdl_mesh: location of `matrix` not found in current program");
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
		//dd_log("avdl: avdl_mesh: location of `matrix` not found in current program");
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

	// draw arrays
	if (m->draw_type) {
		// not possible on OpenGL ES
		//GL(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
		GL(glDrawArrays(GL_LINES, 0, m->vcount));
	}
	else {
		GL(glDrawArrays(GL_TRIANGLES, 0, m->vcount));
	}
	GL(glBindVertexArray(0));

	if (m->img) {
		m->img->unbind(m->img);
	}

	if (m->img_normal) {
		m->img_normal->unbind(m->img_normal);
	}

	if (m->hasTransparency) {
		avdl_graphics_DisableBlend();
	}
	#endif
}

/*
 * add the mesh to be loaded from the asset manager
 */
void avdl_mesh_load(struct avdl_mesh *m, const char *asset, int type) {

	// clean the mesh, if was dirty
	avdl_mesh_clean(m);

	// mark to be loaded
	avdl_assetManager_add(m, AVDL_ASSETMANAGER_MESH2, asset, type);

}

void avdl_mesh_copy(struct avdl_mesh *dest, struct avdl_mesh *src) {
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
}

void avdl_mesh_combine(struct avdl_mesh *dst, struct avdl_mesh *src, float offsetX, float offsetY, float offsetZ) {
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
}

void avdl_mesh_translatef(struct avdl_mesh *o, float x, float y, float z) {
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
}

void avdl_mesh_scalef(struct avdl_mesh *o, float x, float y, float z) {
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
}

void avdl_mesh_set_colour(struct avdl_mesh *m, float r, float g, float b) {
	clean_colour(m);
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
	m->c = malloc(m->vcount *sizeof(float) *3);
	m->dirtyColours = 1;
	for (int i = 0; i < m->vcount *3; i += 3) {
		m->c[i+0] = r;
		m->c[i+1] = g;
		m->c[i+2] = b;
	}
//	#endif
}

void avdl_mesh_set_primitive_texcoords(struct avdl_mesh *m, float offsetX, float offsetY, float sizeX, float sizeY) {
	for (int i = 0; i < m->vcount*2; i += 2) {
		m->t[i+0] *= sizeX;
		m->t[i+0] += offsetX;

		m->t[i+1] *= sizeY;
		m->t[i+1] += offsetY;
	}
}

void avdl_mesh_setTransparency(struct avdl_mesh *o, int transparency) {
	o->hasTransparency = transparency;
}

void avdl_mesh_setTexture(struct avdl_mesh *o, struct dd_image *tex) {
	o->img = tex;
}

void avdl_mesh_setTextureNormal(struct avdl_mesh *o, struct dd_image *tex) {
	o->img_normal = tex;
}

int avdl_mesh_hasTexture(struct avdl_mesh *o) {
	return o->img != 0;
}

void avdl_mesh_setWireframe(struct avdl_mesh *o) {
	o->draw_type = 1;
}

void avdl_mesh_setSolid(struct avdl_mesh *o) {
	o->draw_type = 0;
}
