cmake_minimum_required(VERSION 3.19)

include("avdl_project.cmake")

# set the project name
project("${AVDL_PROJECT_NAME_CODE}" VERSION ${AVDL_VERSION_NAME})

# compile for windows, with dynamic location
add_compile_definitions(
	DD_PLATFORM_NATIVE
	AVDL_DYNAMIC_PKG_LOCATION
	AVDL_OS_WINDOWS
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

# SDL dependency
#set(SDL2_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/libraries/SDL2-2.0.22"     )
if (DEFINED AVDL_DIRECT3D11)
else()
	set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2-2.0.22/include")
	if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
		set(SDL2_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2-2.0.22/lib/x64/SDL2.lib;${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2-2.0.22/lib/x64/SDL2main.lib")
		install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2-2.0.22/lib/x64/SDL2.dll" DESTINATION bin/)
	else ()
		set(SDL2_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2-2.0.22/lib/x86/SDL2.lib;${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2-2.0.22/lib/x86/SDL2main.lib")
		install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2-2.0.22/lib/x86/SDL2.dll" DESTINATION bin/)
	endif ()

	# SDL_mixer dependency
	#set(SDL2_mixer_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libraries/SDL2_mixer-2.6.0/cmake")
	set(SDL2_MIXER_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/include")

	if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
		set(SDL2_MIXER_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/lib/x64/SDL2_mixer.lib")
		install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/lib/x64/SDL2_mixer.dll" DESTINATION bin/)
		install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/lib/x64/optional/libogg-0.dll" DESTINATION bin/)
		#install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/lib/x64/optional/libopus-0.dll" DESTINATION bin/)
		#install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/lib/x64/optional/libopusfile-0.dll" DESTINATION bin/)
	else ()
		set(SDL2_MIXER_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/lib/x86/SDL2_mixer.lib")
		install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/lib/x86/SDL2_mixer.dll" DESTINATION bin/)
		install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/lib/x86/optional/libogg-0.dll" DESTINATION bin/)
		#install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/lib/x86/optional/libopus-0.dll" DESTINATION bin/)
		#install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/SDL2_mixer-2.6.0/lib/x86/optional/libopusfile-0.dll" DESTINATION bin/)
	endif ()

	# glew dependency
	#set(GLEW_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/libraries/glew-2.2.0"      )
	set(GLEW_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/libraries/glew-2.2.0/include")

	if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
		set(GLEW_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/libraries/glew-2.2.0/lib/Release/x64/glew32.lib;${CMAKE_CURRENT_LIST_DIR}/libraries/glew-2.2.0/lib/Release/x64/glew32s.lib")
		install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/glew-2.2.0/bin/Release/x64/glew32.dll" DESTINATION bin/)
	else ()
		set(GLEW_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/libraries/glew-2.2.0/lib/Release/Win32/glew32.lib;${CMAKE_CURRENT_LIST_DIR}/libraries/glew-2.2.0/lib/Release/Win32/glew32s.lib")
		install(FILES "${CMAKE_CURRENT_LIST_DIR}/libraries/glew-2.2.0/bin/Release/Win32/glew32.dll" DESTINATION bin/)
	endif ()

	find_package(OpenGL     REQUIRED)
endif()

# dependency zlib
install(FILES "C:/Program Files (x86)/zlib/bin/zlib.dll" DESTINATION bin/)

# dependency freetype
set(FREETYPE_LIBRARY "C:/Program Files (x86)/freetype/lib/freetype.lib")
set(FREETYPE_INCLUDE_DIR "C:/Program Files (x86)/freetype/include/freetype2")
install(FILES "C:/Program Files (x86)/freetype/bin/freetype.dll" DESTINATION bin/)

# dependency png
set(PNG_LIBRARY "C:/Program Files (x86)/libpng/lib/libpng16.lib")
set(PNG_PNG_INCLUDE_DIR "C:/Program Files (x86)/libpng/include")
install(FILES "C:/Program Files (x86)/libpng/bin/libpng16.dll" DESTINATION bin/)

find_package(PNG	REQUIRED)

if (${AVDL_STEAM})
	set(STEAM_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/AVDL_BUILD/dependencies/windows/include")
	if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
		set(STEAM_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/AVDL_BUILD/dependencies/windows/lib/steam_api64.lib")
		install(FILES "${CMAKE_CURRENT_LIST_DIR}/AVDL_BUILD/dependencies/windows/lib/steam_api64.dll" DESTINATION bin/)
	else()
		set(STEAM_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/AVDL_BUILD/dependencies/windows/lib/steam_api.lib")
		install(FILES "${CMAKE_CURRENT_LIST_DIR}/AVDL_BUILD/dependencies/windows/lib/steam_api.dll" DESTINATION bin/)
	endif()
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${PNG_LIBRARY}         )
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${FREETYPE_LIBRARY}    )
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${STEAM_LIBRARY}       )

# add include directory, for binary, including dependencies
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
	cengine
	include
	${PNG_PNG_INCLUDE_DIR}
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
