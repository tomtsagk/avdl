
:: required first argument
if "%1"=="" (
	echo "avdl_dependencies_windows.bat: first argument is required as the output folder"
	exit
)

:: get into the cache directory
if not exist ".avdl_cache" mkdir ".avdl_cache"
cd ".avdl_cache"

if not exist "avdl_build_dependencies_windows" mkdir "avdl_build_dependencies_windows"
cd "avdl_build_dependencies_windows"

:: dependency versions
set ZLIB_VERSION_WINDOWS=1.3.1
set SDL_VERSION_WINDOWS=2.30.7
set SDL_MIXER_VERSION_WINDOWS=2.8.0
set GLEW_VERSION_WINDOWS=2.2.0
set FREETYPE_VERSION=VER-2-13-1

:: sdl2
if not exist "SDL2-devel.zip" powershell.exe -Command "Invoke-WebRequest https://github.com/libsdl-org/SDL/releases/download/release-%SDL_VERSION_WINDOWS%/SDL2-devel-%SDL_VERSION_WINDOWS%-VC.zip -OutFile SDL2-devel.zip"
if not exist "./avdl_sdl2" (
	mkdir "./avdl_sdl2"
	unzip SDL2-devel.zip -d avdl_sdl2
)

:: sdl2_mixer
if not exist "SDL2_mixer-devel.zip" powershell.exe -Command "Invoke-WebRequest https://github.com/libsdl-org/SDL_mixer/releases/download/release-%SDL_MIXER_VERSION_WINDOWS%/SDL2_mixer-devel-%SDL_MIXER_VERSION_WINDOWS%-VC.zip -OutFile SDL2_mixer-devel.zip"
if not exist "./avdl_sdl2_mixer" (
	mkdir "./avdl_sdl2_mixer"
	unzip SDL2_mixer-devel.zip -d avdl_sdl2_mixer
)

:: glew
if not exist "glew.zip" powershell.exe -Command "Invoke-WebRequest https://github.com/nigels-com/glew/releases/download/glew-%GLEW_VERSION_WINDOWS%/glew-%GLEW_VERSION_WINDOWS%-win32.zip -OutFile glew.zip"
if not exist "./avdl_glew" (
	mkdir "./avdl_glew"
	unzip glew.zip -d avdl_glew
)

:: zlib
if not exist "zlib.tar.gz" (
	powershell.exe -Command "Invoke-WebRequest http://www.zlib.net/zlib-%ZLIB_VERSION_WINDOWS%.tar.gz -OutFile zlib.tar.gz"
	tar xf zlib.tar.gz
)
cd zlib-%ZLIB_VERSION_WINDOWS%
if exist build (
	del /F /Q build
	rmdir build
)
mkdir build
cd build
cmake ../ . -DCMAKE_INSTALL_PREFIX="../../dependencies"
cmake --build . --config Release
cmake --install .
cd ../../

:: libpng
if not exist libpng git clone git://git.code.sf.net/p/libpng/code libpng
cd libpng
if exist build (
	del /F /Q build
	rmdir build
)
mkdir build
cd build
cmake ../ . -DCMAKE_INSTALL_PREFIX="../../dependencies"
cmake --build . --config Release
cmake --install .
cd ../../

:: freetype
if not exist freetype git clone https://gitlab.freedesktop.org/freetype/freetype freetype
cd freetype
git checkout %FREETYPE_VERSION%
if exist build (
	del /F /Q build
	rmdir build
)
mkdir build
cd build
cmake ../ . -DCMAKE_INSTALL_PREFIX="../../dependencies" -DBUILD_SHARED_LIBS=true -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --install .
cd ../../

:: package
set out_directory=..\..\%1
mkdir "%out_directory%"
mkdir "%out_directory%\windows"
mkdir "%out_directory%\windows\include"
mkdir "%out_directory%\windows\lib"
xcopy /e ".\dependencies" "%out_directory%\windows"
xcopy /e ".\avdl_sdl2\SDL2-%SDL_VERSION_WINDOWS%\include\" "%out_directory%\windows\include"
xcopy /e ".\avdl_sdl2\SDL2-%SDL_VERSION_WINDOWS%\lib\" "%out_directory%\windows\lib"
xcopy /e ".\avdl_sdl2_mixer\SDL2_mixer-%SDL_MIXER_VERSION_WINDOWS%\include\" "%out_directory%\windows\include"
xcopy /e ".\avdl_sdl2_mixer\SDL2_mixer-%SDL_MIXER_VERSION_WINDOWS%\lib\" "%out_directory%\windows\lib"
xcopy /e ".\avdl_glew\glew-%GLEW_VERSION_WINDOWS%\include\" "%out_directory%\windows\include"
xcopy /e ".\avdl_glew\glew-%GLEW_VERSION_WINDOWS%\lib\" "%out_directory%\windows\lib"

:: avdl build dependencies windows
cd ..

:: avdl cache
cd ..
