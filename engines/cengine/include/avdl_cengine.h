#include "dd_opengl.h"
#include "dd_dynamic_array.h"
#include "dd_filetomesh.h"
#include "dd_matrix.h"
#include "dd_mesh.h"
#include "dd_meshColour.h"
#include "dd_meshTexture.h"
#include "dd_sound.h"
#include "dd_world.h"
#include "dd_math.h"
#include "dd_fov.h"
#include "dd_game.h"
#include "dd_vec3.h"
#include "dd_vec4.h"
#include "dd_mouse.h"
#include "dd_string3d.h"
#include "dd_meshRising.h"
#include "avdl_shaders.h"
#include "dd_log.h"

#if DD_PLATFORM_NATIVE
/* files temporarily not available in android */
#include "dd_object.h"
//#include "dd_gamejolt.h"
#endif

int dd_main(int argc, char *argv[]);
