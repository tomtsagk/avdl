#ifndef AVDL_GAME_VERSION
#define AVDL_GAME_VERSION "0.0.0"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "dd_dynamic_array.h"
#include "dd_filetomesh.h"
#include "dd_matrix.h"
#include "dd_mesh.h"
#include "dd_meshColour.h"
#include "dd_meshTexture.h"
#include "dd_sound.h"
#include "dd_music.h"
#include "dd_world.h"
#include "dd_math.h"
#include "dd_fov.h"
#include "dd_game.h"
#include "dd_vec3.h"
#include "dd_vec4.h"
#include "dd_mouse.h"
#include "dd_string3d.h"
#include "dd_image.h"
#include "avdl_shaders.h"
#include "dd_log.h"
#include "avdl_data.h"
#include "avdl_assetManager.h"
#include "avdl_particle_system.h"
#include "avdl_localisation.h"
#include "avdl_steam.h"
#include "avdl_achievements.h"
#include "avdl_multiplayer.h"
#include "avdl_input.h"
#include "avdl_physics.h"
#include "avdl_collider.h"
#include "avdl_collider_aabb.h"
#include "avdl_collider_sphere.h"
#include "avdl_rigidbody.h"
#include "avdl_graphics.h"
#include "avdl_engine.h"

int dd_main(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif
