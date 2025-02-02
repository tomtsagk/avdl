#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "avdl_struct_table.h"
#include "avdl_log.h"

struct struct_table_entry struct_table[DD_STRUCT_TABLE_TOTAL];
int struct_table_current = 0;

// initialise the struct table index to no tables
void struct_table_init() {
	struct_table_current = -1;

	// Engine's initial structs and their members
	struct_table_push("dd_world", 0);
	struct_table_push_member("create", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("onload", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("update", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("resize", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("input", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_matrix", 0);
	struct_table_push("avdl_mesh", 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("set_primitive", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("load", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("copy", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("set_colour", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("set_primitive_texcoords", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setTextureNormal", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setTextureIndex", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("hasTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setTransparency", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setWireframe", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setSolid", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_skinned_mesh", "avdl_mesh");
	struct_table_push_member("update", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("PlayAnimation", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("PlayAnimationInstant", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetOnAnimationDone", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_mesh", 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("set_primitive", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("load", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("copy", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_meshColour", "dd_mesh");
	struct_table_push_member("set_colour", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_meshRising", "dd_meshColour");
	struct_table_push_member("set_animation_max", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("set_animation_current", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_meshTexture", "dd_meshColour");
	struct_table_push_member("preloadTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("applyTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("loadTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("set_primitive_texcoords", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("copyTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setTransparency", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_image", 0);
	struct_table_push_member("set", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("bind", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("bindIndex", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("unbind", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_skybox", 0);
	struct_table_push_member("set", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("bind", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("unbind", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_sound", 0);
	struct_table_push_member("load", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("play", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("playLoop", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("stop", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_vec3", 0);
	struct_table_push_member("setf", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getX", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getY", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getZ", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_vec4", 0);
	struct_table_push_member("set", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getX", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getY", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getZ", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getW", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("print", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_string3d", 0);
	struct_table_push_member("setAlign", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setAlignVertical", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setText", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setTextUnicode", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setTextInt", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("drawInt", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("drawIntPadded", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("drawLimit", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("drawLimitTypewriter", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("drawTypewriter", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setFont", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getWidth", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getWidthInt", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("isOnce", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_program", 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setVertexShader", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setFragmentShader", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("useProgram", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_particle_system", 0);
	struct_table_push_member("assignAsset", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("update", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setDelay", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticleLife", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticlePositionXFunc", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticlePositionYFunc", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticlePositionZFunc", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticleScaleFunc", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticlePosition", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticlePositionFuzz", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticleRotation", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticleRotationFuzz", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticleScale", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticleScaleFuzz", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setParticlesTotal", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_localisation", 0);
	struct_table_push_member("set", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getValue", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getValueUnicode", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_multiplayer_identity", 0);
	struct_table_push("avdl_physics", 0);
	struct_table_push_member("update", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("addObject", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clearObjects", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("addConstantForcef", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clearConstantForce", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_collider_collision", 0);
	struct_table_push_member("overlap", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_collider", 0);
	struct_table_push("avdl_collider_aabb", "avdl_collider");
	struct_table_push_member("setMin", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setMax", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getMaxX", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getMaxY", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getMaxZ", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_collider_sphere", "avdl_collider");
	struct_table_push_member("setRadius", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_rigidbody", 0);
	struct_table_push_member("setPositionf", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setMass", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setRestitution", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("addVelocityf", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setVelocityf", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("addAngularVelocityf", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("matrixMultiply", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setCollider", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getPositionX", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getPositionY", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getPositionZ", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("reset", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("hasJustCollided", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_font", 0);
	struct_table_push_member("set", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("addCustomIcon", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_dynamic_array", 0);
	struct_table_push("avdl_ui_element", 0);
	struct_table_push_member("SetSize", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetPosition", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetPositionZ", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetAnchor", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("applyTransform", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("update", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("drawDebug", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("resize", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("mouse_input", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("hasMouseCollided", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("disable", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetOnClick", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("IsSelected", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("IsClicked", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("IsVisible", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetVisible", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetPositionX", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetPositionY", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetPositionZ", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_terrain", 0);
	struct_table_push_member("load", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getSpot", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getWidth", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("getHeight", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("isLoaded", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("isOnTerrain", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setScaleZ", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setMesh", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_transform", 0);
	struct_table_push_member("SetPosition", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetPosition3f", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetRotation", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetRotation3f", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetScale", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetScale3f", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetMatrix", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetInverseMatrix", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetPosition", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetRotation", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetScale", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_node", 0);
	struct_table_push_member("GetGlobalMatrix", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetGlobalInverseMatrix", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetLocalTransform", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("AddChild", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetName", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetChildrenCount", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetChild", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_component", 0);
	struct_table_push_member("after_create", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetNode", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("SetType", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetType", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_component_mesh", "avdl_component");
	struct_table_push_member("mesh_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0); // variable
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_component_terrain", "avdl_component");
	struct_table_push_member("asset_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0); // variable
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetTerrain", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("IsOnTerrain", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetSpot", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	//struct_table_push("dd_gamejolt_response_struct", 0);
}

// push a new struct to the table
int struct_table_push(const char *structname, const char *parentname) {

	// check for max number of structs
	if (struct_table_current+1 > DD_STRUCT_TABLE_TOTAL) {
		printf("struct_table_add: max number of struct tables reached\n");
		exit(-1);
	}

	// increment counter
	struct_table_current++;

	// set the values of the new struct
	struct struct_table_entry *newStruct = &struct_table[struct_table_current];
	strncpy(newStruct->name, structname, DD_STRUCT_TABLE_NAME_SIZE -1);
	newStruct->name[DD_STRUCT_TABLE_NAME_SIZE-1] = '\0';
	newStruct->member_total = 0;
	newStruct->parent = -1;
	newStruct->isStruct = 0;

	// if a parent name was given, check if it exists, and assign as parent of the new struct
	if (parentname) {
		for (int i = 0; i < struct_table_current; i++) {
			if (strcmp(struct_table[i].name, parentname) == 0) {
				newStruct->parent = i;
				break;
			}
		}

		// no parent was found, this is an error!
		if (newStruct->parent < 0) {
			printf("error: struct_table_push: no parent found with name '%s'\n", parentname);
			exit(-1);
		}
	}

	// return the index of the new struct
	return struct_table_current;
}

/* push a new member to the last pushed struct
 * get a reference to the last pushed struct
 * check if there is space in the struct
 * add a new member to it
 */
struct struct_table_entry_member *struct_table_push_member(const char *name, enum dd_variable_type type, const char *nametype, int isRef) {
	struct struct_table_entry *currentStruct = &struct_table[struct_table_current];
	if (currentStruct->member_total+1 >= DD_STRUCT_TABLE_MEMBER_TOTAL) {
		printf("struct_table_push_member: struct '%s': maximum number of members reached\n", currentStruct->name);
		exit(-1);
	}
	struct struct_table_entry_member *newMember = &currentStruct->members[currentStruct->member_total];
	newMember->type = type;
	strncpy(newMember->name, name, DD_STRUCT_TABLE_NAME_SIZE -1);
	newMember->name[DD_STRUCT_TABLE_NAME_SIZE-1] = '\0';
	newMember->arrayCount = 1;
	newMember->isRef = isRef;
	if (nametype) {
		strncpy(newMember->nametype, nametype, DD_STRUCT_TABLE_NAME_SIZE -1);
		newMember->nametype[DD_STRUCT_TABLE_NAME_SIZE-1] = '\0';
	}
	else {
		newMember->nametype[0] = '\0';
	}
	currentStruct->member_total++;

	return &currentStruct->members[currentStruct->member_total -1];
}

void struct_table_push_member_array(const char *name, enum dd_variable_type type, const char *nametype, int arrayCount, int isRef) {
	struct struct_table_entry_member *newMember = struct_table_push_member(name, type, nametype, isRef);
	newMember->arrayCount = arrayCount;
}

void struct_table_pop() {
	struct struct_table_entry *currentTable = &struct_table[struct_table_current];
	if (currentTable->member_total > 0) {
		currentTable->member_total--;
	}
}

/* print all structs and their members
 * this is meant for debug only
 */
void struct_table_print() {
	printf("struct table start\n");
	for (int i = 0; i <= struct_table_current; i++) {
		struct struct_table_entry *s = &struct_table[i];
		printf("struct: %s", s->name);
		if (s->parent >= 0) {
			printf(":%s", struct_table_get_name(s->parent));
		}
		printf("\n");

		for (int j = 0; j < s->member_total; j++) {
			struct struct_table_entry_member *m = &s->members[j];
			printf("	member: %s\n", m->name);
		}
	}
	printf("struct table end\n");
}

// return the name of the struct on index, make sure index is in bounds
const char *struct_table_get_name(int index) {
	if (index < 0 || index > struct_table_current) {
		avdl_log_error("struct_table_get_name: index out of bounds: %d\n", index);
		return "<null>";
	}
	return struct_table[index].name;
}

const char *struct_table_get_member_name(int structIndex, int memberIndex) {
	return struct_table[structIndex].members[memberIndex].name;
}

int struct_table_getMemberArrayCount(int structIndex, int memberIndex) {
	return struct_table[structIndex].members[memberIndex].arrayCount;
}

int struct_table_getMemberIsRef(int structIndex, int memberIndex) {
	return struct_table[structIndex].members[memberIndex].isRef;
}

enum dd_variable_type struct_table_get_member_type(int structIndex, int memberIndex) {
	return struct_table[structIndex].members[memberIndex].type;
}

int struct_table_get_member(int structIndex, const char *membername) {
	for (int i = 0; i < struct_table[structIndex].member_total; i++) {
		struct struct_table_entry_member *m = &struct_table[structIndex].members[i];
		if (strcmp(m->name, membername) == 0) {
			return i;
		}
	}
	return -1;
}

int struct_table_is_member_primitive(int structIndex, int memberIndex) {
	struct struct_table_entry_member *m = &struct_table[structIndex].members[memberIndex];
	return m->type != DD_VARIABLE_TYPE_STRUCT;
}

int struct_table_is_member_primitive_string(int structIndex, const char *membername) {
	if (struct_table_has_member(structIndex, membername)) {
		int memberId = struct_table_get_member(structIndex, membername);
		struct struct_table_entry_member *m = &struct_table[structIndex].members[memberId];
		return m->type != DD_VARIABLE_TYPE_STRUCT;
	}
	else
	if (struct_table[structIndex].parent >= 0) {
		return struct_table_is_member_primitive_string(struct_table[structIndex].parent, membername);
	}
	return 0;
}

int struct_table_has_member(int structIndex, const char *membername) {
	if (structIndex < 0 || structIndex > struct_table_current) {
		printf("error: struct_table_has_member: index out of bounds: %d for member '%s'\n", structIndex, membername);
		exit(-1);
	}

	struct struct_table_entry *e = &struct_table[structIndex];
	for (int i = 0; i < e->member_total; i++) {
		struct struct_table_entry_member *m = &e->members[i];
		if (strcmp(m->name, membername) == 0) {
			return 1;
		}
	}

	return 0;
}

int struct_table_has_member_parent(int structIndex, const char *membername) {
	if (structIndex < 0 || structIndex > struct_table_current) {
		printf("error: struct_table_has_member: index out of bounds: %d for member '%s'\n", structIndex, membername);
		exit(-1);
	}

	// has member
	if (struct_table_has_member(structIndex, membername)) {
		return 1;
	}

	// maybe parent has member ?
	if (struct_table[structIndex].parent >= 0) {
		return struct_table_has_member_parent(struct_table[structIndex].parent, membername);
	}

	// doesn't have member
	return 0;
}

static int parent_level;
static int parent_level_current;
static int struct_table_is_member_parent_search(int structIndex, const char *membername) {
	if (structIndex < 0 || structIndex > struct_table_current) {
		avdl_log_error("struct_table_is_member_parent: index out of bounds: %d %s", structIndex, membername);
		return -1;
	}

	parent_level++;
	struct struct_table_entry *t = &struct_table[structIndex];

	// is member of this struct
	if (struct_table_has_member(structIndex, membername)) {
		parent_level_current = parent_level;
	}

	if (t->parent == -1) {
		return parent_level_current;
	}

	return struct_table_is_member_parent_search(t->parent, membername);
}

int struct_table_is_member_parent(int structIndex, const char *membername) {
	parent_level_current = -1;
	parent_level = -1;
	return struct_table_is_member_parent_search(structIndex, membername);
}

int struct_table_get_index(const char *structname) {

	if (!structname) {
		return -1;
	}

	for (int i = 0; i <= struct_table_current; i++) {
		if (strcmp(struct_table[i].name, structname) == 0) {
			return i;
		}
	}

	return -1;
}

int struct_table_get_member_scope(int structIndex, int memberIndex) {
	if (structIndex < 0 || structIndex > struct_table_current) {
		printf("error: struct_table_get_member_scope: index out of bounds: %d\n", structIndex);
		exit(-1);
	}

	struct struct_table_entry_member *m = &struct_table[structIndex].members[memberIndex];
	if (m->type == DD_VARIABLE_TYPE_STRUCT) {
		return struct_table_get_index(m->nametype);
	}
	printf("struct_table_get_member_scope: error: '%s' of '%s' is not struct\n", m->name, struct_table_get_name(structIndex));
	exit(-1);
}

int struct_table_get_member_scope_string(int structIndex, const char *membername) {
	if (structIndex < 0 || structIndex > struct_table_current) {
		printf("error: struct_table_get_member_scope_string: index out of bounds: %d\n", structIndex);
		exit(-1);
	}

	if (struct_table_has_member(structIndex, membername)) {
		int memberIndex = struct_table_get_member(structIndex, membername);
		struct struct_table_entry_member *m = &struct_table[structIndex].members[memberIndex];
		if (m->type == DD_VARIABLE_TYPE_STRUCT) {
			return struct_table_get_index(m->nametype);
		}
		printf("struct_table_get_member_scope: error: '%s' of '%s' is not struct\n", m->name, struct_table_get_name(structIndex));
		exit(-1);
	}
	else
	if (struct_table[structIndex].parent) {
		return struct_table_get_member_scope_string(struct_table[structIndex].parent, membername);
	}
	printf("failed searching for %d %s\n", structIndex, membername);
	struct_table_print();

	printf("struct_table_get_member_scope: error: \n");
	exit(-1);
}

int struct_table_get_parent(int structIndex) {
	return struct_table[structIndex].parent;
}

unsigned int struct_table_get_member_total(int structIndex) {
	return struct_table[structIndex].member_total;
}

char *struct_table_get_member_nametype(int structIndex, int memberIndex) {
	return struct_table[structIndex].members[memberIndex].nametype;
}

int struct_table_count() {
	return struct_table_current+1;
}

int struct_table_exists(const char *structname) {

	if (!structname) {
		printf("struct_table_exists: no struct given\n");
		exit(-1);
	}

	for (int i = 0; i <= struct_table_current; i++) {
		if (strcmp(struct_table[i].name, structname) == 0) {
			return 1;
		}
	}

	return 0;
}

void struct_table_SetStruct(int structIndex) {
	struct_table[structIndex].isStruct = 1;
}

int struct_table_IsStruct(int structIndex) {
	return struct_table[structIndex].isStruct;
}
