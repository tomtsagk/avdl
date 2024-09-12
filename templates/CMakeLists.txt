cmake_minimum_required(VERSION 3.19)

include("avdl_project.cmake")

# set the project name
project("${AVDL_PROJECT_NAME_CODE}" VERSION ${AVDL_VERSION_NAME})

# compile for windows, with dynamic location
add_compile_definitions(
	AVDL_DYNAMIC_PKG_LOCATION
	AVDL_WINDOWS
)

add_definitions(
	"-DAVDL_GAME_VERSION=\"${AVDL_VERSION_NAME}\""
)

if (${AVDL_STEAM})
	add_compile_definitions(
		AVDL_STEAM
	)
endif()

# add the executable
add_executable("${CMAKE_PROJECT_NAME}" WIN32
	${AVDL_SRC}
)

if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
	set(AVDL_ARCH "x64")
else()
	set(AVDL_ARCH "x86")
endif()
message("avdl: arch: ${AVDL_ARCH}")

if (NOT DEFINED AVDL_DIRECTORY)
	message(FATAL_ERROR "No AVDL_DIRECTORY given.")
endif()

set(AVDL_DEPENDENCIES_DIRECTORY "${AVDL_DIRECTORY}/dependencies/windows/")

# sdl2
set(SDL2_INCLUDE_DIRS "${AVDL_DEPENDENCIES_DIRECTORY}/include")
set(SDL2_LIBRARIES "${AVDL_DEPENDENCIES_DIRECTORY}/lib/${AVDL_ARCH}/SDL2.lib;${AVDL_DEPENDENCIES_DIRECTORY}/lib/${AVDL_ARCH}/SDL2main.lib")
set(SDL2_DLL "${AVDL_DEPENDENCIES_DIRECTORY}/lib/${AVDL_ARCH}/SDL2.dll")
message("avdl: SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
message("avdl: SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
message("avdl: SDL2_DLL: ${SDL2_DLL}")
foreach(i ${SDL2_DLL})
	install(FILES "${i}" DESTINATION bin/)
	message("avdl: SDL2_DLL iterator: ${i}")
endforeach()

# sdl2_mixer
set(SDL2_MIXER_INCLUDE_DIRS "${AVDL_DEPENDENCIES_DIRECTORY}/include")
set(SDL2_MIXER_LIBRARIES "${AVDL_DEPENDENCIES_DIRECTORY}/lib/${AVDL_ARCH}/SDL2_mixer.lib")
set(SDL2_MIXER_DLL
	"${AVDL_DEPENDENCIES_DIRECTORY}/lib/x64/SDL2_mixer.dll"
	"${AVDL_DEPENDENCIES_DIRECTORY}/lib/x64/optional/libogg-0.dll"
)
message("avdl: SDL2_MIXER_INCLUDE_DIRS: ${SDL2_MIXER_INCLUDE_DIRS}")
message("avdl: SDL2_MIXER_LIBRARIES: ${SDL2_MIXER_LIBRARIES}")
message("avdl: SDL2_MIXER_DLL: ${SDL2_MIXER_DLL}")
foreach(i ${SDL2_MIXER_DLL})
	install(FILES "${i}" DESTINATION bin/)
	message("avdl: SDL2_MIXER_DLL iterator: ${i}")
endforeach()

# glew
set(GLEW_INCLUDE_DIRS "${AVDL_DEPENDENCIES_DIRECTORY}/include")
set(GLEW_LIBRARIES "${AVDL_DEPENDENCIES_DIRECTORY}/lib/Release/${AVDL_ARCH}/glew32.lib;${AVDL_DEPENDENCIES_DIRECTORY}/lib/Release/${AVDL_ARCH}/glew32s.lib")
set(GLEW_DLL "${AVDL_DEPENDENCIES_DIRECTORY}/bin/Release/${AVDL_ARCH}/glew32.dll")
message("avdl: GLEW_INCLUDE_DIRS: ${GLEW_INCLUDE_DIRS}")
message("avdl: GLEW_LIBRARIES: ${GLEW_LIBRARIES}")
message("avdl: GLEW_DLL: ${GLEW_DLL}")
foreach(i ${GLEW_DLL})
	install(FILES "${i}" DESTINATION bin/)
	message("avdl: GLEW_DLL iterator: ${i}")
endforeach()

# zlib
set(ZLIB_INCLUDE_DIR "${AVDL_DEPENDENCIES_DIRECTORY}/zlib/include")
set(ZLIB_DLL "${AVDL_DEPENDENCIES_DIRECTORY}/zlib/bin/zlib.dll")
install(FILES ZLIB_DLL DESTINATION bin/)
message("avdl: ZLIB_INCLUDE_DIR: ${ZLIB_INCLUDE_DIR}")
message("avdl: ZLIB_DLL: ${ZLIB_DLL}")

# freetype
set(FREETYPE_INCLUDE_DIR "${AVDL_DEPENDENCIES_DIRECTORY}/freetype/include")
set(FREETYPE_LIBRARY "${AVDL_DEPENDENCIES_DIRECTORY}/freetype/lib/freetype.lib")
set(FREETYPE_DLL "${AVDL_DEPENDENCIES_DIRECTORY}/freetype/bin/freetype.dll")
install(FILES FREETYPE_DLL DESTINATION bin/)
message("avdl: FREETYPE_INCLUDE_DIR: ${FREETYPE_INCLUDE_DIR}")
message("avdl: FREETYPE_LIBRARY: ${FREETYPE_LIBRARY}")
message("avdl: FREETYPE_DLL: ${FREETYPE_DLL}")

# png
set(PNG_LIBRARY "${AVDL_DEPENDENCIES_DIRECTORY}/libpng/lib/libpng16.lib")
set(PNG_INCLUDE_DIR "${AVDL_DEPENDENCIES_DIRECTORY}/libpng/include")
set(PNG_DLL "${AVDL_DEPENDENCIES_DIRECTORY}/libpng/bin/libpng16.dll")
install(FILES PNG_DLL DESTINATION bin/)
message("avdl: PNG_LIBRARY: ${PNG_LIBRARY}")
message("avdl: PNG_INCLUDE_DIR: ${PNG_INCLUDE_DIR}")
message("avdl: PNG_DLL: ${PNG_DLL}")
find_package(PNG REQUIRED)

# opengl
if (DEFINED AVDL_DIRECT3D11)
else()
	find_package(OpenGL     REQUIRED)
endif()

# steam
if (${AVDL_STEAM})
	set(STEAM_INCLUDE_DIR "${AVDL_DEPENDENCIES_DIRECTORY}/include")
	if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
		set(STEAM_LIBRARY "${AVDL_DEPENDENCIES_DIRECTORY}/lib/steam_api64.lib")
		install(FILES "${AVDL_DEPENDENCIES_DIRECTORY}/lib/steam_api64.dll" DESTINATION bin/)
	else()
		set(STEAM_LIBRARY "${AVDL_DEPENDENCIES_DIRECTORY}/lib/steam_api.lib")
		install(FILES "${AVDL_DEPENDENCIES_DIRECTORY}/lib/steam_api.dll" DESTINATION bin/)
	endif()
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${PNG_LIBRARY}     )
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${FREETYPE_LIBRARY})
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${STEAM_LIBRARY}   )

# add include directory, for binary, including dependencies
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
	cengine
	include
	${PNG_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${STEAM_INCLUDE_DIR}
)

if (DEFINED AVDL_DIRECT3D11)
else()
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${GLEW_LIBRARIES}      )
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES}      )
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${SDL2_MIXER_LIBRARIES})
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${OPENGL_LIBRARIES}    )

	target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
		${SDL2_INCLUDE_DIRS}
		${SDL2_MIXER_INCLUDE_DIRS}
		${GLEW_INCLUDE_DIRS}
		${OPENGL_INCLUDE_DIR}
	)
endif()

# main executable
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin/)

# install assets
install(FILES ${AVDL_ASSETS} DESTINATION assets)

# packaging using `cpack`
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VENDOR "Afloofdev")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A card game about decisions, love and regret")
set(CPACK_PACKAGE_DESCRIPTION "A card game where all the players are sitting around a table. A rose appears on the table that moves around.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://afloofdev.com/games/rue")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
set(CPACK_NSIS_MODIFY_PATH ON)
include(CPack)
