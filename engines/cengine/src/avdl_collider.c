#include "avdl_collider.h"
#include "avdl_collider_aabb.h"
#include "avdl_collider_sphere.h"
#include "shared/avdl_log.h"
#include "shared/avdl_math.h"
#include "avdl_component_terrain.h"

#include <ode/ode.h>
dSpaceID avdl_collision_space;

void avdl_collider_create(struct avdl_collider *o) {
	o->type = AVDL_COLLIDER_TYPE_POINT;
	o->geom = 0;
}

void avdl_collider_clean(struct avdl_collider *o) {
}

int avdl_collider_collision(struct avdl_collider *o1, struct dd_matrix *m1, struct dd_matrix *nm1, struct avdl_collider *o2, struct dd_matrix *m2, struct dd_matrix *nm2) {

	if (o1->type == AVDL_COLLIDER_TYPE_AABB && o2->type == AVDL_COLLIDER_TYPE_AABB) {

		struct avdl_collider_aabb *col1 = o1;
		struct avdl_collider_aabb *col2 = o2;

		// collect vertices
		struct avdl_vec4 vertices1[8];
		struct avdl_vec4 vertices2[8];
		avdl_vec4_Setf(&vertices1[0],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		avdl_vec4_Setf(&vertices1[1],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		avdl_vec4_Setf(&vertices1[2],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		avdl_vec4_Setf(&vertices1[3],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		avdl_vec4_Setf(&vertices1[4],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		avdl_vec4_Setf(&vertices1[5],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		avdl_vec4_Setf(&vertices1[6],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		avdl_vec4_Setf(&vertices1[7],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		avdl_vec4_Setf(&vertices2[0],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		avdl_vec4_Setf(&vertices2[1],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		avdl_vec4_Setf(&vertices2[2],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		avdl_vec4_Setf(&vertices2[3],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		avdl_vec4_Setf(&vertices2[4],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		avdl_vec4_Setf(&vertices2[5],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		avdl_vec4_Setf(&vertices2[6],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		avdl_vec4_Setf(&vertices2[7],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		for (int i = 0; i < 8; i++) {
			avdl_vec4_MultiplyMatrix(&vertices1[i], m1);
			avdl_vec4_MultiplyMatrix(&vertices2[i], m2);
		}

/*
		struct avdl_vec4 min1;
		avdl_vec4_Setf(&min1,
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		avdl_vec4_MultiplyMatrix(&min1, m1);
		struct avdl_vec4 max1;
		avdl_vec4_Setf(&max1,
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		avdl_vec4_MultiplyMatrix(&max1, m1);
		avdl_log("min/max 1");
		avdl_vec4_print(&min1);
		avdl_vec4_print(&max1);

		struct avdl_vec4 min2;
		avdl_vec4_Setf(&min2,
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		avdl_vec4_MultiplyMatrix(&min2, m2);
		struct avdl_vec4 max2;
		avdl_vec4_Setf(&max2,
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		avdl_vec4_MultiplyMatrix(&max2, m2);
		avdl_log("min/max 2");
		avdl_vec4_print(&min2);
		avdl_vec4_print(&max2);
		*/

		// mult by 90 degrees matrix to find normal
		struct dd_matrix norm_matrix;
		dd_matrix_identity(&norm_matrix);

		// normals: up, right, front
		int total_normals = 6;
		struct avdl_vec4 normals[15];
		avdl_vec4_Setf(&normals[0], 0, 1, 0, 1);
		avdl_vec4_Setf(&normals[1], 1, 0, 0, 1);
		avdl_vec4_Setf(&normals[2], 0, 0, 1, 1);

		avdl_vec4_Setf(&normals[3], 0, 1, 0, 1);
		avdl_vec4_Setf(&normals[4], 1, 0, 0, 1);
		avdl_vec4_Setf(&normals[5], 0, 0, 1, 1);

		for (int i = 0; i < 6; i++) {
			if (i < 3) {
				avdl_vec4_MultiplyMatrix(&normals[i], nm1);
			}
			else {
				avdl_vec4_MultiplyMatrix(&normals[i], nm2);
			}
		}

		for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++) {
			if (avdl_vec4_X(&normals[i]) == avdl_vec4_X(&normals[3 +j])
			&&  avdl_vec4_Y(&normals[i]) == avdl_vec4_Y(&normals[3 +j])
			&&  avdl_vec4_Z(&normals[i]) == avdl_vec4_Z(&normals[3 +j])) {
				continue;
			}
			int index = total_normals;//6 +(i*3 +j);
			//avdl_log("set normal index: %d", index);
			avdl_vec4_Setf(&normals[index],
				avdl_vec4_X(&normals[i]),
				avdl_vec4_Y(&normals[i]),
				avdl_vec4_Z(&normals[i]),
				avdl_vec4_W(&normals[i])
			);
			//avdl_vec4_print(&normals[index]);
			//avdl_vec4_print(&normals[3 +j]);
			avdl_vec4_Cross(&normals[index], &normals[3 +j]);
			total_normals++;
		}

		//avdl_log("~~~ Normal checks");
		for (int i = 0; i < total_normals; i++) {
			//avdl_log("Normal %d", i);
			if (avdl_vec4_X(&normals[i]) == 0
			&&  avdl_vec4_Y(&normals[i]) == 0
			&&  avdl_vec4_Z(&normals[i]) == 0) {
				//avdl_log("skip");
				continue;
			}
			//avdl_vec4_print(&normals[i]);
			avdl_vec4_Normalise(&normals[i]);

			int haslimits = 0;
			float finalMin1 = 0;
			float finalMax1 = 0;
			float finalMin2 = 0;
			float finalMax2 = 0;

			for (int j = 0; j < 8; j++) {
				//avdl_log("vertex: %d", j);
				//avdl_vec4_print(&vertices1[j]);
				//avdl_vec4_print(&vertices2[j]);
				// just dot it to get the min/max along this axis.
				float dotValMin1 = avdl_vec4_Dot(&normals[i], &vertices1[j]);
				float dotValMin2 = avdl_vec4_Dot(&normals[i], &vertices2[j]);

				if (!haslimits) {
					finalMin1 = dotValMin1;
					finalMax1 = dotValMin1;
					finalMin2 = dotValMin2;
					finalMax2 = dotValMin2;
					haslimits = 1;
					//avdl_log("set limits %d - %f %f - %f %f", i, finalMin1, finalMax1, finalMin2, finalMax2);
				}
				else {
					if (dotValMin1 < finalMin1) {
						finalMin1 = dotValMin1;
					}
					else
					if (dotValMin1 > finalMax1) {
						finalMax1 = dotValMin1;
					}
					if (dotValMin2 < finalMin2) {
						finalMin2 = dotValMin2;
					}
					else
					if (dotValMin2 > finalMax2) {
						finalMax2 = dotValMin2;
					}
					//avdl_log("modify limits %d - %f %f - %f %f", i, finalMin1, finalMax1, finalMin2, finalMax2);
				}
			}

			//avdl_log("inv %d - %f %f - %f %f", i, finalMin1, finalMax1, finalMin2, finalMax2);
			//avdl_vec4_print(&normals[i]);

			// aabb collision
			if ((finalMax2 -finalMin1 > 0) != (finalMin2 -finalMax1 > 0)) {
				//avdl_log("sat collide");
			}
			else {
				//avdl_log("sat not collide");
				return 0;
			}
		}
		return 1;

		/*
		// aabb collision
		if ((max2.cell[0] -min1.cell[0] > 0) != (min2.cell[0] -max1.cell[0] > 0)
		&&  (max2.cell[1] -min1.cell[1] > 0) != (min2.cell[1] -max1.cell[1] > 0)
		&&  (max2.cell[2] -min1.cell[2] > 0) != (min2.cell[2] -max1.cell[2] > 0)) {
			return 1;
		}
		*/

		return 0;
	}

	avdl_log("collision not supported");
	return 0;
}

int once = 0;
int avdl_collider_collisionNode(struct avdl_collider *o1, struct avdl_node *n1, struct avdl_collider *o2, struct avdl_node *n2, struct avdl_collider_collision *collision) {

	if (o1->type == AVDL_COLLIDER_TYPE_AABB && o2->type == AVDL_COLLIDER_TYPE_AABB) {

		//avdl_log("aabb to aabb");
		struct avdl_collider_aabb *col1 = o1;
		struct avdl_collider_aabb *col2 = o2;

		struct avdl_vec4 vertex1;
		struct avdl_vec4 vertex2;
		avdl_vec4_Setf(&vertex1, 0, 0, 0, 1);
		avdl_vec4_Setf(&vertex2, 0, 0, 0, 1);
		avdl_vec4_MultiplyMatrix(&vertex1, avdl_node_GetGlobalMatrix(n1));
		avdl_vec4_MultiplyMatrix(&vertex2, avdl_node_GetGlobalMatrix(n2));

		dGeomSetPosition(col1->parent.geom,
			avdl_vec4_X(&vertex1),
			avdl_vec4_Y(&vertex1),
			avdl_vec4_Z(&vertex1)
		);
		dGeomSetPosition(col2->parent.geom,
			avdl_vec4_X(&vertex2),
			avdl_vec4_Y(&vertex2),
			avdl_vec4_Z(&vertex2)
		);

		struct dd_matrix *nm = avdl_node_GetGlobalNormalMatrix(n1); 
		struct dd_matrix *nm2 = avdl_node_GetGlobalNormalMatrix(n2); 

		dMatrix3 R;
		dMatrix3 R2;
		for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++) {

			// `dMatrix3` is row-major but `dd_matrix` is column major
			int indexRowMajor = (y *4) +x;
			int indexColumnMajor = (x *4) +y;

			// dMatrix3 is 3x4, so skip the last row
			if (y == 3) continue;

			R[indexRowMajor] = nm->cell[indexColumnMajor];
			R2[indexRowMajor] = nm2->cell[indexColumnMajor];
		}
		dGeomSetRotation(col1->parent.geom, R);
		dGeomSetRotation(col2->parent.geom, R2);

		struct avdl_vec3 *scale1 = avdl_node_GetGlobalScale(n1);
		struct avdl_vec3 *scale2 = avdl_node_GetGlobalScale(n2);
		dGeomBoxSetLengths(col1->parent.geom,
			(avdl_collider_aabb_getLengthX(col1) *avdl_vec3_X(scale1)),
			(avdl_collider_aabb_getLengthX(col1) *avdl_vec3_Y(scale1)),
			(avdl_collider_aabb_getLengthX(col1) *avdl_vec3_Z(scale1))
		);
		dGeomBoxSetLengths(col2->parent.geom,
			(avdl_collider_aabb_getLengthX(col2) *avdl_vec3_X(scale2)),
			(avdl_collider_aabb_getLengthX(col2) *avdl_vec3_Y(scale2)),
			(avdl_collider_aabb_getLengthX(col2) *avdl_vec3_Z(scale2))
		);

		dContact contact[4];
		int n = dCollide(col1->parent.geom, col2->parent.geom, 4, &contact[0].geom, sizeof(dContact));
	
		if (n > 0) {
			for (int i = 0; i < n; i++) {
				/*
				printf("Collision aabb vs aabb!\n");
				printf("Contact point: (%f, %f, %f)\n",
					contact[i].geom.pos[0],
					contact[i].geom.pos[1],
					contact[i].geom.pos[2]);
	
				printf("Contact normal: (%f, %f, %f)\n",
					contact[i].geom.normal[0],
					contact[i].geom.normal[1],
					contact[i].geom.normal[2]);
	
				printf("Penetration depth: %f\n",
					contact[i].geom.depth);
				printf("-------------------------\n");
				*/
				if (collision) {
					avdl_vec4_Setf(&collision->overlap,
						-contact[i].geom.normal[0],
						-contact[i].geom.normal[1],
						contact[i].geom.normal[2],
						0
					);
					avdl_vec4_Normalise3(&collision->overlap);
					avdl_vec4_Multiply1f(&collision->overlap, contact[i].geom.depth);
					avdl_vec4_Setf(&collision->normal1,
						contact[i].geom.normal[0],
						contact[i].geom.normal[1],
						contact[i].geom.normal[2],
						0
					);
				}
				return 1;
			}
		}

		return 0;
	}
	else
	if (o1->type == AVDL_COLLIDER_TYPE_SPHERE && o2->type == AVDL_COLLIDER_TYPE_SPHERE) {

		//avdl_log("sphere to sphere collision");
		struct avdl_collider_sphere *col1 = o1;
		struct avdl_collider_sphere *col2 = o2;

		struct avdl_vec4 vertex1;
		struct avdl_vec4 vertex2;
		avdl_vec4_Setf(&vertex1, 0, 0, 0, 1);
		avdl_vec4_Setf(&vertex2, 0, 0, 0, 1);
		avdl_vec4_MultiplyMatrix(&vertex1, avdl_node_GetGlobalMatrix(n1));
		avdl_vec4_MultiplyMatrix(&vertex2, avdl_node_GetGlobalMatrix(n2));

		dGeomSetPosition(col1->parent.geom,
			avdl_vec4_X(&vertex1),
			avdl_vec4_Y(&vertex1),
			avdl_vec4_Z(&vertex1)
		);
		dGeomSetPosition(col2->parent.geom,
			avdl_vec4_X(&vertex2),
			avdl_vec4_Y(&vertex2),
			avdl_vec4_Z(&vertex2)
		);

		dContact contact[4];
		int n = dCollide(col1->parent.geom, col2->parent.geom, 4, &contact[0].geom, sizeof(dContact));
	
		if (n > 0) {
			for (int i = 0; i < n; i++) {
				printf("Collision!\n");
				printf("Contact point: (%f, %f, %f)\n",
					contact[i].geom.pos[0],
					contact[i].geom.pos[1],
					contact[i].geom.pos[2]);
	
				printf("Contact normal: (%f, %f, %f)\n",
					contact[i].geom.normal[0],
					contact[i].geom.normal[1],
					contact[i].geom.normal[2]);
	
				printf("Penetration depth: %f\n",
					contact[i].geom.depth);
				printf("-------------------------\n");
				/*
				*/
				if (collision) {
					avdl_vec4_Setf(&collision->overlap,
						-contact[i].geom.normal[0],
						-contact[i].geom.normal[1],
						contact[i].geom.normal[2],
						0
					);
					avdl_vec4_Normalise3(&collision->overlap);
					avdl_vec4_Multiply1f(&collision->overlap, contact[i].geom.depth);
				}
				return 1;
			}
		}
		return 0;
	}
	else
	if (o1->type == AVDL_COLLIDER_TYPE_AABB && o2->type == AVDL_COLLIDER_TYPE_SPHERE) {

		//avdl_log("aabb to sphere collision");
		struct avdl_collider_aabb *col1 = o1;
		struct avdl_collider_sphere *col2 = o2;

		struct avdl_vec4 vertex1;
		struct avdl_vec4 vertex2;
		avdl_vec4_Setf(&vertex1, 0, 0, 0, 1);
		avdl_vec4_Setf(&vertex2, 0, 0, 0, 1);
		avdl_vec4_MultiplyMatrix(&vertex1, avdl_node_GetGlobalMatrix(n1));
		avdl_vec4_MultiplyMatrix(&vertex2, avdl_node_GetGlobalMatrix(n2));

		dGeomSetPosition(col1->parent.geom,
			avdl_vec4_X(&vertex1),
			avdl_vec4_Y(&vertex1),
			avdl_vec4_Z(&vertex1)
		);

		struct avdl_transform *t = avdl_node_GetLocalTransform(n1);
		dMatrix3 R;
		dRFromEulerAngles(R,
			dd_math_dec2rad(avdl_transform_GetRotationX(t)),
			dd_math_dec2rad(avdl_transform_GetRotationY(t)),
			dd_math_dec2rad(avdl_transform_GetRotationZ(t))
		);
		dGeomSetRotation(col1->parent.geom, R);
		dGeomSetPosition(col2->parent.geom,
			avdl_vec4_X(&vertex2),
			avdl_vec4_Y(&vertex2),
			avdl_vec4_Z(&vertex2)
		);

		dContact contact[4];
		int n = dCollide(col1->parent.geom, col2->parent.geom, 4, &contact[0].geom, sizeof(dContact));
	
		if (n > 0) {
			for (int i = 0; i < n; i++) {
				/*
				printf("Collision aabb vs sphere!\n");
				printf("Contact point: (%f, %f, %f)\n",
					contact[i].geom.pos[0],
					contact[i].geom.pos[1],
					contact[i].geom.pos[2]);
	
				printf("Contact normal: (%f, %f, %f)\n",
					contact[i].geom.normal[0],
					contact[i].geom.normal[1],
					contact[i].geom.normal[2]);
	
				printf("Penetration depth: %f\n",
					contact[i].geom.depth);
				printf("-------------------------\n");
				*/
				if (collision) {
					avdl_vec4_Setf(&collision->overlap,
						-contact[i].geom.normal[0],
						-contact[i].geom.normal[1],
						contact[i].geom.normal[2],
						0
					);
					avdl_vec4_Normalise3(&collision->overlap);
					avdl_vec4_Multiply1f(&collision->overlap, contact[i].geom.depth);
					avdl_vec4_Setf(&collision->normal1,
						contact[i].geom.normal[0],
						contact[i].geom.normal[1],
						contact[i].geom.normal[2],
						0
					);
				}
				return 1;
			}
		}

		return 0;
	}
	else
	if ((o1->type == AVDL_COLLIDER_TYPE_TERRRAIN && o2->type == AVDL_COLLIDER_TYPE_POINT )
	||  (o1->type == AVDL_COLLIDER_TYPE_TERRRAIN && o2->type == AVDL_COLLIDER_TYPE_SPHERE)) {
		//avdl_log("terrain point collision");

		// get point position
		struct avdl_vec4 vertex2;
		avdl_vec4_Setf(&vertex2, 0, 0, 0, 1);

		if (o2->type == AVDL_COLLIDER_TYPE_SPHERE) {
			struct avdl_collider_sphere *col2 = o2;
			avdl_vec4_Setf(&vertex2, 0, -col2->radius, 0, 1);
		}
		avdl_vec4_MultiplyMatrix(&vertex2, avdl_node_GetGlobalMatrix(n2));
		avdl_vec4_MultiplyMatrix(&vertex2, avdl_node_GetGlobalInverseMatrix(n1));

		struct avdl_terrain *t;
		t = avdl_collider_terrain_GetTerrain(o1);

		float terrainSpot = avdl_terrain_getSpot(t, avdl_vec4_X(&vertex2), -avdl_vec4_Z(&vertex2));

		//avdl_log("player position:");
		//avdl_vec3_Print(&vertex2);
		//avdl_log("terrain spot: %f", terrainSpot);
		if (avdl_vec4_Y(&vertex2) < terrainSpot) {
			if (collision) {
				avdl_vec4_Setf(&collision->overlap, 0, 0, 0, 0);
				avdl_vec4_SetY(&collision->overlap, terrainSpot -avdl_vec4_Y(&vertex2));

				avdl_terrain_getNormal(t, avdl_vec4_X(&vertex2), -avdl_vec4_Z(&vertex2), &collision->normal1);
				avdl_vec3_Setf(&collision->normal2, 0, 0, 0);
			}
			return 1;
		}

		return 0;
	}
	else
	if (o1->type == AVDL_COLLIDER_TYPE_SPHERE && o2->type == AVDL_COLLIDER_TYPE_POINT) {
		//avdl_log("sphere point collision");

		/*
		// get sphere position
		struct avdl_vec4 vertexSphere;
		avdl_vec4_Setf(&vertexSphere, 0, 0, 0, 1);
		avdl_vec4_MultiplyMatrix(&vertexSphere, avdl_node_GetGlobalMatrix(n1));

		// get point position
		struct avdl_vec4 vertexPoint;
		avdl_vec4_Setf(&vertexPoint, 0, 0, 0, 1);
		avdl_vec4_MultiplyMatrix(&vertexPoint, avdl_node_GetGlobalMatrix(n2));

		if (dd_math_abs(avdl_vec4_Distance(&vertexPoint, &vertexSphere)) < 1.0) {
			avdl_log("sphere collision: %f", avdl_vec4_Distance(&vertexPoint, &vertexSphere));
			avdl_vec4_Set(&collision->overlap, &vertexSphere);
			avdl_vec4_Subtract(&collision->overlap, &vertexPoint);
			avdl_vec4_Invert(&collision->overlap);

			avdl_vec4_Set(&collision->normal1, &collision->overlap);
			avdl_vec4_Normalise(&collision->normal1);

			avdl_vec4_Set(&collision->normal2, &collision->overlap);
			avdl_vec4_Normalise(&collision->normal2);
			avdl_vec4_Invert(&collision->normal2);
			return 1;
		}
		*/

		/*
		//avdl_log("player position:");
		//avdl_vec3_Print(&vertexPoint);
		//avdl_log("terrain spot: %f", terrainSpot);
		if (avdl_vec4_Y(&vertexPoint) < terrainSpot) {
			avdl_vec4_Setf(&collision->overlap, 0, 0, 0, 0);
			avdl_vec4_SetY(&collision->overlap, terrainSpot -avdl_vec4_Y(&vertexPoint));

			avdl_terrain_getNormal(t, avdl_vec4_X(&vertexPoint), -avdl_vec4_Z(&vertexPoint), &collision->normal1);
			avdl_vec3_Setf(&collision->normal2, 0, 0, 0);
			return 1;
		}
		*/
		return 0;
	}
	else
	// swaps
	if ((o1->type == AVDL_COLLIDER_TYPE_POINT && o2->type == AVDL_COLLIDER_TYPE_TERRRAIN)
	||  (o1->type == AVDL_COLLIDER_TYPE_POINT && o2->type == AVDL_COLLIDER_TYPE_SPHERE  )
	||  (o1->type == AVDL_COLLIDER_TYPE_SPHERE && o2->type == AVDL_COLLIDER_TYPE_TERRRAIN)
	||  (o1->type == AVDL_COLLIDER_TYPE_SPHERE && o2->type == AVDL_COLLIDER_TYPE_AABB)) {
		if (!avdl_collider_collisionNode(o2, n2, o1, n1, collision)) {
			return 0;
		}

		if (collision) {
			// swap data
			avdl_vec4_Invert(&collision->overlap);

			struct avdl_vec3 temp;
			avdl_vec3_Set(&temp, &collision->normal1);
			avdl_vec3_Set(&collision->normal1, &collision->normal2);
			avdl_vec3_Set(&collision->normal2, &temp);
		}

		return 1;
	}

	if (!once) {
		char *t1 = 0;
		char *t2 = 0;

		if (o1->type == AVDL_COLLIDER_TYPE_POINT) {
			t1 = "Point";
		}
		else
		if (o1->type == AVDL_COLLIDER_TYPE_SPHERE) {
			t1 = "Sphere";
		}
		else
		if (o1->type == AVDL_COLLIDER_TYPE_AABB) {
			t1 = "AABB";
		}

		if (o2->type == AVDL_COLLIDER_TYPE_POINT) {
			t2 = "Point";
		}
		else
		if (o2->type == AVDL_COLLIDER_TYPE_SPHERE) {
			t2 = "Sphere";
		}
		else
		if (o2->type == AVDL_COLLIDER_TYPE_AABB) {
			t2 = "AABB";
		}

		avdl_log("Collision not supported: (%s) - (%s)", t1, t2);
		once = 1;
	}
	return 0;
}

void avdl_collider_collision_create(struct avdl_collider_collision *o) {
}

void avdl_collider_collision_clean(struct avdl_collider_collision *o) {
}

struct avdl_vec4 *avdl_collider_collision_GetOverlap(struct avdl_collider_collision *o) {
	return &o->overlap;
}

struct avdl_vec3 *avdl_collider_collision_GetNormal1(struct avdl_collider_collision *o) {
	return &o->normal1;
}

struct avdl_vec3 *avdl_collider_collision_GetNormal2(struct avdl_collider_collision *o) {
	return &o->normal2;
}

int avdl_collider_DrawDebug(struct avdl_collider *o, struct avdl_node *n) {
	if (o->type == AVDL_COLLIDER_TYPE_AABB) {
		return avdl_collider_aabb_DrawDebug(o, n);
	}
	return 0;
}

int avdl_collider_DrawDebugNode(struct avdl_collider *o, struct avdl_node *n) {
	if (o->type == AVDL_COLLIDER_TYPE_AABB) {
		return avdl_collider_aabb_DrawDebugNode(o, n);
	}
	return 0;
}

void avdl_collider_init() {
	avdl_collider_aabb_init();
}

void avdl_collider_deinit() {
	avdl_collider_aabb_deinit();
}
