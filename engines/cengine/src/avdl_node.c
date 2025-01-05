#include "avdl_node.h"
#include "avdl_log.h"
#include "avdl_component.h"
#include "avdl_json.h"
#include "dd_vec3.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "avdl_component_mesh.h"
#include "avdl_component_custom.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void avdl_node_create(struct avdl_node *o) {
	o->parent = 0;

	o->name[0] = '\0';

	o->GetLocalTransform = avdl_node_GetLocalTransform;
	o->GetGlobalMatrix = avdl_node_GetGlobalMatrix;
	o->AddChild = avdl_node_AddChild;
	o->SetName = avdl_node_SetName;
	o->GetName = avdl_node_GetName;
	o->AddComponentsToArray = avdl_node_AddComponentsToArray;

	o->GetChildrenCount = avdl_node_GetChildrenCount;
	o->GetChild = avdl_node_GetChild;

	dd_matrix_identity(&o->globalMatrix);
	dd_matrix_identity(&o->globalNormalMatrix);
	avdl_transform_create(&o->localTransform);

	dd_dynamic_array_create(&o->components);
	dd_da_init(&o->components, sizeof(struct avdl_component *));

	dd_dynamic_array_create(&o->children);
	dd_da_init(&o->children, sizeof(struct avdl_node *));
}

void avdl_node_clean(struct avdl_node *o) {
	// clean children
	for (unsigned int i = 0; i < dd_da_count(&o->children); i++) {
		struct avdl_node *child = dd_da_getDeref(&o->children, i);
		avdl_node_clean(child);
		free(child);
	}
	dd_da_empty(&o->children);

	for (unsigned int i = 0; i < dd_da_count(&o->components); i++) {
		struct avdl_component *c = dd_da_getDeref(&o->components, i);
		avdl_component_clean(c);
		free(c);
	}
	dd_da_empty(&o->components);
}

struct avdl_transform *avdl_node_GetLocalTransform(struct avdl_node *o) {
	return &o->localTransform;
}

struct dd_matrix *avdl_node_GetGlobalMatrix(struct avdl_node *o) {
	dd_matrix_identity(&o->globalMatrix);
	if (o->parent) {
		dd_matrix_copy(&o->globalMatrix, avdl_node_GetGlobalMatrix(o->parent));
	}
	dd_matrix_mult(&o->globalMatrix, avdl_transform_GetMatrix(&o->localTransform));
	return &o->globalMatrix;
}

struct dd_matrix *avdl_node_GetGlobalNormalMatrix(struct avdl_node *o) {
	dd_matrix_identity(&o->globalNormalMatrix);
	if (o->parent) {
		dd_matrix_copy(&o->globalNormalMatrix, avdl_node_GetGlobalNormalMatrix(o->parent));
	}
	dd_matrix_mult(&o->globalNormalMatrix, avdl_transform_GetNormalMatrix(&o->localTransform));
	return &o->globalNormalMatrix;
}

struct avdl_node *avdl_node_AddChild(struct avdl_node *o) {
	struct avdl_node *child = malloc(sizeof(struct avdl_node));
	dd_da_push(&o->children, &child);
	//struct avdl_node *child = dd_da_get(&o->children, -1);
	avdl_node_create(child);
	child->parent = o;
	return child;
}

struct avdl_component *avdl_node_AddComponentInternal(struct avdl_node *o, int size, void (*constructor)(void *)) {
	struct avdl_component *c = malloc(size);
	dd_da_push(&o->components, &c);
	constructor(c);
	// each component has a reference to the node they are attached to
	c->node = o;
	return c;
}

static void avdl_node_printInternal(struct avdl_node *o, int tabs) {

	// Print tabs (if any)
	for (int i = 0; i < tabs; i++) {
		printf("\t");
	}

	if (tabs == 0) {
		printf("avdl_node:\n");
		printf("*** ");
	}
	else {
		printf("* ");
	}

	struct dd_vec3 *pos = avdl_transform_GetPosition(&o->localTransform);
	printf("%s | position %f %f %f\n", o->name[0] != '\0' ? o->name : "no_node_name", pos->x, pos->y, pos->z);
	if (dd_da_count(&o->components) > 0) {
		for (int i = 0; i < tabs+1; i++) {
			printf("\t");
		}
		for (int i = 0; i < dd_da_count(&o->components); i++) {
			printf("component: %d %x %x\n", i, dd_da_get(&o->components, i), dd_da_getDeref(&o->components, i));
		}
	}

	/*
	// Print components
	for (unsigned int i = 0; i < dd_da_count(&o->components); i++) {
		printf("	component %d\n", i);
	}
	*/

	// Print children
	for (unsigned int i = 0; i < dd_da_count(&o->children); i++) {
		struct avdl_node *child = dd_da_getDeref(&o->children, i);
		avdl_node_printInternal(child, tabs+1);
	}
}

void avdl_node_print(struct avdl_node *o) {
	avdl_node_printInternal(o, 0);
}

void avdl_node_SetName(struct avdl_node *o, char *name) {

	if (strlen(name) >= AVDL_NODE_NAME_LENGTH) {
		avdl_log("cannot copy node name, too big: %s", name);
		return;
	}

	//avdl_log("set name to : %s", name);
	strncpy(o->name, name, AVDL_NODE_NAME_LENGTH-1);
	o->name[AVDL_NODE_NAME_LENGTH-1] = '\0';
}

char *avdl_node_GetName(struct avdl_node *o) {
	if (o->name[0] == '\0') {
		return 0;
	}
	else {
		return o->name;
	}
}

void avdl_node_AddComponentsToArray(struct avdl_node *o, struct dd_dynamic_array *array, int component_type) {

	// Print components
	for (unsigned int i = 0; i < dd_da_count(&o->components); i++) {
		struct avdl_component *c = *((struct avdl_component **) dd_da_get(&o->components, i));

		if (c->type == component_type) {
			dd_da_push(array, &c);
		}
	}

	// Print children
	for (unsigned int i = 0; i < dd_da_count(&o->children); i++) {
		struct avdl_node *child = dd_da_getDeref(&o->children, i);
		avdl_node_AddComponentsToArray(child, array, component_type);
	}
}

int avdl_node_GetChildrenCount(struct avdl_node *o) {
	return dd_da_count(&o->children);
}

struct avdl_node *avdl_node_GetChild(struct avdl_node *o, int index) {
	if (index < 0 || index >= avdl_node_GetChildrenCount(o)) {
		avdl_log("avdl_node_GetChild wrong index: %d", index);
		return 0;
	}
	return dd_da_getDeref(&o->children, index);
}

static int NodeToJson_PrintTabs(int fd, int tabs) {
	char *content;
	for (int i = 0; i < tabs; i++) {
		content = "\t";
		write(fd, content, strlen(content));
	}
}

static int NodeToJson_PrintVec3(int fd, struct dd_vec3 *v) {
	char *content;
	char buffer[100];
	// x
	snprintf(buffer, 80, "%f", v->x);
	write(fd, buffer, strlen(buffer));

	content = ", ";
	write(fd, content, strlen(content));

	// y
	snprintf(buffer, 80, "%f", v->y);
	write(fd, buffer, strlen(buffer));

	content = ", ";
	write(fd, content, strlen(content));

	// z
	snprintf(buffer, 80, "%f", v->z);
	write(fd, buffer, strlen(buffer));

	return 0;
}

static int NodeToJson_PrintComponent(int fd, struct avdl_component *o, int tabs) {

	char *content;

	// start
	NodeToJson_PrintTabs(fd, tabs);
	content = "{\n";
	write(fd, content, strlen(content));

	NodeToJson_PrintTabs(fd, tabs);
	content = "\"name\": \"";
	write(fd, content, strlen(content));
	if (o->type == AVDL_COMPONENT_MESH_ENUM) {
		content = "avdl_component_mesh";
		write(fd, content, strlen(content));
		content = "\",\n";
		write(fd, content, strlen(content));
	}
	else
	if (o->type == AVDL_COMPONENT_CUSTOM_EDITOR_ENUM) {
		struct avdl_component_custom *c = o;
		content = avdl_component_custom_GetName(c);
		write(fd, content, strlen(content));
		content = "\",\n";
		write(fd, content, strlen(content));

		for (int i = 0; i+2 < dd_da_count(&c->values); i += 3) {
			struct avdl_string *varName = dd_da_getDeref(&c->values, i);
			struct avdl_string *varValue = dd_da_getDeref(&c->values, i+1);
			struct avdl_string *varType = dd_da_getDeref(&c->values, i+2);
			//avdl_log("var name and value: %s - %s", avdl_string_toCharPtr(varName), avdl_string_toCharPtr(varValue));

			NodeToJson_PrintTabs(fd, tabs);
			content = "\"";
			write(fd, content, strlen(content));
			content = avdl_string_toCharPtr(varName);
			write(fd, content, strlen(content));
			content = "\": ";
			write(fd, content, strlen(content));

			if (strcmp(avdl_string_toCharPtr(varType), "string") == 0) {
				// string
				content = "\"";
				write(fd, content, strlen(content));
				content = avdl_string_toCharPtr(varValue);
				write(fd, content, strlen(content));
				content = "\",\n";
				write(fd, content, strlen(content));
			}
			else
			if (strcmp(avdl_string_toCharPtr(varType), "int") == 0) {
				// string
				content = avdl_string_toCharPtr(varValue);
				write(fd, content, strlen(content));
				content = ",\n";
				write(fd, content, strlen(content));
			}
			else
			if (strcmp(avdl_string_toCharPtr(varType), "float") == 0) {
				// string
				content = avdl_string_toCharPtr(varValue);
				write(fd, content, strlen(content));
				content = ",\n";
				write(fd, content, strlen(content));
			}
			else {
				avdl_logError("unrecognized component variable type: %s", avdl_string_toCharPtr(varType));
				return -1;
			}
		}
	}

	// end
	NodeToJson_PrintTabs(fd, tabs);
	content = "}\n";
	write(fd, content, strlen(content));
}

static int NodeToJson_PrintNode(int fd, struct avdl_node *o, int tabs) {

	char *content;

	// start
	NodeToJson_PrintTabs(fd, tabs);
	content = "{\n";
	write(fd, content, strlen(content));

	// content
	if (avdl_node_GetName(o)) {
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "\"name\": \"";
		write(fd, content, strlen(content));
		content = avdl_node_GetName(o);
		write(fd, content, strlen(content));
		content = "\",\n";
		write(fd, content, strlen(content));
	}
	else {
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "\"name\": \"";
		write(fd, content, strlen(content));
		content = "NO_NAME";
		write(fd, content, strlen(content));
		content = "\",\n";
		write(fd, content, strlen(content));
	}

	struct avdl_transform *t = avdl_node_GetLocalTransform(o);

	// position
	struct dd_vec3 *pos = avdl_transform_GetPosition(t);
	NodeToJson_PrintTabs(fd, tabs+1);
	content = "\"position\": [ ";
	write(fd, content, strlen(content));
	NodeToJson_PrintVec3(fd, pos);
	content = " ],\n";
	write(fd, content, strlen(content));

	// rotation
	struct dd_vec3 *rot = avdl_transform_GetRotation(t);
	NodeToJson_PrintTabs(fd, tabs+1);
	content = "\"rotation\": [ ";
	write(fd, content, strlen(content));
	NodeToJson_PrintVec3(fd, rot);
	content = " ],\n";
	write(fd, content, strlen(content));

	// scale
	struct dd_vec3 *scale = avdl_transform_GetScale(t);
	NodeToJson_PrintTabs(fd, tabs+1);
	content = "\"scale\": [ ";
	write(fd, content, strlen(content));
	NodeToJson_PrintVec3(fd, scale);
	content = " ],\n";
	write(fd, content, strlen(content));

	// components
	if (dd_da_count(&o->components) > 0) {
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "\"components\": [\n";
		write(fd, content, strlen(content));
		for (int i = 0; i < dd_da_count(&o->components); i++) {
			struct avdl_component *component = dd_da_getDeref(&o->components, i);
			NodeToJson_PrintComponent(fd, component, tabs+1);
		}
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "]\n";
		write(fd, content, strlen(content));
	}

	// children
	if (dd_da_count(&o->children) > 0) {
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "\"children\": [\n";
		write(fd, content, strlen(content));
		for (int i = 0; i < dd_da_count(&o->children); i++) {
			struct avdl_node *child = dd_da_getDeref(&o->children, i);
			NodeToJson_PrintNode(fd, child, tabs+1);
		}
		NodeToJson_PrintTabs(fd, tabs+1);
		content = "]\n";
		write(fd, content, strlen(content));
	}

	// end
	NodeToJson_PrintTabs(fd, tabs);
	content = "}\n";
	write(fd, content, strlen(content));
}

int avdl_node_NodeToJson(struct avdl_node *o, char *filename) {
	avdl_log("NodeToJson at %s", filename);

	remove(filename);

	int fd = open(filename, O_RDWR | O_CREAT, 0777);
	if (fd == -1) {
		avdl_log("NodeToJson: Unable to open '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	int startingTabs = 0;
	NodeToJson_PrintNode(fd, o, startingTabs);

	close(fd);

	return 0;
}

static int json_expect_component(struct avdl_json_object *json, struct avdl_node *node) {

	// check main object
	if (avdl_json_getToken(json) != AVDL_JSON_OBJECT_START) {
		avdl_log("Json component should start with a '{': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
		return -1;
	}

	struct avdl_component *c = 0;

	/*
	char component_name[100];
	strcpy(component_name, "c_");
	snprintf(component_name +2, 80, "%d", component_counter);
	*/
	avdl_json_next(json);
	while (avdl_json_getToken(json) != AVDL_JSON_OBJECT_END) {

		// find key
		if (avdl_json_getToken(json) != AVDL_JSON_KEY) {
			avdl_log("json expected key, got something else: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
			return -1;
		}
		//avdl_log("got component key: %s", avdl_json_getTokenString(json));

		/*
		char *content = "\t# found component \"";
		write(fd, content, strlen(content));
		content = avdl_json_getTokenString(json);
		write(fd, content, strlen(content));
		content = "\"\n";
		write(fd, content, strlen(content));
		*/

		if (strcmp(avdl_json_getTokenString(json), "name") == 0) {
			avdl_json_next(json);
			if (avdl_json_getToken(json) == AVDL_JSON_STRING) {
				if (strlen(avdl_json_getTokenString(json)) >= 99) {
					avdl_log("component type too big: %s", avdl_json_getTokenString(json));
					return -1;
				}

				if (strcmp(avdl_json_getTokenString(json), "avdl_component_mesh") == 0) {
					//avdl_log("found mesh component");
					c = avdl_node_AddComponent(node, avdl_component_mesh);
				}
				else {
					struct avdl_component_custom *custom = avdl_node_AddComponent(node, avdl_component_custom);
					avdl_component_custom_SetName(custom, avdl_json_getTokenString(json));
					c = custom;
				}
			}
			else {
				avdl_logError("component name can only be string");
				return -1;
			}
		}
		else {
			if (!c) {
				avdl_logError("component name should come first: %s", avdl_json_getTokenString(json));
				return -1;
			}
			if (strlen(avdl_json_getTokenString(json)) >= 99) {
				avdl_logError("component variable too big: %s", avdl_json_getTokenString(json));
				return -1;
			}
			//avdl_log("component variable name: %s", avdl_json_getTokenString(json));
			struct avdl_component_custom *custom = c;
			avdl_component_custom_AddVariableName(custom, avdl_json_getTokenString(json));

			avdl_json_next(json);

			if (avdl_json_getToken(json) == AVDL_JSON_STRING) {
				//avdl_log("component variable value (string): %s", avdl_json_getTokenString(json));
				avdl_component_custom_AddVariableValue(custom, avdl_json_getTokenString(json), "string");
			}
			else
			if (avdl_json_getToken(json) == AVDL_JSON_INT) {
				//avdl_log("component variable value (int): %s", avdl_json_getTokenString(json));
				avdl_component_custom_AddVariableValue(custom, avdl_json_getTokenString(json), "int");
			}
			else
			if (avdl_json_getToken(json) == AVDL_JSON_FLOAT) {
				//avdl_log("component variable value (float): %s", avdl_json_getTokenString(json));
				avdl_component_custom_AddVariableValue(custom, avdl_json_getTokenString(json), "float");
			}
			else {
				avdl_log("unsupported component variable value type: %s", avdl_json_getTokenString(json));
				return -1;
			}

		}

		avdl_json_next(json);
	}

	avdl_json_next(json);

	if (c) {
		c->after_create(c);
	}

	return 0;
}

static int json_expect_array3f(struct avdl_json_object *json, struct dd_vec3 *v) {

	if (avdl_json_getToken(json) != AVDL_JSON_ARRAY_START) {
		avdl_log("Json: expected array start for 3f: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
		return -1;
	}

	avdl_json_next(json);
	for (int i = 0; i < 3; i++) {
		if (avdl_json_getToken(json) != AVDL_JSON_FLOAT) {
			avdl_log("Json 3f: was expecting 3 floats but found something else: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
			return -1;
		}

		if (i == 0) {
			v->x = avdl_json_getTokenFloat(json);
		}
		else
		if (i == 1) {
			v->y = avdl_json_getTokenFloat(json);
		}
		else
		if (i == 2) {
			v->z = avdl_json_getTokenFloat(json);
		}
		avdl_json_next(json);
	}

	return 0;
}

static int json_expect_node(struct avdl_json_object *json, struct avdl_node *node) {

	// check main object
	if (avdl_json_getToken(json) != AVDL_JSON_OBJECT_START) {
		avdl_log("Json node should start with a '{': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
		return -1;
	}

	struct avdl_transform *t = node->GetLocalTransform(node);

	avdl_json_next(json);
	while (avdl_json_getToken(json) != AVDL_JSON_OBJECT_END) {

		// find key
		if (avdl_json_getToken(json) != AVDL_JSON_KEY) {
			avdl_log("json expected key, got something else: %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
			return -1;
		}

		if (strcmp(avdl_json_getTokenString(json), "name") == 0) {
			avdl_json_next(json);
			if (avdl_json_getToken(json) == AVDL_JSON_STRING) {
				//avdl_log("got string name: %s", avdl_json_getTokenString(json));
				node->SetName(node, avdl_json_getTokenString(json));
			}
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "position") == 0) {
			avdl_json_next(json);
			struct dd_vec3 v;
			json_expect_array3f(json, &v);
			//avdl_log("got position: %f %f %f", v.x, v.y, v.z);
			t->SetPosition(t, &v);
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "rotation") == 0) {
			avdl_json_next(json);
			struct dd_vec3 v;
			json_expect_array3f(json, &v);
			//avdl_log("got rotation: %f %f %f", v.x, v.y, v.z);
			t->SetRotation(t, &v);
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "scale") == 0) {
			avdl_json_next(json);
			struct dd_vec3 v;
			json_expect_array3f(json, &v);
			//avdl_log("got scale: %f %f %f", v.x, v.y, v.z);
			t->SetScale(t, &v);
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "components") == 0) {
			avdl_json_next(json);

			// expect array
			if (avdl_json_getToken(json) != AVDL_JSON_ARRAY_START) {
				avdl_log("Json expected array start '[': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
				return -1;
			}

			avdl_json_next(json);
			while (avdl_json_getToken(json) != AVDL_JSON_ARRAY_END) {
				json_expect_component(json, node);
			}
		}
		else
		if (strcmp(avdl_json_getTokenString(json), "children") == 0) {
			avdl_json_next(json);

			// expect array
			if (avdl_json_getToken(json) != AVDL_JSON_ARRAY_START) {
				avdl_log("Json expected array start '[': %d %s", avdl_json_getToken(json), avdl_json_getTokenString(json));
				return -1;
			}

			avdl_json_next(json);
			while (avdl_json_getToken(json) != AVDL_JSON_ARRAY_END) {
				struct avdl_node *child = node->AddChild(node);

				/*
				char *content = "\n";
				write(fd, content, strlen(content));
				*/
				if (json_expect_node(json, child) != 0) {
					avdl_log("json: error parsing node");
					return -1;
				}
			}
		}
		else {
			avdl_json_next(json);
			avdl_log("something else: %s", avdl_json_getTokenString(json));
			break;
		}

		avdl_json_next(json);
	}
	avdl_json_next(json);

	return 0;
}

int avdl_node_JsonToNode(char *filename, struct avdl_node *o) {

	avdl_log("opening %s to convert to hierarchy", filename);

	struct avdl_json_object json;
	if (avdl_json_initFile(&json, filename) != 0) {
		avdl_log("could not open json file %s to convert to node", filename);
		return -1;
	}
	avdl_json_next(&json);

	if (json_expect_node(&json, o) != 0) {
		avdl_log("error translating json to dd");
		avdl_json_deinit(&json);
		return -1;
	}

	avdl_json_deinit(&json);

	return 0;
}
