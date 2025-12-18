#include "avdl_ray3.h"
#include "avdl_vec3.h"
#include "avdl_vec4.h"
#include "shared/avdl_log.h"
#include "shared/avdl_math.h"

void avdl_ray3_create(struct avdl_ray3 *o) {
	avdl_vec3_Setf(&o->position, 0, 0, 0);
	avdl_vec3_Setf(&o->direction, 0, 0, -1);
}

void avdl_ray3_SetPosition(struct avdl_ray3 *o, struct avdl_vec3 *position) {
	avdl_ray3_SetPosition3f(o, avdl_vec3_X(position), avdl_vec3_Y(position), avdl_vec3_Z(position));
}

void avdl_ray3_SetPositionVec4(struct avdl_ray3 *o, struct avdl_vec4 *position) {
	avdl_ray3_SetPosition3f(o, avdl_vec4_X(position), avdl_vec4_Y(position), avdl_vec4_Z(position));
}

void avdl_ray3_SetPosition3f(struct avdl_ray3 *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->position, x, y, z);
}

struct avdl_vec3 *avdl_ray3_GetPosition(struct avdl_ray3 *o) {
	return &o->position;
}

void avdl_ray3_SetDirection(struct avdl_ray3 *o, struct avdl_vec3 *direction) {
	avdl_ray3_SetDirection3f(o, avdl_vec3_X(direction), avdl_vec3_Y(direction), avdl_vec3_Z(direction));
}

void avdl_ray3_SetDirectionVec4(struct avdl_ray3 *o, struct avdl_vec4 *direction) {
	avdl_ray3_SetDirection3f(o, avdl_vec4_X(direction), avdl_vec4_Y(direction), avdl_vec4_Z(direction));
}

void avdl_ray3_SetDirection3f(struct avdl_ray3 *o, float x, float y, float z) {
	avdl_vec3_Setf(&o->direction, x, y, z);
}

struct avdl_vec3 *avdl_ray3_GetDirection(struct avdl_ray3 *o) {
	return &o->direction;
}

void avdl_ray3_Print(struct avdl_ray3 *o) {
	avdl_log("Ray:");
	avdl_vec3_Print(&o->position);
	avdl_vec3_Print(&o->direction);
}

int avdl_ray3_CollisionWithAABB(struct avdl_ray3 *ray, struct avdl_collider_aabb *collider) {

	// Code currently does obb test, can be optimised to do aabb instead
	struct avdl_vec3 center;
	avdl_vec3_Setf(&center,
		avdl_vec3_X(&collider->min) +(avdl_vec3_X(&collider->max) -avdl_vec3_X(&collider->min))/2,
		avdl_vec3_Y(&collider->min) +(avdl_vec3_Y(&collider->max) -avdl_vec3_Y(&collider->min))/2,
		avdl_vec3_Z(&collider->min) +(avdl_vec3_Z(&collider->max) -avdl_vec3_Z(&collider->min))/2
	);

	struct avdl_vec3 planeCenter[6];
	struct avdl_vec3 planeNormal[6];
	avdl_vec3_Setf(&planeCenter[0], avdl_vec3_X(&center       ), avdl_vec3_Y(&center       ), avdl_vec3_Z(&collider->max));
	avdl_vec3_Setf(&planeCenter[1], avdl_vec3_X(&collider->min), avdl_vec3_Y(&center       ), avdl_vec3_Z(&center       ));
	avdl_vec3_Setf(&planeCenter[2], avdl_vec3_X(&center       ), avdl_vec3_Y(&center       ), avdl_vec3_Z(&collider->min));
	avdl_vec3_Setf(&planeCenter[3], avdl_vec3_X(&collider->max), avdl_vec3_Y(&center       ), avdl_vec3_Z(&center       ));
	avdl_vec3_Setf(&planeCenter[4], avdl_vec3_X(&center       ), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&center       ));
	avdl_vec3_Setf(&planeCenter[4], avdl_vec3_X(&center       ), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&center       ));
	avdl_vec3_Setf(&planeNormal[0],  0,  0,  1);
	avdl_vec3_Setf(&planeNormal[1], -1,  0,  0);
	avdl_vec3_Setf(&planeNormal[2],  0,  0, -1);
	avdl_vec3_Setf(&planeNormal[3],  1,  0,  0);
	avdl_vec3_Setf(&planeNormal[4],  0,  1,  0);
	avdl_vec3_Setf(&planeNormal[5],  0, -1,  0);

	// for each plane
	for (int j = 0; j < 6; j += 1) {

		// check if ray is parallel to plane, if so, no intersection
		float dot;
		dot = avdl_vec3_Dot(&planeNormal[j], avdl_ray3_GetDirection(ray));
		if (dd_math_abs(dot) < 0.001) {
			//avdl_log("almost parallel dot");
			continue;
		}

		// get t of ray to plane
		struct avdl_vec3 difference;
		avdl_vec3_Set(&difference, &planeCenter[j]);
		avdl_vec3_Subtract(&difference, avdl_ray3_GetPosition(ray));

		float t;
		t = avdl_vec3_Dot(&difference, &planeNormal[j]) /dot;
		if (t < 0) {
			//avdl_log("ray pointing away from plane");
			continue;
		}

		// use t to find position on plane
		struct avdl_vec3 rot;
		avdl_vec3_Set(&rot, avdl_ray3_GetDirection(ray));
		avdl_vec3_Multiply1f(&rot, t);

		struct avdl_vec3 finalPosition;
		avdl_vec3_Set(&finalPosition, avdl_ray3_GetPosition(ray));
		avdl_vec3_Addf(&finalPosition,
			avdl_vec3_X(&rot),
			avdl_vec3_Y(&rot),
			avdl_vec3_Z(&rot)
		);

		// 4 vertices, one on each corner of the box's side
		struct avdl_vec3 vertices[4];
		if (j == 0) {
			avdl_vec3_Setf(&vertices[0], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->max));
			avdl_vec3_Setf(&vertices[1], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->max));
			avdl_vec3_Setf(&vertices[2], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->max));
			avdl_vec3_Setf(&vertices[3], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->max));
		}
		else
		if (j == 1) {
			avdl_vec3_Setf(&vertices[0], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[1], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[2], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->max));
			avdl_vec3_Setf(&vertices[3], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->max));
		}
		else
		if (j == 2) {
			avdl_vec3_Setf(&vertices[0], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[1], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[2], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[3], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->min));
		}
		else
		if (j == 3) {
			avdl_vec3_Setf(&vertices[0], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[1], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[2], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->max));
			avdl_vec3_Setf(&vertices[3], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->max));
		}
		else
		if (j == 4) {
			avdl_vec3_Setf(&vertices[0], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->max));
			avdl_vec3_Setf(&vertices[1], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[2], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[3], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->max), avdl_vec3_Z(&collider->max));
		}
		else
		if (j == 5) {
			avdl_vec3_Setf(&vertices[0], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->max));
			avdl_vec3_Setf(&vertices[1], avdl_vec3_X(&collider->min), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[2], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->min));
			avdl_vec3_Setf(&vertices[3], avdl_vec3_X(&collider->max), avdl_vec3_Y(&collider->min), avdl_vec3_Z(&collider->max));
		}
//		(for (int k 0) (< k 4) (+= k 1) {
//			#(vertices[k].MultiplyMatrix (node.GetGlobalMatrix) 1)
//			#(vertices[k].MultiplyMatrix (this.camera_node.GetGlobalInverseMatrix) 1)
//		})

		int failCheck = 0;
		// for each edge
		for (int k = 0; k < 4; k++) {

			int baseIndex = k % 4;
			int firstIndex = (k + 1) % 4;
			int secondIndex = (k + 4 -1) % 4;

			// edge AB
			struct avdl_vec3 temp1;
			struct avdl_vec3 temp2;
			struct avdl_vec3 temp3;
			avdl_vec3_Set(&temp1, &vertices[firstIndex]);
			avdl_vec3_Set(&temp2, &finalPosition);
			avdl_vec3_Set(&temp3, &vertices[secondIndex]);
			avdl_vec3_Subtract(&temp1, &vertices[baseIndex]);
			avdl_vec3_Subtract(&temp2, &vertices[baseIndex]);
			avdl_vec3_Subtract(&temp3, &vertices[baseIndex]);

			struct avdl_vec3 cp1;
			struct avdl_vec3 cp2;

			avdl_vec3_Set(&cp1, &temp1);
			avdl_vec3_Cross(&cp1, &temp2);

			avdl_vec3_Set(&cp2, &temp1);
			avdl_vec3_Cross(&cp2, &temp3);

			if ((avdl_vec3_Dot(&planeNormal[j], &cp1) *avdl_vec3_Dot(&planeNormal[j], &cp2)) < 0) {
				// avdl_log("WRONG side of k: %d" k);
				failCheck = 1;
				break;
			}
		}
		if (failCheck) {
			continue;
		}

		return 1;
	}
	return 0;
}
