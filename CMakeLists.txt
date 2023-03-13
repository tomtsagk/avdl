cmake_minimum_required(VERSION 3.19)

# set the project name
project(avdl VERSION 0.7.8)

# add the executable
add_executable(avdl
	src/avdl_ast_node.c
	src/avdl_dynamic_array.c
	src/avdl_lexer.c
	src/avdl_parser_cglut.c
	src/avdl_semantic_analyser.c
	src/avdl_symtable.c
	src/avdl_commands.c
	src/avdl_file_op.c
	src/avdl_main.c
	src/avdl_struct_table.c
	src/avdl_variable_type.c
	src/avdl_std.c
	src/whereami.c
	src/avdl_pkg.c
	src/avdl_settings.c
	src/avdl_log.c
	src/avdl_string.c
	src/avdl_arguments.c
)

add_compile_definitions(
	PKG_VERSION=\"${CMAKE_PROJECT_VERSION}\"
	AVDL_DYNAMIC_PKG_LOCATION
)

# add include directory, for binary `avdl`
target_include_directories(avdl PUBLIC include)

# main executable
install(TARGETS avdl DESTINATION bin)

# cengine
install(FILES
	engines/cengine/src/avdl_achievements.c
	engines/cengine/src/avdl_achievements_steam.cpp
	engines/cengine/src/avdl_assetManager.c
	engines/cengine/src/avdl_data.c
	engines/cengine/src/avdl_localisation.c
	engines/cengine/src/avdl_multiplayer.c
	engines/cengine/src/avdl_multiplayer_steam.cpp
	engines/cengine/src/avdl_particle_system.c
	engines/cengine/src/avdl_steam_cpp.cpp
	engines/cengine/src/avdl_particle_system.c
	engines/cengine/src/avdl_shaders.c
	engines/cengine/src/avdl_steam.c
	engines/cengine/src/dd_dynamic_array.c
	engines/cengine/src/dd_filetomesh.c
	engines/cengine/src/dd_fov.c
	engines/cengine/src/dd_game.c
	engines/cengine/src/dd_gamejolt.c
	engines/cengine/src/dd_image.c
	engines/cengine/src/dd_json.c
	engines/cengine/src/dd_log.c
	engines/cengine/src/dd_math.c
	engines/cengine/src/dd_matrix.c
	engines/cengine/src/dd_mesh.c
	engines/cengine/src/dd_meshColour.c
	engines/cengine/src/dd_meshTexture.c
	engines/cengine/src/dd_mouse.c
	engines/cengine/src/dd_music.c
	engines/cengine/src/dd_opengl.c
	engines/cengine/src/dd_sound.c
	engines/cengine/src/dd_string3d.c
	engines/cengine/src/dd_vec3.c
	engines/cengine/src/dd_vec4.c
	engines/cengine/src/dd_world.c
	engines/cengine/src/main.c
	engines/cengine/src/main_cpp.cpp
	engines/cengine/src/avdl_input.c
	engines/cengine/src/whereami.c
	DESTINATION
	share/avdl/cengine
)
install(FILES
	engines/cengine/include/avdl_achievements.h
	engines/cengine/include/avdl_assetManager.h
	engines/cengine/include/avdl_cengine.h
	engines/cengine/include/avdl_data.h
	engines/cengine/include/avdl_localisation.h
	engines/cengine/include/avdl_multiplayer.h
	engines/cengine/include/avdl_particle_system.h
	engines/cengine/include/avdl_shaders.h
	engines/cengine/include/avdl_steam.h
	engines/cengine/include/dd_async_call.h
	engines/cengine/include/dd_dynamic_array.h
	engines/cengine/include/dd_filetomesh.h
	engines/cengine/include/dd_fov.h
	engines/cengine/include/dd_game.h
	engines/cengine/include/dd_gamejolt.h
	engines/cengine/include/dd_image.h
	engines/cengine/include/dd_json.h
	engines/cengine/include/dd_log.h
	engines/cengine/include/dd_math.h
	engines/cengine/include/dd_matrix.h
	engines/cengine/include/dd_meshColour.h
	engines/cengine/include/dd_mesh.h
	engines/cengine/include/dd_meshTexture.h
	engines/cengine/include/dd_mouse.h
	engines/cengine/include/dd_music.h
	engines/cengine/include/dd_opengl.h
	engines/cengine/include/dd_sound.h
	engines/cengine/include/dd_string3d.h
	engines/cengine/include/dd_vec2.h
	engines/cengine/include/dd_vec3.h
	engines/cengine/include/dd_vec4.h
	engines/cengine/include/dd_world.h
	engines/cengine/include/avdl_input.h
	engines/cengine/include/whereami.h
	DESTINATION
	include
)

# android engine
install(DIRECTORY engines/android DESTINATION share/avdl/)

# packaging using `cpack`
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VENDOR "Dark Dimension")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Abstract video-game development language compiler")
set(CPACK_PACKAGE_DESCRIPTION "A high level video game programming language. Games made with it are meant to be described in an abstract way and then be compiled for a variety of platforms.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://darkdimension.org/avdl.html")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "avdl")
set(CPACK_NSIS_MODIFY_PATH ON)
include(CPack)
