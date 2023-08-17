cmake_minimum_required(VERSION 3.19)

# set the project name
project(avdl VERSION 0.16.6)

# add the executable
add_executable(${PROJECT_NAME}
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
	src/avdl_time.c
)

add_compile_definitions(
	PKG_VERSION=\"${CMAKE_PROJECT_VERSION}\"
	AVDL_DYNAMIC_PKG_LOCATION
)

# add include directory, for binary `avdl`
target_include_directories(${PROJECT_NAME} PUBLIC include)

# main executable
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# cengine
include("engines/cengine/cengine.cmake")

# android engine
install(DIRECTORY engines/android DESTINATION share/avdl/)

# templates
install(DIRECTORY templates DESTINATION share/avdl/)

# packaging using `cpack`
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VENDOR "Afloofdev")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Abstract video-game development language compiler")
set(CPACK_PACKAGE_DESCRIPTION "A high level video game programming language. Games made with it are meant to be described in an abstract way and then be compiled for a variety of platforms.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://afloofdev.com/avdl/")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_NSIS_MODIFY_PATH ON)
include(CPack)
