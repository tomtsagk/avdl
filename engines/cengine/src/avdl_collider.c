#include "avdl_collider.h"
#include "avdl_collider_aabb.h"
#include "avdl_collider_sphere.h"
#include "avdl_log.h"
#include "dd_math.h"
#include "avdl_component_terrain.h"

void avdl_collider_create(struct avdl_collider *o) {
	o->type = AVDL_COLLIDER_TYPE_POINT;
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

int avdl_collider_collisionNode(struct avdl_collider *o1, struct avdl_node *n1, struct avdl_collider *o2, struct avdl_node *n2, struct avdl_collider_collision *collision) {

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
			avdl_vec4_MultiplyMatrix(&vertices1[i], avdl_node_GetGlobalMatrix(n1));
			avdl_vec4_MultiplyMatrix(&vertices2[i], avdl_node_GetGlobalMatrix(n2));
		}

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
				avdl_vec4_MultiplyMatrix(&normals[i], avdl_node_GetGlobalNormalMatrix(n1));
			}
			else {
				avdl_vec4_MultiplyMatrix(&normals[i], avdl_node_GetGlobalNormalMatrix(n2));
			}
		}

		for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++) {
			if (avdl_vec4_X(&normals[i]) == avdl_vec4_X(&normals[3 +j])
			&&  avdl_vec4_Y(&normals[i]) == avdl_vec4_Y(&normals[3 +j])
			&&  avdl_vec4_Z(&normals[i]) == avdl_vec4_Z(&normals[3 +j])) {
				continue;
			}
			int index = total_normals;
			avdl_vec4_Setf(&normals[index],
				avdl_vec4_X(&normals[i]),
				avdl_vec4_Y(&normals[i]),
				avdl_vec4_Z(&normals[i]),
				avdl_vec4_W(&normals[i])
			);
			avdl_vec4_Cross(&normals[index], &normals[3 +j]);
			total_normals++;
		}

		for (int i = 0; i < total_normals; i++) {
			if (avdl_vec4_X(&normals[i]) == 0
			&&  avdl_vec4_Y(&normals[i]) == 0
			&&  avdl_vec4_Z(&normals[i]) == 0) {
				continue;
			}
			avdl_vec4_Normalise(&normals[i]);

			int haslimits = 0;
			float finalMin1 = 0;
			float finalMax1 = 0;
			float finalMin2 = 0;
			float finalMax2 = 0;

			for (int j = 0; j < 8; j++) {
				// just dot it to get the min/max along this axis.
				float dotValMin1 = avdl_vec4_Dot(&normals[i], &vertices1[j]);
				float dotValMin2 = avdl_vec4_Dot(&normals[i], &vertices2[j]);

				if (!haslimits) {
					finalMin1 = dotValMin1;
					finalMax1 = dotValMin1;
					finalMin2 = dotValMin2;
					finalMax2 = dotValMin2;
					haslimits = 1;
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
				}
			}

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
	else
	if (o1->type == AVDL_COLLIDER_TYPE_SPHERE && o2->type == AVDL_COLLIDER_TYPE_SPHERE) {

		struct avdl_collider_sphere *col1 = o1;
		struct avdl_collider_sphere *col2 = o2;

		// collect vertices
		struct avdl_vec4 vertex1;
		struct avdl_vec4 vertex2;
		avdl_vec4_Setf(&vertex1, 0, 0, 0, 1);
		avdl_vec4_Setf(&vertex2, 0, 0, 0, 1);
		avdl_vec4_MultiplyMatrix(&vertex1, avdl_node_GetGlobalMatrix(n1));
		avdl_vec4_MultiplyMatrix(&vertex2, avdl_node_GetGlobalMatrix(n2));

		// collect radius
		struct avdl_vec4 rad1;
		struct avdl_vec4 rad2;
		avdl_vec4_Setf(&rad1, 0, 0, col1->radius, 1);
		avdl_vec4_Setf(&rad2, 0, 0, col2->radius, 1);
		avdl_vec4_MultiplyMatrix(&rad1, avdl_node_GetGlobalMatrix(n1));
		avdl_vec4_MultiplyMatrix(&rad2, avdl_node_GetGlobalMatrix(n2));
		float rad1f = avdl_vec4_Distance(&vertex1, &rad1);
		float rad2f = avdl_vec4_Distance(&vertex2, &rad2);

		float distance = avdl_vec4_Distance(&vertex1, &vertex2);

		if (distance < rad1f +rad2f) {

			if (collision) {
				avdl_vec4_Setf(&collision->overlap,
					avdl_vec4_X(&vertex2) -avdl_vec4_X(&vertex1),
					avdl_vec4_Y(&vertex2) -avdl_vec4_Y(&vertex1),
					avdl_vec4_Z(&vertex2) -avdl_vec4_Z(&vertex1),
					avdl_vec4_W(&vertex2) -avdl_vec4_W(&vertex1)
				);
				avdl_vec4_Normalise(&collision->overlap);
				avdl_vec4_Multiply1f(&collision->overlap, rad1f +rad2f -distance);
			}

			return 1;
		}

		return 0;
	}
	else
	if (o1->type == AVDL_COLLIDER_TYPE_AABB && o2->type == AVDL_COLLIDER_TYPE_SPHERE) {

		struct avdl_collider_aabb *col1 = o1;
		struct avdl_collider_sphere *col2 = o2;

		struct avdl_vec4 vertex2;
		avdl_vec4_Setf(&vertex2, 0, 0, 0, 1);
		avdl_vec4_MultiplyMatrix(&vertex2, avdl_node_GetGlobalMatrix(n2));
		avdl_vec4_MultiplyMatrix(&vertex2, avdl_node_GetGlobalInverseMatrix(n1));

		// collect radius
		struct avdl_vec4 rad2;
		avdl_vec4_Setf(&rad2, col2->radius, 0, 0, 1);
		avdl_vec4_MultiplyMatrix(&rad2, avdl_node_GetGlobalMatrix(n2));
		avdl_vec4_MultiplyMatrix(&rad2, avdl_node_GetGlobalInverseMatrix(n1));
		float rad2f = avdl_vec4_Distance(&vertex2, &rad2);
		//float rad2fx = rad2f;
		avdl_vec4_Setf(&rad2, 0, col2->radius, 0, 1);
		avdl_vec4_MultiplyMatrix(&rad2, avdl_node_GetGlobalMatrix(n2));
		avdl_vec4_MultiplyMatrix(&rad2, avdl_node_GetGlobalInverseMatrix(n1));
		rad2f = dd_math_max(avdl_vec4_Distance(&vertex2, &rad2), rad2f);
		//float rad2fy = rad2f;
		avdl_vec4_Setf(&rad2, 0, 0, col2->radius, 1);
		avdl_vec4_MultiplyMatrix(&rad2, avdl_node_GetGlobalMatrix(n2));
		avdl_vec4_MultiplyMatrix(&rad2, avdl_node_GetGlobalInverseMatrix(n1));
		rad2f = dd_math_max(avdl_vec4_Distance(&vertex2, &rad2), rad2f);
		//float rad2fz = rad2f;

		struct avdl_vec4 closest_point;
		avdl_vec4_Setf(&closest_point,
			dd_math_max(dd_math_min(avdl_vec4_X(&vertex2), avdl_collider_aabb_getMaxX(col1)), avdl_collider_aabb_getMinX(col1)),
			dd_math_max(dd_math_min(avdl_vec4_Y(&vertex2), avdl_collider_aabb_getMaxY(col1)), avdl_collider_aabb_getMinY(col1)),
			dd_math_max(dd_math_min(avdl_vec4_Z(&vertex2), avdl_collider_aabb_getMaxZ(col1)), avdl_collider_aabb_getMinZ(col1)),
			1
		);

		float distance = avdl_vec4_Distance(&closest_point, &vertex2);

		// TODO:
		// Closest point should be on surface of box
		// if sphere is inside box, do a different calculation for overlap
		if (distance < rad2f) {

			// distance 0 is edge case?
			if (collision) {
				avdl_vec4_Setf(&collision->overlap,
					avdl_vec4_X(&closest_point) -avdl_vec4_X(&vertex2),
					avdl_vec4_Y(&closest_point) -avdl_vec4_Y(&vertex2),
					avdl_vec4_Z(&closest_point) -avdl_vec4_Z(&vertex2),
					avdl_vec4_W(&closest_point) -avdl_vec4_W(&vertex2)
				);
				avdl_vec4_MultiplyMatrix(&collision->overlap, avdl_node_GetGlobalNormalMatrix(n1));
				// here it becomes nan
				avdl_vec4_Normalise(&collision->overlap);
				avdl_vec4_Multiply1f(&collision->overlap, rad2f -distance);
				if (distance <= 0) {
					avdl_log("Edge case!");
					return 0;
				}
			}
			return 1;
		}

		return 0;
	}
	else
	if (o1->type == AVDL_COLLIDER_TYPE_SPHERE && o2->type == AVDL_COLLIDER_TYPE_AABB) {
		return avdl_collider_collisionNode(o2, n2, o1, n1, collision);
	}
	else
	if (o1->type == AVDL_COLLIDER_TYPE_TERRRAIN && o2->type == AVDL_COLLIDER_TYPE_POINT) {
		//avdl_log("terrain point collision");

		// get player's position
		struct avdl_vec4 vertex2;
		avdl_vec4_Setf(&vertex2, 0, 0, 0, 1);
		avdl_vec4_MultiplyMatrix(&vertex2, avdl_node_GetGlobalMatrix(n2));
		avdl_vec4_MultiplyMatrix(&vertex2, avdl_node_GetGlobalInverseMatrix(n1));

		struct avdl_terrain *t;
		t = avdl_collider_terrain_GetTerrain(o1);

		float terrainSpot = avdl_terrain_getSpot(t, avdl_vec4_X(&vertex2), -avdl_vec4_Z(&vertex2));

		//avdl_log("player position:");
		//avdl_vec3_Print(&vertex2);
		//avdl_log("terrain spot: %f", terrainSpot);
		if (avdl_vec4_Y(&vertex2) < terrainSpot) {
			avdl_vec4_Setf(&collision->overlap, 0, 0, 0, 0);
			avdl_vec4_SetY(&collision->overlap, terrainSpot -avdl_vec4_Y(&vertex2));

			avdl_terrain_getNormal(t, avdl_vec4_X(&vertex2), -avdl_vec4_Z(&vertex2), &collision->normal1);
			avdl_vec3_Setf(&collision->normal2, 0, 0, 0);
			return 1;
		}

		return 0;
	}
	else
	if (o1->type == AVDL_COLLIDER_TYPE_POINT && o2->type == AVDL_COLLIDER_TYPE_TERRRAIN) {
		if (!avdl_collider_collisionNode(o2, n2, o1, n1, collision)) {
			return 0;
		}

		// swap data
		avdl_vec4_Invert(&collision->overlap);

		struct avdl_vec3 temp;
		avdl_vec3_Set(&temp, &collision->normal1);
		avdl_vec3_Set(&collision->normal1, &collision->normal2);
		avdl_vec3_Set(&collision->normal2, &temp);

		return 1;
	}

	avdl_log("collision not supported");
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
