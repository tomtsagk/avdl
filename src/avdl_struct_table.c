#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "avdl_struct_table.h"
#include "shared/avdl_log.h"

struct struct_table_entry struct_table[DD_STRUCT_TABLE_TOTAL];
int struct_table_current = 0;

// initialise the struct table index to no tables
void struct_table_init() {
	struct_table_current = -1;

	// Engine's initial structs and their members
	struct_table_push("dd_world", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("onload", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("update", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("resize", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("input", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_matrix", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_mesh", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("draw", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("set_primitive", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("load", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("copy", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("set_colour", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("set_primitive_texcoords", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setTexture", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setTextureNormal", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setTextureIndex", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("hasTexture", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setTransparency", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setWireframe", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setSolid", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetTypeLine", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetBoundsPositive", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetBoundsNegative", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_skinned_mesh", "avdl_mesh");
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("draw", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("update", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("PlayAnimation", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("PlayAnimationInstant", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetOnAnimationDone", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("PrintAnimations", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_mesh", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("draw", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("set_primitive", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("load", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("copy", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_meshColour", "dd_mesh");
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("set_colour", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_meshRising", "dd_meshColour");
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("set_animation_max", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("set_animation_current", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_meshTexture", "dd_meshColour");
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("preloadTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("applyTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("loadTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("set_primitive_texcoords", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("copyTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setTexture", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setTransparency", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_texture", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("set", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_skybox", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("set", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("bind", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("draw", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("unbind", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("dd_sound", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("load", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("play", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("playLoop", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("stop", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_vec3", 0);
	struct_table_push_member("X", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Y", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Z", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Setf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Set", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetX", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetY", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Add", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Addf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Subtract", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Subtractf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Divide", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Dividef", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Multiply", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Multiplyf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Multiply1f", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("MultiplyMatrix", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Cross", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Dot", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Normalise", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Magnitude", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("RotateX", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("RotateY", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("RotateZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Invert", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Print", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_vec4", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Setf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetVec3", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Set", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetX", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetY", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetW", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("X", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Y", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Z", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("W", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Add", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Addf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Subtract", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Subtractf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Multiply", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Multiplyf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Multiply1f", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("MultiplyMatrix", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Divide", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Dividef", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Cross", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Dot", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Distance", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Invert", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Print", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Normalise", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Magnitude", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_ray3", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetPosition", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetPositionVec4", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetPosition3f", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetPosition", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetDirection", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetDirectionVec4", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetDirection3f", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetDirection", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Print", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("CollisionWithAABB", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_plane", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetPosition", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetPositionVec4", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetPosition3f", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetPosition", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetNormal", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetNormalVec4", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetNormal3f", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetNormal", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("dd_string3d", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setAlign", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setAlignVertical", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setText", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setTextInt", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("draw", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("drawInt", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("drawIntPadded", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("drawLimit", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("drawLimitTypewriter", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("drawTypewriter", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setFont", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getWidth", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getWidthInt", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("isOnce", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_program", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("setVertexShader", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setFragmentShader", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("useProgram", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_particle_system", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("assignAsset", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("update", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("draw", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setDelay", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticleLife", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticlePositionXFunc", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticlePositionYFunc", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticlePositionZFunc", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticleScaleFunc", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticlePosition", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticlePositionFuzz", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticleRotation", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticleRotationFuzz", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticleScale", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticleScaleFuzz", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setParticlesTotal", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_localisation", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("set", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getValue", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_physics", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("update", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("draw", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("addObject", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clearObjects", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("addConstantForcef", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clearConstantForce", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_collider_collision", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetOverlap", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetNormal1", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetNormal2", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_collider", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_collider_aabb", "avdl_collider");
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setMin", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setMax", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getMaxX", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getMaxY", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getMaxZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_collider_sphere", "avdl_collider");
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setRadius", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_collider_terrain", "avdl_collider");
	struct_table_push_member("SetTerrain", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_rigidbody", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setPositionf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setMass", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setRestitution", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("addVelocityf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setVelocityf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("addAngularVelocityf", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("matrixMultiply", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setCollider", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getPositionX", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getPositionY", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getPositionZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("reset", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("hasJustCollided", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_font", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("set", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("addCustomIcon", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_dynamic_array", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_ui_element", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetSize", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetPosition", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetPositionZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetAnchor", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("applyTransform", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("update", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("drawDebug", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("resize", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("mouse_input", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("hasMouseCollided", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("disable", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetOnClick", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("IsSelected", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("IsClicked", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("IsVisible", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetVisible", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetPositionX", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetPositionY", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetPositionZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_terrain", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("load", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("draw", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getSpot", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getWidth", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getHeight", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("isLoaded", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("isOnTerrain", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setScaleZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setTextureIndex", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("setMesh", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_transform", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetPosition", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetPosition3f", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetRotation", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetRotation3f", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetScale", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetScale3f", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetMatrix", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetInverseMatrix", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetNormalMatrix", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetNormalInverseMatrix", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetPosition", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetPositionX", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetPositionY", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetPositionZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetRotation", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetRotationX", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetRotationY", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetRotationZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetScale", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetScaleX", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetScaleY", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetScaleZ", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_node", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetGlobalMatrix", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetGlobalInverseMatrix", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetGlobalNormalMatrix", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetGlobalNormalInverseMatrix", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetLocalTransform", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("AddChild", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("RemoveChild", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetName", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetName", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetChildrenCount", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetChild", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetParent", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Copy", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("Duplicate", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetComponent", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_component", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("after_create", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("Copy", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetNode", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetType", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetType", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_component_mesh", "avdl_component");
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("after_create", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("mesh_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0); // variable
	struct_table_push_member("texture_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0); // variable
	struct_table_push_member("hasTransparency", DD_VARIABLE_TYPE_FUNCTION, 0, 0); // variable
	struct_table_push_member("draw", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("GetBoundsCenter", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetBoundsExtend", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push("avdl_component_skinned_mesh", "avdl_component");
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("after_create", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("mesh_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0); // variable
	struct_table_push_member("texture_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0); // variable
	struct_table_push_member("hasTransparency", DD_VARIABLE_TYPE_FUNCTION, 0, 0); // variable
	struct_table_push_member("PlayAnimation", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("PlayAnimationInstant", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetOnAnimationDone", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("update", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("draw", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("PrintAnimations", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_component_terrain", "avdl_component");
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("after_create", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("asset_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0); // variable
	struct_table_push_member("draw", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetTerrain", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("IsOnTerrain", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetSpot", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("GetCollider", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("scaleZ", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("texture_main_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("texture0_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("texture1_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("texture2_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("texture3_name", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
	struct_table_push("avdl_time", 0);
	struct_table_push("avdl_string", 0);
	struct_table_push_member("create", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("SetMaxCharacters", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("toCharPtr", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("isValid", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("getError", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("cat", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("copy", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("empty", AVDL_VARIABLE_TYPE_FUNCTION_INLINE, 0, 0);
	struct_table_push_member("clean", DD_VARIABLE_TYPE_FUNCTION, 0, 0);
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
		printf("struct: %s (%d)", s->name, i);
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

		// for inline functions return the first struct found
		int memberIndex = struct_table_get_member(structIndex, membername);
		if (struct_table_get_member_type(structIndex, memberIndex) == AVDL_VARIABLE_TYPE_FUNCTION_INLINE) {
			return parent_level_current;
		}
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

int struct_table_HasMemberInAnyParent(int structIndex, const char *member) {
	int parentIndex = struct_table[structIndex].parent;
	while (parentIndex >= 0) {
		if (struct_table_has_member(parentIndex, member)) {
			return 1;
		}
		parentIndex = struct_table[parentIndex].parent;
	}

	return 0;
}

int struct_table_GetNearestParentWithMember(int structIndex, const char *member) {
	int parentIndex = struct_table[structIndex].parent;
	while (parentIndex >= 0) {
		if (struct_table_has_member(parentIndex, member)) {
			return parentIndex;
		}
		parentIndex = struct_table[parentIndex].parent;
	}

	return 0;
}
