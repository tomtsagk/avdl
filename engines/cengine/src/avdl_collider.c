#include "avdl_collider.h"
#include "avdl_collider_aabb.h"
#include "dd_log.h"
#include "dd_math.h"

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
		struct dd_vec4 vertices1[8];
		struct dd_vec4 vertices2[8];
		dd_vec4_set(&vertices1[0],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		dd_vec4_set(&vertices1[1],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		dd_vec4_set(&vertices1[2],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		dd_vec4_set(&vertices1[3],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		dd_vec4_set(&vertices1[4],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		dd_vec4_set(&vertices1[5],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		dd_vec4_set(&vertices1[6],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		dd_vec4_set(&vertices1[7],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		dd_vec4_set(&vertices2[0],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		dd_vec4_set(&vertices2[1],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		dd_vec4_set(&vertices2[2],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		dd_vec4_set(&vertices2[3],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		dd_vec4_set(&vertices2[4],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		dd_vec4_set(&vertices2[5],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		dd_vec4_set(&vertices2[6],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		dd_vec4_set(&vertices2[7],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		for (int i = 0; i < 8; i++) {
			dd_vec4_multiply(&vertices1[i], m1);
			dd_vec4_multiply(&vertices2[i], m2);
		}

/*
		struct dd_vec4 min1;
		dd_vec4_set(&min1,
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		dd_vec4_multiply(&min1, m1);
		struct dd_vec4 max1;
		dd_vec4_set(&max1,
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		dd_vec4_multiply(&max1, m1);
		dd_log("min/max 1");
		dd_vec4_print(&min1);
		dd_vec4_print(&max1);

		struct dd_vec4 min2;
		dd_vec4_set(&min2,
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		dd_vec4_multiply(&min2, m2);
		struct dd_vec4 max2;
		dd_vec4_set(&max2,
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		dd_vec4_multiply(&max2, m2);
		dd_log("min/max 2");
		dd_vec4_print(&min2);
		dd_vec4_print(&max2);
		*/

		// mult by 90 degrees matrix to find normal
		struct dd_matrix norm_matrix;
		dd_matrix_identity(&norm_matrix);

		// normals: up, right, front
		int total_normals = 6;
		struct dd_vec4 normals[15];
		dd_vec4_set(&normals[0], 0, 1, 0, 1);
		dd_vec4_set(&normals[1], 1, 0, 0, 1);
		dd_vec4_set(&normals[2], 0, 0, 1, 1);

		dd_vec4_set(&normals[3], 0, 1, 0, 1);
		dd_vec4_set(&normals[4], 1, 0, 0, 1);
		dd_vec4_set(&normals[5], 0, 0, 1, 1);

		for (int i = 0; i < 6; i++) {
			if (i < 3) {
				dd_vec4_multiply(&normals[i], nm1);
			}
			else {
				dd_vec4_multiply(&normals[i], nm2);
			}
		}

		for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++) {
			if (dd_vec4_getX(&normals[i]) == dd_vec4_getX(&normals[3 +j])
			&&  dd_vec4_getY(&normals[i]) == dd_vec4_getY(&normals[3 +j])
			&&  dd_vec4_getZ(&normals[i]) == dd_vec4_getZ(&normals[3 +j])) {
				continue;
			}
			int index = total_normals;//6 +(i*3 +j);
			//dd_log("set normal index: %d", index);
			dd_vec4_set(&normals[index],
				dd_vec4_getX(&normals[i]),
				dd_vec4_getY(&normals[i]),
				dd_vec4_getZ(&normals[i]),
				dd_vec4_getW(&normals[i])
			);
			//dd_vec4_print(&normals[index]);
			//dd_vec4_print(&normals[3 +j]);
			dd_vec4_cross(&normals[index], &normals[3 +j]);
			total_normals++;
		}

		//dd_log("~~~ Normal checks");
		for (int i = 0; i < total_normals; i++) {
			//dd_log("Normal %d", i);
			if (dd_vec4_getX(&normals[i]) == 0
			&&  dd_vec4_getY(&normals[i]) == 0
			&&  dd_vec4_getZ(&normals[i]) == 0) {
				//dd_log("skip");
				continue;
			}
			//dd_vec4_print(&normals[i]);
			dd_vec4_normalise(&normals[i]);

			int haslimits = 0;
			float finalMin1 = 0;
			float finalMax1 = 0;
			float finalMin2 = 0;
			float finalMax2 = 0;

			for (int j = 0; j < 8; j++) {
				//dd_log("vertex: %d", j);
				//dd_vec4_print(&vertices1[j]);
				//dd_vec4_print(&vertices2[j]);
				// just dot it to get the min/max along this axis.
				float dotValMin1 = dd_vec4_dot(&normals[i], &vertices1[j]);
				float dotValMin2 = dd_vec4_dot(&normals[i], &vertices2[j]);

				if (!haslimits) {
					finalMin1 = dotValMin1;
					finalMax1 = dotValMin1;
					finalMin2 = dotValMin2;
					finalMax2 = dotValMin2;
					haslimits = 1;
					//dd_log("set limits %d - %f %f - %f %f", i, finalMin1, finalMax1, finalMin2, finalMax2);
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
					//dd_log("modify limits %d - %f %f - %f %f", i, finalMin1, finalMax1, finalMin2, finalMax2);
				}
			}

			//dd_log("inv %d - %f %f - %f %f", i, finalMin1, finalMax1, finalMin2, finalMax2);
			//dd_vec4_print(&normals[i]);

			// aabb collision
			if ((finalMax2 -finalMin1 > 0) != (finalMin2 -finalMax1 > 0)) {
				//dd_log("sat collide");
			}
			else {
				//dd_log("sat not collide");
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

	dd_log("collision not supported");
	return 0;
}

int avdl_collider_collisionNode(struct avdl_collider *o1, struct avdl_node *n1, struct avdl_collider *o2, struct avdl_node *n2) {

	if (o1->type == AVDL_COLLIDER_TYPE_AABB && o2->type == AVDL_COLLIDER_TYPE_AABB) {

		struct avdl_collider_aabb *col1 = o1;
		struct avdl_collider_aabb *col2 = o2;

		// collect vertices
		struct dd_vec4 vertices1[8];
		struct dd_vec4 vertices2[8];
		dd_vec4_set(&vertices1[0],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		dd_vec4_set(&vertices1[1],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		dd_vec4_set(&vertices1[2],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		dd_vec4_set(&vertices1[3],
			avdl_collider_aabb_getMinX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		dd_vec4_set(&vertices1[4],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		dd_vec4_set(&vertices1[5],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMinZ(col1),
			1
		);
		dd_vec4_set(&vertices1[6],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMinY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		dd_vec4_set(&vertices1[7],
			avdl_collider_aabb_getMaxX(col1),
			avdl_collider_aabb_getMaxY(col1),
			avdl_collider_aabb_getMaxZ(col1),
			1
		);
		dd_vec4_set(&vertices2[0],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		dd_vec4_set(&vertices2[1],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		dd_vec4_set(&vertices2[2],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		dd_vec4_set(&vertices2[3],
			avdl_collider_aabb_getMinX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		dd_vec4_set(&vertices2[4],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		dd_vec4_set(&vertices2[5],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMinZ(col2),
			1
		);
		dd_vec4_set(&vertices2[6],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMinY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		dd_vec4_set(&vertices2[7],
			avdl_collider_aabb_getMaxX(col2),
			avdl_collider_aabb_getMaxY(col2),
			avdl_collider_aabb_getMaxZ(col2),
			1
		);
		for (int i = 0; i < 8; i++) {
			dd_vec4_multiply(&vertices1[i], avdl_node_GetGlobalMatrix(n1));
			dd_vec4_multiply(&vertices2[i], avdl_node_GetGlobalMatrix(n2));
		}

		// normals: up, right, front
		int total_normals = 6;
		struct dd_vec4 normals[15];
		dd_vec4_set(&normals[0], 0, 1, 0, 1);
		dd_vec4_set(&normals[1], 1, 0, 0, 1);
		dd_vec4_set(&normals[2], 0, 0, 1, 1);

		dd_vec4_set(&normals[3], 0, 1, 0, 1);
		dd_vec4_set(&normals[4], 1, 0, 0, 1);
		dd_vec4_set(&normals[5], 0, 0, 1, 1);

		for (int i = 0; i < 6; i++) {
			if (i < 3) {
				dd_vec4_multiply(&normals[i], avdl_node_GetGlobalNormalMatrix(n1));
			}
			else {
				dd_vec4_multiply(&normals[i], avdl_node_GetGlobalNormalMatrix(n2));
			}
		}

		for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++) {
			if (dd_vec4_getX(&normals[i]) == dd_vec4_getX(&normals[3 +j])
			&&  dd_vec4_getY(&normals[i]) == dd_vec4_getY(&normals[3 +j])
			&&  dd_vec4_getZ(&normals[i]) == dd_vec4_getZ(&normals[3 +j])) {
				continue;
			}
			int index = total_normals;
			dd_vec4_set(&normals[index],
				dd_vec4_getX(&normals[i]),
				dd_vec4_getY(&normals[i]),
				dd_vec4_getZ(&normals[i]),
				dd_vec4_getW(&normals[i])
			);
			dd_vec4_cross(&normals[index], &normals[3 +j]);
			total_normals++;
		}

		for (int i = 0; i < total_normals; i++) {
			if (dd_vec4_getX(&normals[i]) == 0
			&&  dd_vec4_getY(&normals[i]) == 0
			&&  dd_vec4_getZ(&normals[i]) == 0) {
				continue;
			}
			dd_vec4_normalise(&normals[i]);

			int haslimits = 0;
			float finalMin1 = 0;
			float finalMax1 = 0;
			float finalMin2 = 0;
			float finalMax2 = 0;

			for (int j = 0; j < 8; j++) {
				// just dot it to get the min/max along this axis.
				float dotValMin1 = dd_vec4_dot(&normals[i], &vertices1[j]);
				float dotValMin2 = dd_vec4_dot(&normals[i], &vertices2[j]);

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
				//dd_log("sat collide");
			}
			else {
				//dd_log("sat not collide");
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

	dd_log("collision not supported");
	return 0;
}
