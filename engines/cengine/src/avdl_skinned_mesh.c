#include "avdl_skinned_mesh.h"
#include "dd_log.h"

#ifndef AVDL_DIRECT3D11
extern GLuint defaultProgram;
extern GLuint currentProgram;
#endif

static void clean_boneIds(struct avdl_skinned_mesh *m) {
	if (m->boneIds && m->dirtyBoneIds) {
		free(m->boneIds);
		m->dirtyBoneIds = 0;
	}
	m->boneIds = 0;
}
static void clean_weights(struct avdl_skinned_mesh *m) {
	if (m->weights && m->dirtyWeights) {
		free(m->weights);
		m->dirtyWeights = 0;
	}
	m->weights = 0;
}

// constructor
void avdl_skinned_mesh_create(struct avdl_skinned_mesh *m) {

	avdl_mesh_create(&m->parent);

	m->parent.LoadFromLoadedMesh = avdl_skinned_mesh_LoadFromLoadedMesh;
	m->parent.draw = avdl_skinned_mesh_draw;

	// skeleton
	m->boneIds = 0;
	m->dirtyBoneIds = 0;
	m->weights = 0;
	m->dirtyWeights = 0;

	// animations
	m->update = avdl_skinned_mesh_update;
	m->PlayAnimation = avdl_skinned_mesh_PlayAnimation;
	m->SetOnAnimationDone = avdl_skinned_mesh_SetOnAnimationDone;

	avdl_skeleton_create(&m->skeleton);
}

/* Free mesh from allocated memory
 * the mesh is left in an undefined state.
 * It should either get a new state with a
 * load function or not used anymore.
 */
void avdl_skinned_mesh_clean(struct avdl_skinned_mesh *m) {
	avdl_mesh_clean(m);
	clean_boneIds(m);
	clean_weights(m);
	avdl_skeleton_clean(&m->skeleton);
}

extern struct dd_matrix matPerspective;
extern struct dd_matrix matView;
extern struct dd_matrix matModel[];
extern int matModel_index;

/* draw the mesh itself
 */
void avdl_skinned_mesh_draw(struct avdl_skinned_mesh *m) {
	#ifdef AVDL_DIRECT3D11
	#else
	if (!m->parent.v) {
		return;
	}

	if (m->parent.array == 0 || m->parent.graphicsContextId != avdl_graphics_getContextId()) {

		// keep graphics context up to date
                m->parent.graphicsContextId = avdl_graphics_getContextId();

		size_t totalSize = 0;

		// vertex positions
		size_t posOffset = 0;
		size_t posSize = sizeof(float) *3 *m->parent.vcount;
		totalSize += posSize;

		// vertex colours
		size_t colOffset = posOffset +posSize;
		size_t colSize = 0;
		if (m->parent.c) {
			//#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
			//colSize = sizeof(float) *4 *m->vcount;
			//#else
			colSize = sizeof(float) *3 *m->parent.vcount;
			//#endif
		}
		totalSize += colSize;

		// texture coordinates
		size_t texOffset = colOffset +colSize;
		size_t texSize = 0;
		if (m->parent.t) {
			texSize = sizeof(float) *2 *m->parent.vcount;
		}
		totalSize += texSize;

		// normals
		size_t norOffset = texOffset +texSize;
		size_t norSize = 0;
		if (m->parent.n) {
			norSize = sizeof(float) *3 *m->parent.vcount;
		}
		totalSize += norSize;

		// tan
		size_t tanOffset = norOffset +norSize;
		size_t tanSize = 0;
		if (m->parent.tan) {
			tanSize = sizeof(float) *3 *m->parent.vcount;
		}
		totalSize += tanSize;

		// bitan
		size_t bitanOffset = tanOffset +tanSize;
		size_t bitanSize = 0;
		if (m->parent.bitan) {
			bitanSize = sizeof(float) *3 *m->parent.vcount;
		}
		totalSize += bitanSize;

		// bone ids
		size_t boneIdsOffset = bitanOffset +bitanSize;
		size_t boneIdsSize = 0;
		if (m->boneIds) {
			boneIdsSize = sizeof(int) *4 *m->parent.vcount;
		}
		totalSize += boneIdsSize;

		// weights
		size_t weightsOffset = boneIdsOffset +boneIdsSize;
		size_t weightsSize = 0;
		if (m->weights) {
			weightsSize = sizeof(float) *4 *m->parent.vcount;
		}
		totalSize += weightsSize;

		// create array as one unit
		m->parent.verticesCol = malloc( totalSize );
		m->parent.dirtyColourArrayObject = 1;
		memcpy(((char *)m->parent.verticesCol) +posOffset, m->parent.v, posSize);
		if (m->parent.c) {
			memcpy(((char *)m->parent.verticesCol) +colOffset, m->parent.c, colSize);
		}
		if (m->parent.t) {
			memcpy(((char *)m->parent.verticesCol) +texOffset, m->parent.t, texSize);
		}
		if (m->parent.n) {
			memcpy(((char *)m->parent.verticesCol) +norOffset, m->parent.n, norSize);
		}
		if (m->parent.tan) {
			memcpy(((char *)m->parent.verticesCol) +tanOffset, m->parent.tan, tanSize);
		}
		if (m->parent.bitan) {
			memcpy(((char *)m->parent.verticesCol) +bitanOffset, m->parent.bitan, bitanSize);
		}
		if (m->boneIds) {
			memcpy(((char *)m->parent.verticesCol) +boneIdsOffset, m->boneIds, boneIdsSize);
		}
		if (m->weights) {
			memcpy(((char *)m->parent.verticesCol) +weightsOffset, m->weights, weightsSize);
		}

		// generate array object
		GL(glGenVertexArrays(1, &m->parent.array));
		GL(glBindVertexArray(m->parent.array));
	
		// generate buffer attached to array
		GL(glGenBuffers(1, &m->parent.buffer));
		GL(glBindBuffer(GL_ARRAY_BUFFER, m->parent.buffer));

		// give data to buffer
		GL(glBufferData(GL_ARRAY_BUFFER, totalSize, m->parent.verticesCol, GL_STATIC_DRAW));
	
		// attach vertex positions to current program
		int pos = glGetAttribLocation(currentProgram, "position");
		// program has `position`
		if (pos != -1) {
			GL(glVertexAttribPointer(pos, 3, GL_FLOAT, 0, 0, (void *) posOffset));
			GL(glEnableVertexAttribArray(pos));
		}

		// attach vertex colours
		if (m->parent.c) {
			int col = glGetAttribLocation(currentProgram, "colour");
			// program has colours
			if (col != -1) {
				GL(glVertexAttribPointer(col, 3, GL_FLOAT, 0, 0, (void *) colOffset));
				GL(glEnableVertexAttribArray(col));
			}
		}

		// attach texture coordinates
		if (m->parent.t) {
			int tex = glGetAttribLocation(currentProgram, "texCoord");
			// program has texCoord
			if (tex != -1) {
				GL(glVertexAttribPointer(tex, 2, GL_FLOAT, 0, 0, (void *) texOffset));
				GL(glEnableVertexAttribArray(tex));
			}
		}

		// attach normal
		if (m->parent.n) {
			int nor = glGetAttribLocation(currentProgram, "normal");
			// program has normal
			if (nor != -1) {
				GL(glVertexAttribPointer(nor, 3, GL_FLOAT, 0, 0, (void *) norOffset));
				GL(glEnableVertexAttribArray(nor));
			}
		}

		// attach tan
		if (m->parent.tan) {
			int tanLoc = glGetAttribLocation(currentProgram, "tangent");
			// program has tan
			if (tanLoc != -1) {
				GL(glVertexAttribPointer(tanLoc, 3, GL_FLOAT, 0, 0, (void *) tanOffset));
				GL(glEnableVertexAttribArray(tanLoc));
			}
		}

		// attach bitan
		if (m->parent.bitan) {
			int bitanLoc = glGetAttribLocation(currentProgram, "bitangent");
			// program has bitan
			if (bitanLoc != -1) {
				GL(glVertexAttribPointer(bitanLoc, 3, GL_FLOAT, 0, 0, (void *) bitanOffset));
				GL(glEnableVertexAttribArray(bitanLoc));
			}
		}

		// attach bone ids
		if (m->boneIds) {
			int boneIdsLoc = glGetAttribLocation(currentProgram, "boneIds");
			// program has boneIds
			if (boneIdsLoc != -1) {
				GL(glVertexAttribIPointer(boneIdsLoc, 4, GL_INT, 0, (void *) boneIdsOffset));
				GL(glEnableVertexAttribArray(boneIdsLoc));
			}
		}

		// attach weights
		if (m->weights) {
			int weightsLoc = glGetAttribLocation(currentProgram, "weights");
			//dd_log("weights loc: %d", weightsLoc);
			// program has weights
			if (weightsLoc != -1) {
				//dd_log("attached weights");
				GL(glVertexAttribPointer(weightsLoc, 4, GL_FLOAT, 0, 0, (void *) weightsOffset));
				GL(glEnableVertexAttribArray(weightsLoc));
			}
		}
	}

	if (m->parent.hasTransparency) {
		avdl_graphics_EnableBlend();
	}

	if (m->parent.img) {
		m->parent.img->bindIndex(m->parent.img, 0);
		GLuint loc = glGetUniformLocation(currentProgram, "image");
		if (loc != -1) {
			GL(glUniform1i(loc, 0));
		}
	}
	if (m->parent.img_normal) {
		m->parent.img_normal->bindIndex(m->parent.img_normal, 1);
		GLuint loc = glGetUniformLocation(currentProgram, "image_normal");
		if (loc != -1) {
			GL(glUniform1i(loc, 1));
		}
	}

	GL(glBindVertexArray(m->parent.array));

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

	if (avdl_skeleton_IsActive(&m->skeleton)) {
		int boneMatricesId = avdl_graphics_GetUniformLocation(currentProgram, "avdl_finalBonesMatrices");
		if (boneMatricesId >= 0) {
			avdl_graphics_SetUniformMatrix4fMultiple(boneMatricesId, AVDL_SKELETON_NUMBER_OF_BONES, avdl_skeleton_GetFinalMatrices(&m->skeleton));
		}
	}

	// draw arrays
	if (m->parent.draw_type) {
		// not possible on OpenGL ES
		//GL(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
		GL(glDrawArrays(GL_LINES, 0, m->parent.vcount));
	}
	else {
		GL(glDrawArrays(GL_TRIANGLES, 0, m->parent.vcount));
	}
	GL(glBindVertexArray(0));

	if (m->parent.img) {
		m->parent.img->unbind(m->parent.img);
	}

	if (m->parent.img_normal) {
		m->parent.img_normal->unbind(m->parent.img_normal);
	}

	if (m->parent.hasTransparency) {
		avdl_graphics_DisableBlend();
	}
	#endif
}

void avdl_skinned_mesh_update(struct avdl_skinned_mesh *o, float dt) {
	avdl_skeleton_update(&o->skeleton, dt);
}

void avdl_skinned_mesh_PlayAnimation(struct avdl_skinned_mesh *o, const char *animName) {
	avdl_skeleton_PlayAnimation(&o->skeleton, animName);
}

void avdl_skinned_mesh_PrintAnimations(struct avdl_skinned_mesh *o) {
	if (!avdl_skeleton_IsActive(&o->skeleton)) return;
	for (int i = 0; i < o->skeleton.animations_count; i++) {
		dd_log(o->skeleton.animations[i].name);
	}
}

void avdl_skinned_mesh_LoadFromLoadedMesh(struct avdl_skinned_mesh *o, struct dd_loaded_mesh *lm) {
	avdl_skinned_mesh_clean(o);
	avdl_mesh_LoadFromLoadedMesh(&o->parent, lm);
	if (lm->boneIds) {
		o->boneIds = lm->boneIds;
		o->dirtyBoneIds = 1;
	}
	if (lm->weights) {
		o->weights = lm->weights;
		o->dirtyWeights = 1;
	}

	// skeleton exists if there are bones, inverse bind matrices and at least one animation
	if (lm->boneCount > 0 && lm->inverseBindMatrices && lm->animationsCount > 0) {
		avdl_skeleton_Activate(&o->skeleton);

		// skeleton data
		o->skeleton.boneCount = lm->boneCount;
		o->skeleton.inverseBindMatrices = lm->inverseBindMatrices;
		o->skeleton.rootIndex = lm->rootIndex;
		o->skeleton.children_indices = lm->children_indices;
		o->skeleton.children_indices_count = lm->children_indices_count;
		dd_matrix_copy(&o->skeleton.rootMatrix, &lm->rootMatrix);

		avdl_skeleton_SetAnimations(&o->skeleton, lm->animationsCount, lm->animations);
	}
}

void avdl_skinned_mesh_SetOnAnimationDone(struct avdl_skinned_mesh *o, void (*func)(void *ctx), void *context) {
	o->skeleton.SetOnAnimationDone(&o->skeleton, func, context);
}
