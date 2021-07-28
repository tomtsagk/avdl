#include "dd_mesh.h"
#include "dd_filetomesh.h"
#include "dd_dynamic_array.h"
#include "dd_log.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#if DD_PLATFORM_ANDROID
int dd_loadstring_ply(struct dd_loaded_mesh *m, const char *asset, int settings);

/* Select the right function depending on file_type */
int dd_filetomesh(struct dd_loaded_mesh *m, const char *asset, int settings, int file_type) {

	switch (file_type) {
		case DD_PLY: return dd_loadstring_ply(m, asset, settings);
	}

	return -1;
}

/* Parse PLY string */
int dd_loadstring_ply(struct dd_loaded_mesh *m, const char *asset, int settings) {

	/*
	//Open file and check error
	FILE *f = fopen(path, "r");
	if (!f)
	{
		fail = 1;
		//dd_log("load_ply: error opening file: %s", path);
		//return -1;
	}
	*/

	const char *f = asset;
	int charRead;

	//Buffer
	char buff[1024];

	//Data for parsing
	unsigned int vertices = 0, faces = 0;

	struct dd_vec3 {
		float x, y, z;
	};

	struct dd_vec2 {
		float x, y;
	};

	/* Vertex positions sorted by index (as they are read)
	 * and by face (what the final array should look like to render the mesh)
	 */
	struct dd_vec3 *v_pos_index = 0;
	struct dd_vec3 *v_col_index = 0;
	int *v_col_index_d = 0;
	struct dd_vec2 *v_tex_index = 0;

	int MAX_FACE_VERTICES = 3;
	unsigned int *v_pos_face2 = 0;

	struct dd_dynamic_array v_pos_face;
	dd_da_init(&v_pos_face , sizeof(struct dd_vec3));

	struct dd_dynamic_array v_col_face;
	dd_da_init(&v_col_face , sizeof(struct dd_vec3));

	struct dd_dynamic_array v_tex_face;
	dd_da_init(&v_tex_face , sizeof(struct dd_vec2));

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

	struct ply_format_desc {
		char *name;
		enum ply_format format;
		char *parseSymbol;
	} format_description[] = {
		{"char", PLY_FORMAT_CHAR, "hhd"},
		{"uchar", PLY_FORMAT_UCHAR, "hhu"},
		{"short", PLY_FORMAT_SHORT, "hd"},
		{"ushort", PLY_FORMAT_USHORT, "hu"},
		{"int", PLY_FORMAT_INT, "d"},
		{"uint", PLY_FORMAT_UINT, "u"},
		{"float", PLY_FORMAT_FLOAT, "f"},
		{"double", PLY_FORMAT_DOUBLE, "lf"},
		{"none", PLY_FORMAT_NONE, ""},
	};
	int formats_total = sizeof(format_description) /sizeof(struct ply_format_desc);

	struct ply_property {
		char name[100];
		enum ply_format format;
		void *target;
		int offsetSize;

		enum ply_format list_format;
	};

	struct ply_element {
		char name[100];
		struct ply_property p[100];
		int propertyCurrent;
		int amount;
	};

	struct ply_element elements[10];

	int elementCurrent = -1;

	/* Check ply magic number:
	 *		Get 3 first chars. (ignore new line char)
	 *		Add terminating byte.
	 *		Compare with "ply".
	 */
	if ( sscanf(f, "%3c%*1c%n", buff, &charRead) == EOF) {
		dd_log("end of file?");
		goto error;
	}
	f += charRead;

	buff[3] = '\0';
	if ( strcmp(buff, "ply") != 0) {
		dd_log("not ply file: %s", buff);
		goto error;
	}

	//Check format (let's skip it for now, assume "ascii 1.0")

	/* What this parser reads:
	 *		element vertex
				x, y, z
				red, green, blue
	 *		element face
				property vertex_indices
	 */

	//Read words until end of file or end of header
	while ( sscanf(f, "%s%n", buff, &charRead) != EOF
		&& strcmp(buff, "end_header") != 0
		&& f[0] != '\0')
	{
		f += charRead;
		// Found comment - Skip line
		if ( strcmp(buff, "comment") == 0 ) {
			if ( sscanf(f, "%*[^\n]%*1c%n", &charRead) == EOF) {
				goto error;
			}
			f += charRead;
		}
		else
		// Found an element
		if ( strcmp(buff, "element") == 0 ) {
			elementCurrent++;
			elements[elementCurrent].propertyCurrent = -1;

			//Get next word
			if (sscanf(f, "%s%n", buff, &charRead) == EOF ) {
				goto error;
			}
			f += charRead;

			strncpy(elements[elementCurrent].name, buff, 99);
			elements[elementCurrent].name[99] = '\0';

			//Found vertices - save number of vertices
			if ( strcmp(buff, "vertex") == 0 ) {
				if ( sscanf(f, "%u%n", &vertices, &charRead) == EOF ) {
					goto error;
				}
				f += charRead;
				elements[elementCurrent].amount = vertices;
				v_pos_index = malloc(sizeof(struct dd_vec3) *vertices);
				v_col_index = malloc(sizeof(struct dd_vec3) *vertices);
				v_col_index_d = malloc(sizeof(int) *vertices *3);
				memset(v_col_index_d, 0, sizeof(int) *vertices *3);
				v_tex_index = malloc(sizeof(struct dd_vec2) *vertices);
			}
			else
			//Found faces - save number of faces
			if ( strcmp(buff, "face") == 0 ) {
				if ( sscanf(f, "%u%n", &faces, &charRead) == EOF ) {
					goto error;
				}
				f += charRead;
				elements[elementCurrent].amount = faces;
				v_pos_face2 = malloc(sizeof(unsigned int) *faces *MAX_FACE_VERTICES);
				memset(v_pos_face2, 0, sizeof(unsigned int) *faces *MAX_FACE_VERTICES);
			}
		}
		else
		// Found a property
		if ( strcmp(buff, "property") == 0 ) {
			struct ply_element *element = &elements[elementCurrent];
			element->propertyCurrent++;
			struct ply_property *property = &element->p[element->propertyCurrent];

			// Get format/list
			if ( sscanf(f, "%s%n", buff, &charRead) == EOF ) {
				goto error;
			}
			f += charRead;

			// if list, get list's format
			property->list_format = PLY_FORMAT_NONE;
			if (strcmp(buff, "list") == 0) {
				if ( sscanf(f, "%s%n", buff, &charRead) == EOF ) {
					goto error;
				}
				f += charRead;

				for (int i = 0; i < formats_total; i++) {
					if (strcmp(buff, format_description[i].name) == 0) {
						property->list_format = format_description[i].format;
						break;
					}
				}

				// next word is format
				if ( sscanf(f, "%s%n", buff, &charRead) == EOF ) {
					goto error;
				}
				f += charRead;
			}

			// get format or property
			property->format = PLY_FORMAT_NONE;
			for (int i = 0; i < formats_total; i++) {
				if (strcmp(buff, format_description[i].name) == 0) {
					property->format = format_description[i].format;
					break;
				}
			}

			// Get property name
			if ( sscanf(f, "%s%n", buff, &charRead) == EOF ) {
				goto error;
			}
			f += charRead;

			strncpy(property->name, buff, 99);
			property->name[99] = '\0';

			// attach property to one of the wanted values
			property->target = 0;
			if (strcmp(element->name, "vertex") == 0) {
				if (strcmp(property->name, "x") == 0) {
					property->target = &v_pos_index->x;
					property->offsetSize = sizeof(struct dd_vec3);
				}
				else
				if (strcmp(property->name, "y") == 0) {
					property->target = &v_pos_index->y;
					property->offsetSize = sizeof(struct dd_vec3);
				}
				else
				if (strcmp(property->name, "z") == 0) {
					property->target = &v_pos_index->z;
					property->offsetSize = sizeof(struct dd_vec3);
				}
				else
				if (strcmp(property->name, "s") == 0) {
					property->target = &v_tex_index->x;
					property->offsetSize = sizeof(struct dd_vec2);
				}
				else
				if (strcmp(property->name, "t") == 0) {
					property->target = &v_tex_index->y;
					property->offsetSize = sizeof(struct dd_vec2);
				}
				else
				if (strcmp(property->name, "red") == 0) {
					property->target = v_col_index_d;
					property->offsetSize = sizeof(int) *3;
				}
				else
				if (strcmp(property->name, "green") == 0) {
					property->target = v_col_index_d +1;
					property->offsetSize = sizeof(int) *3;
				}
				else
				if (strcmp(property->name, "blue") == 0) {
					property->target = v_col_index_d +2;
					property->offsetSize = sizeof(int) *3;
				}
			}
			else
			if (strcmp(element->name, "face") == 0) {
				if (strcmp(property->name, "vertex_indices") == 0) {
					property->target = v_pos_face2;
					property->offsetSize = sizeof(unsigned int);
				}
			}
		}
	}
	f += charRead;

	// print elements and properties
	for (int i = 0; i <= elementCurrent; i++) {
		struct ply_element *element = &elements[i];
		//dd_log("element %s amount: %d", element->name, element->amount);

		for (int p = 0; p < element->amount; p++) {

			for (int j = 0; j <= element->propertyCurrent; j++) {
				struct ply_property *property = &element->p[j];
				//if (i == 1) dd_log("\tproperty: %s %s", property->name, format_description[property->format].name);

				int iterator = 1;
				// is list
				if (property->list_format != PLY_FORMAT_NONE) {
					//dd_log("\t\tis list: %s", format_description[property->list_format].name);

					if (property->list_format == PLY_FORMAT_FLOAT
					||  property->list_format == PLY_FORMAT_DOUBLE) {
						//dd_log("dd_filetomesh.c: %s error parsing: list is float or double", path);
						//exit(-1);
					}

					strcpy(buff, "%");
					strcat(buff, format_description[property->list_format].parseSymbol);
					strcat(buff, "%n");
					sscanf(f, buff, &iterator, &charRead);

					f += charRead;
					if (iterator > 3) {
						//dd_log("dd_filetomesh.c: %s error parsing: only triangulated meshes are supported", path);
						//exit(-1);
					}
				}

				for (int list_iterator = 0; list_iterator < iterator; list_iterator++) {
					// has target
					if (property->target) {
						strcpy(buff, "%");
						strcat(buff, format_description[property->format].parseSymbol);
						strcat(buff, "%n");
						//if (i == 1) dd_log("\tparsing %s", buff);
						//fscanf(f, buff, (char*) property->target +(j *property->offsetSize +list_iterator));
						sscanf(f, buff, (char*) property->target +(p *(property->offsetSize *iterator)) +(list_iterator *property->offsetSize), &charRead);
						f += charRead;
						if (property->format == PLY_FORMAT_FLOAT) {
							//dd_log("parsed float: %f", ((float *) property->target)[p]);
						}
						else
						if (property->format == PLY_FORMAT_UCHAR) {
							//dd_log("parsed uchar: %d", ((unsigned char *) property->target)[p]);
						}
						else
						if (property->format == PLY_FORMAT_UINT) {
							//dd_log("parsed uint: %d", ((unsigned int *) property->target)[0]);
						}
					}
					else {
						strcpy(buff, "%*");
						strcat(buff, format_description[property->format].parseSymbol);
						strcat(buff, "%n");
						sscanf(f, buff, &charRead);
						f += charRead;
						//dd_log("\tskipping %s", buff);
					}
					//dd_log("");
				}
			}
		}
	}

	for (int i = 0; i < vertices; i++) {
		int colr = v_col_index_d[i*3+0];
		int colg = v_col_index_d[i*3+1];
		int colb = v_col_index_d[i*3+2];
		struct dd_vec3 *cvec = &v_col_index[i];
		cvec->x = colr /255.0;
		cvec->y = colg /255.0;
		cvec->z = colb /255.0;
	}

	for (int i = 0; i < faces *3; i++) {
		struct dd_vec3 vertex;
		vertex.x = v_pos_index[v_pos_face2[i]].x;
		vertex.y = v_pos_index[v_pos_face2[i]].y;
		vertex.z = v_pos_index[v_pos_face2[i]].z;
		dd_da_add(&v_pos_face, &vertex);

		struct dd_vec3 cvertex;
		cvertex.x = v_col_index[v_pos_face2[i]].x;
		cvertex.y = v_col_index[v_pos_face2[i]].y;
		cvertex.z = v_col_index[v_pos_face2[i]].z;
		dd_da_add(&v_col_face, &cvertex);

		struct dd_vec2 tex;
		tex.x = v_tex_index[v_pos_face2[i]].x;
		tex.y = v_tex_index[v_pos_face2[i]].y;
		dd_da_add(&v_tex_face, &tex);
	}
/*
	//Close file
	fclose(f);
	*/

	// cleanup
	free(v_pos_face2);
	free(v_pos_index);
	free(v_col_index);
	free(v_col_index_d);
	free(v_tex_index);

	//Mesh to return
	m->vcount = v_pos_face.elements;
	//m->vcount = 5;
	if (settings & DD_FILETOMESH_SETTINGS_POSITION) {
		m->v = malloc(sizeof(float) *m->vcount *3);

		/*
		m->v[0] = 1.5;
		m->v[1] = 0.5;
		m->v[2] = 0;

		m->v[3] = -0.5;
		m->v[4] = 0.5;
		m->v[5] = 0;

		m->v[6] = 0;
		m->v[7] = -0.5;
		m->v[8] = 0;
		*/
	}
	else {
		m->v = 0;
	}

	if (settings & DD_FILETOMESH_SETTINGS_COLOUR) {
		m->c = malloc(sizeof(float) *m->vcount *4);

		/*
		m->c[0] = 1.0;
		m->c[1] = 0.0;
		m->c[2] = 1.0;
		m->c[3] = 0.0;

		m->c[4] = 0.0;
		m->c[5] = 1.0;
		m->c[6] = 1.0;
		m->c[7] = 0.0;

		m->c[8] = 1.0;
		m->c[9] = 1.0;
		m->c[10] = 0.0;
		m->c[11] = 0.0;
		*/
	}
	else {
		m->c = 0;
	}

	if (settings & DD_FILETOMESH_SETTINGS_TEX_COORD) {
		m->t = malloc(sizeof(float) *m->vcount *2);
	}
	else {
		m->t = 0;
	}

	for (unsigned int i = 0; i < m->vcount; i++) {
		if (settings & DD_FILETOMESH_SETTINGS_POSITION) {
			struct dd_vec3 *vec = dd_da_get(&v_pos_face, i);
			m->v[(i*3)] = vec->x;
			m->v[(i*3)+1] = vec->y;
			m->v[(i*3)+2] = vec->z;
		}

		if (settings & DD_FILETOMESH_SETTINGS_COLOUR) {
			struct dd_vec3 *col = dd_da_get(&v_col_face, i);
			m->c[(i*4)] = col->x;
			m->c[(i*4)+1] = col->y;
			m->c[(i*4)+2] = col->z;
			m->c[(i*4)+3] = 0;
		}

		if (settings & DD_FILETOMESH_SETTINGS_TEX_COORD) {
			struct dd_vec2 *tex = dd_da_get(&v_tex_face, i);
			m->t[(i*2)] = tex->x;
			m->t[(i*2)+1] = tex->y;
		}
	}

	dd_da_free(&v_pos_face);
	dd_da_free(&v_col_face);
	dd_da_free(&v_tex_face);

	//Success!
	return 0;

	//Error handling
	error:
	dd_log("error loading file");
	/*
	if (ferror(f)) {
		dd_log("load_ply: error while parsing %s: %s", path,
			strerror(errno));
	} else
	if (feof(f)) {
		dd_log("load_ply: unexpected end of file on %s", path);
	}
	else {
		dd_log("load_ply: unexpected error on %s", path);
	}
	fclose(f);
	*/
	return -1;
}
#else

/* Different functions for each file */
int dd_load_ply(struct dd_loaded_mesh *m, const char *path, int settings);
int dd_load_obj(struct dd_loaded_mesh *m, const char *path, int settings);

/* Select the right function depending on file_type */
int dd_filetomesh(struct dd_loaded_mesh *m, const char *path, int settings, int file_type) {

	switch (file_type) {
		case DD_PLY: return dd_load_ply(m, path, settings);
		case DD_OBJ: return dd_load_obj(m, path, settings);
	}

	return -1;
}

/* Parse PLY - STILL WORKING ON IT */
/*
 */
int dd_load_ply(struct dd_loaded_mesh *m, const char *path, int settings) {

	//Open file and check error
	FILE *f = fopen(path, "r");
	if (!f)
	{
		dd_log("load_ply: error opening file: %s", path);
		return -1;
	}

	//Buffer
	char buff[1024];

	//Data for parsing
	unsigned int vertices = 0, faces = 0;

	struct dd_vec3 {
		float x, y, z;
	};

	struct dd_vec2 {
		float x, y;
	};

	/* Vertex positions sorted by index (as they are read)
	 * and by face (what the final array should look like to render the mesh)
	 */
	struct dd_vec3 *v_pos_index = 0;
	struct dd_vec3 *v_col_index = 0;
	int *v_col_index_d = 0;
	struct dd_vec2 *v_tex_index = 0;

	int MAX_FACE_VERTICES = 3;
	unsigned int *v_pos_face2 = 0;

	struct dd_dynamic_array v_pos_face;
	dd_da_init(&v_pos_face , sizeof(struct dd_vec3));

	struct dd_dynamic_array v_col_face;
	dd_da_init(&v_col_face , sizeof(struct dd_vec3));

	struct dd_dynamic_array v_tex_face;
	dd_da_init(&v_tex_face , sizeof(struct dd_vec2));

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

	struct ply_format_desc {
		char *name;
		enum ply_format format;
		char *parseSymbol;
	} format_description[] = {
		{"char", PLY_FORMAT_CHAR, "hhd"},
		{"uchar", PLY_FORMAT_UCHAR, "hhu"},
		{"short", PLY_FORMAT_SHORT, "hd"},
		{"ushort", PLY_FORMAT_USHORT, "hu"},
		{"int", PLY_FORMAT_INT, "d"},
		{"uint", PLY_FORMAT_UINT, "u"},
		{"float", PLY_FORMAT_FLOAT, "f"},
		{"double", PLY_FORMAT_DOUBLE, "lf"},
		{"none", PLY_FORMAT_NONE, ""},
	};
	int formats_total = sizeof(format_description) /sizeof(struct ply_format_desc);

	struct ply_property {
		char name[100];
		enum ply_format format;
		void *target;
		int offsetSize;

		enum ply_format list_format;
	};

	struct ply_element {
		char name[100];
		struct ply_property p[100];
		int propertyCurrent;
		int amount;
	};

	struct ply_element elements[100];

	int elementCurrent = -1;

	/* Check ply magic number:
	 *		Get 3 first chars. (ignore new line char)
	 *		Add terminating byte.
	 *		Compare with "ply".
	 */
	if ( fscanf(f, "%3c%*1c", buff) == EOF)
		goto error;

	buff[3] = '\0';
	if ( strcmp(buff, "ply") != 0) goto error;

	//Check format (let's skip it for now, assume "ascii 1.0")

	/* What this parser reads:
	 *		element vertex
	 			x, y, z
				red, green, blue
	 *		element face
				property vertex_indices
	 */

	//Read words until end of file or end of header
	while ( fscanf(f, "%s", buff) != EOF
		&& strcmp(buff, "end_header") != 0 )
	{
		// Found comment - Skip line
		if ( strcmp(buff, "comment") == 0 ) {
			if ( fscanf(f, "%*[^\n]%*1c") == EOF) {
				goto error;
			}
		}
		else
		// Found an element
		if ( strcmp(buff, "element") == 0 ) {
			elementCurrent++;
			elements[elementCurrent].propertyCurrent = -1;

			//Get next word
			if ( fscanf(f, "%s", buff) == EOF )
				goto error;

			strncpy(elements[elementCurrent].name, buff, 99);
			elements[elementCurrent].name[99] = '\0';

			//Found vertices - save number of vertices
			if ( strcmp(buff, "vertex") == 0 ) {
				if ( fscanf(f, "%u", &vertices) == EOF ) {
					goto error;
				}
				elements[elementCurrent].amount = vertices;
				v_pos_index = malloc(sizeof(struct dd_vec3) *vertices);
				v_col_index = malloc(sizeof(struct dd_vec3) *vertices);
				v_col_index_d = malloc(sizeof(int) *vertices *3);
				memset(v_col_index_d, 0, sizeof(int) *vertices *3);
				v_tex_index = malloc(sizeof(struct dd_vec2) *vertices);
			}
			else
			//Found faces - save number of faces
			if ( strcmp(buff, "face") == 0 ) {
				if ( fscanf(f, "%u", &faces) == EOF ) {
					goto error;
				}
				elements[elementCurrent].amount = faces;
				v_pos_face2 = malloc(sizeof(unsigned int) *faces *MAX_FACE_VERTICES);
				memset(v_pos_face2, 0, sizeof(unsigned int) *faces *MAX_FACE_VERTICES);
			}
		}
		else
		// Found a property
		if ( strcmp(buff, "property") == 0 ) {
			struct ply_element *element = &elements[elementCurrent];
			element->propertyCurrent++;
			struct ply_property *property = &element->p[element->propertyCurrent];

			// Get format/list
			if ( fscanf(f, "%s", buff) == EOF )
				goto error;

			// if list, get list's format
			property->list_format = PLY_FORMAT_NONE;
			if (strcmp(buff, "list") == 0) {
				if ( fscanf(f, "%s", buff) == EOF )
					goto error;

				for (int i = 0; i < formats_total; i++) {
					if (strcmp(buff, format_description[i].name) == 0) {
						property->list_format = format_description[i].format;
						break;
					}
				}

				// next word is format
				if ( fscanf(f, "%s", buff) == EOF )
					goto error;
			}

			// get format or property
			property->format = PLY_FORMAT_NONE;
			for (int i = 0; i < formats_total; i++) {
				if (strcmp(buff, format_description[i].name) == 0) {
					property->format = format_description[i].format;
					break;
				}
			}

			// Get property name
			if ( fscanf(f, "%s", buff) == EOF )
				goto error;

			strncpy(property->name, buff, 99);
			property->name[99] = '\0';

			// attach property to one of the wanted values
			property->target = 0;
			if (strcmp(element->name, "vertex") == 0) {
				if (strcmp(property->name, "x") == 0) {
					property->target = &v_pos_index->x;
					property->offsetSize = sizeof(struct dd_vec3);
				}
				else
				if (strcmp(property->name, "y") == 0) {
					property->target = &v_pos_index->y;
					property->offsetSize = sizeof(struct dd_vec3);
				}
				else
				if (strcmp(property->name, "z") == 0) {
					property->target = &v_pos_index->z;
					property->offsetSize = sizeof(struct dd_vec3);
				}
				else
				if (strcmp(property->name, "s") == 0) {
					property->target = &v_tex_index->x;
					property->offsetSize = sizeof(struct dd_vec2);
				}
				else
				if (strcmp(property->name, "t") == 0) {
					property->target = &v_tex_index->y;
					property->offsetSize = sizeof(struct dd_vec2);
				}
				else
				if (strcmp(property->name, "red") == 0) {
					property->target = v_col_index_d;
					property->offsetSize = sizeof(int) *3;
				}
				else
				if (strcmp(property->name, "green") == 0) {
					property->target = v_col_index_d +1;
					property->offsetSize = sizeof(int) *3;
				}
				else
				if (strcmp(property->name, "blue") == 0) {
					property->target = v_col_index_d +2;
					property->offsetSize = sizeof(int) *3;
				}
			}
			else
			if (strcmp(element->name, "face") == 0) {
				if (strcmp(property->name, "vertex_indices") == 0) {
					property->target = v_pos_face2;
					property->offsetSize = sizeof(unsigned int);
				}
			}
		}
	}

	// print elements and properties
	for (int i = 0; i <= elementCurrent; i++) {
		struct ply_element *element = &elements[i];

		for (int p = 0; p < element->amount; p++) {

			for (int j = 0; j <= element->propertyCurrent; j++) {
				struct ply_property *property = &element->p[j];

				int iterator = 1;
				// is list
				if (property->list_format != PLY_FORMAT_NONE) {

					if (property->list_format == PLY_FORMAT_FLOAT
					||  property->list_format == PLY_FORMAT_DOUBLE) {
						dd_log("dd_filetomesh.c: %s error parsing: list is float or double", path);
						exit(-1);
					}

					strcpy(buff, "%");
					strcat(buff, format_description[property->list_format].parseSymbol);
					fscanf(f, buff, &iterator);

					if (iterator > 3) {
						dd_log("dd_filetomesh.c: %s error parsing: only triangulated meshes are supported", path);
						exit(-1);
					}
				}

				for (int list_iterator = 0; list_iterator < iterator; list_iterator++) {
					// has target
					if (property->target) {
						strcpy(buff, "%");
						strcat(buff, format_description[property->format].parseSymbol);
						fscanf(f, buff, (char*) property->target +(p *(property->offsetSize *iterator)) +(list_iterator *property->offsetSize));
					}
					// skipping
					else {
						strcpy(buff, "%*");
						strcat(buff, format_description[property->format].parseSymbol);
						fputs(buff, f);
					}
				}
			}
		}
	}

	for (int i = 0; i < vertices; i++) {
		int colr = v_col_index_d[i*3+0];
		int colg = v_col_index_d[i*3+1];
		int colb = v_col_index_d[i*3+2];
		struct dd_vec3 *cvec = &v_col_index[i];
		cvec->x = colr /255.0;
		cvec->y = colg /255.0;
		cvec->z = colb /255.0;
	}

	for (int i = 0; i < faces *3; i++) {
		struct dd_vec3 vertex;
		vertex.x = v_pos_index[v_pos_face2[i]].x;
		vertex.y = v_pos_index[v_pos_face2[i]].y;
		vertex.z = v_pos_index[v_pos_face2[i]].z;
		dd_da_add(&v_pos_face, &vertex);

		struct dd_vec3 cvertex;
		cvertex.x = v_col_index[v_pos_face2[i]].x;
		cvertex.y = v_col_index[v_pos_face2[i]].y;
		cvertex.z = v_col_index[v_pos_face2[i]].z;
		dd_da_add(&v_col_face, &cvertex);

		struct dd_vec2 tex;
		tex.x = v_tex_index[v_pos_face2[i]].x;
		tex.y = v_tex_index[v_pos_face2[i]].y;
		dd_da_add(&v_tex_face, &tex);
	}

	//Close file
	fclose(f);

	// cleanup
	free(v_pos_face2);
	free(v_pos_index);
	free(v_col_index);
	free(v_col_index_d);
	free(v_tex_index);

	//Mesh to return
	m->vcount = v_pos_face.elements;
	if (settings & DD_FILETOMESH_SETTINGS_POSITION) {
		m->v = malloc(sizeof(float) *m->vcount *3);
	}
	else {
		m->v = 0;
	}

	if (settings & DD_FILETOMESH_SETTINGS_COLOUR) {
		m->c = malloc(sizeof(float) *m->vcount *3);
	}
	else {
		m->c = 0;
	}

	if (settings & DD_FILETOMESH_SETTINGS_TEX_COORD) {
		m->t = malloc(sizeof(float) *m->vcount *2);
	}
	else {
		m->t = 0;
	}

	for (unsigned int i = 0; i < v_pos_face.elements; i++) {
		if (settings & DD_FILETOMESH_SETTINGS_POSITION) {
			struct dd_vec3 *vec = dd_da_get(&v_pos_face, i);
			m->v[(i*3)] = vec->x;
			m->v[(i*3)+1] = vec->y;
			m->v[(i*3)+2] = vec->z;
		}

		if (settings & DD_FILETOMESH_SETTINGS_COLOUR) {
			struct dd_vec3 *col = dd_da_get(&v_col_face, i);
			m->c[(i*3)] = col->x;
			m->c[(i*3)+1] = col->y;
			m->c[(i*3)+2] = col->z;
		}

		if (settings & DD_FILETOMESH_SETTINGS_TEX_COORD) {
			struct dd_vec2 *tex = dd_da_get(&v_tex_face, i);
			m->t[(i*2)] = tex->x;
			m->t[(i*2)+1] = tex->y;
		}
	}

	dd_da_free(&v_pos_face);
	dd_da_free(&v_col_face);
	dd_da_free(&v_tex_face);

	//Success!
	return 0;

	//Error handling
	error:
	if (ferror(f)) {
		dd_log("load_ply: error while parsing %s: %s", path, strerror(errno));
	} else
	if (feof(f)) {
		dd_log("load_ply: unexpected end of file on %s", path);
	}
	else {
		dd_log("load_ply: unexpected error on %s", path);
	}
	fclose(f);
	return -1;
}

/* Parse OBJ */
int dd_load_obj(struct dd_loaded_mesh *m, const char *path, int settings) {

	//(void) attr;

	struct dd_vec3 {
		float x, y, z;
	};
	//Variables
	struct dd_dynamic_array v_ind, v_out;

	//Init dynamic arrays
	dd_da_init(&v_ind, sizeof(struct dd_vec3));
	dd_da_init(&v_out, sizeof(struct dd_vec3));

	//File
	FILE *f = fopen(path, "r");
	if ( !f ) {
		dd_log("load_obj: %s: failed to open file: %s", path, strerror(errno));
		goto error;
	}

	//Buffer
	char buff[1024];

	//Get the first character of each line
	while ( fscanf(f, "%s", buff) != EOF ) {
		//Vertex
		if (buff[0] == 'v') {
			//Get vertex xyz
			struct dd_vec3 v;
			if ( fscanf(f, "%f %f %f", &v.x, &v.y, &v.z) == EOF ) {
				goto error;
			}
			//dd_log("parse vertex %f %f %f", v.x, v.y, v.z);

			//Check mirroring
			/*
			if (mirror & 1) v.x = -v.x;
			if (mirror & 2) v.y = -v.y;
			if (mirror & 4) v.z = -v.z;
			*/

			//Push vertex
			dd_da_add(&v_ind, &v);

			//Skip until next line
			fscanf(f, "%*[^\n]%*1c");
		} else
		//Face
		if (buff[0] == 'f') {

			//Assumes each face has at least 3 vertices
			int base, last, new;
			fscanf(f, "%d %d %d", &base, &last, &new);
			//dd_log("parse face %d %d %d", base, last, new);

			/* every face minus one, because obj starts from 1 */
			base--;
			last--;
			new--;

			/* add first face (assuming each face has at least 3 vertices) */
			dd_da_add(&v_out, dd_da_get(&v_ind, base));
			dd_da_add(&v_out, dd_da_get(&v_ind, last));
			dd_da_add(&v_out, dd_da_get(&v_ind, new ));

			/* for each extra vertex, add a new face, like a fan */
			int vert_ind;
			while( fscanf(f, "%d", &vert_ind) && !feof(f) ) {
				vert_ind--;
				last = new;
				new = vert_ind;
				//dd_log("parse extra face %d %d %d", base, last, new);

				dd_da_add(&v_out, dd_da_get(&v_ind, base));
				dd_da_add(&v_out, dd_da_get(&v_ind, last));
				dd_da_add(&v_out, dd_da_get(&v_ind, new ));

			}
		}
		//Unsupported element
		else {
			//Skip line
			if ( fscanf(f, "%[^\n]%*1c", buff) == EOF ) {
				goto error;
			}
		}
	}

	//Close file
	fclose(f);

	//Create mesh_data
	m->vcount = v_out.elements *3;
	m->v = malloc(sizeof(float) *v_out.elements *3);
	for (unsigned int i = 0; i < v_out.elements; i++) {
		struct dd_vec3 *vec = dd_da_get(&v_out, i);
		m->v[(i*3)] = vec->x;
		m->v[(i*3)+1] = vec->y;
		m->v[(i*3)+2] = vec->z;
	}

	//dd_mesh_to_vbo(m);

	return 0;

	error:
	if (ferror(f)) {
		dd_log("load_obj: error while parsing %s: %s", path, strerror(errno));
	} else
	if (feof(f)) {
		dd_log("load_obj: unexpected end of file on %s", path);
	}
	else {
		dd_log("load_obj: unexpected error on %s", path);
	}
	fclose(f);

	//dd_log("stop parsing");
	return -1;
}

#endif
