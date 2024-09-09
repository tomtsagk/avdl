
:: required first argument
if "%1"=="" (
	echo "avdl_project_windows.bat: first argument is required as the output folder"
	exit
)

:: prepare avdl
if not exist "avdl_windows.zip" powershell.exe -Command "Invoke-WebRequest https:\/github.com/tomtsagk/avdl/releases/download/v0.20.0/avdl_v0.20.0_windows.zip -OutFile avdl_windows.zip"
if not exist AVDL_BUILD mkdir AVDL_BUILD
unzip avdl_windows.zip -d AVDL_BUILD

.\AVDL_BUILD\bin\avdl.exe --cmake

:: get into the cache directory
if not exist ".avdl_cache" mkdir ".avdl_cache"
cd ".avdl_cache"

if not exist "avdl_project_windows" mkdir "avdl_project_windows"
cd "avdl_project_windows"

:: bring dependencies into avdl
if not exist ..\..\libraries mkdir ..\..\libraries
xcopy /e "..\..\AVDL_BUILD\dependencies\windows\libpng" "..\..\libraries"
xcopy /e "..\..\AVDL_BUILD\dependencies\windows\zlib" "..\..\libraries"

:: glew
if not exist glew-win32.zip powershell.exe -Command "Invoke-WebRequest https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip -OutFile glew-win32.zip"
unzip glew-win32.zip -d ..\..\libraries

:: sdl2
if not exist SDL2-devel-VC.zip powershell.exe -Command "Invoke-WebRequest https://github.com/libsdl-org/SDL/releases/download/release-2.0.22/SDL2-devel-2.0.22-VC.zip -OutFile SDL2-devel-VC.zip"
unzip SDL2-devel-VC.zip -d ..\..\libraries

:: sdl2_mixer
if not exist SDL2_mixer-devel-VC.zip powershell.exe -Command "Invoke-WebRequest https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.6.0/SDL2_mixer-devel-2.6.0-VC.zip -OutFile SDL2_mixer-devel-VC.zip"
unzip SDL2_mixer-devel-VC.zip -d ..\..\libraries

:: imagemagick ?

mkdir ..\..\cengine
xcopy /e "..\..\AVDL_BUILD\share\avdl\cengine\" "..\..\cengine"
xcopy /e "..\..\AVDL_BUILD\include\" "..\..\cengine"
cmake ../../ . -DCMAKE_INSTALL_PREFIX="GAME_BUILD"
cmake --build . --config Release
cmake --install .

set out_directory=..\..\%1
if not exist "%out_directory%" mkdir "%out_directory%"
xcopy /e "GAME_BUILD\" "%out_directory%"

:: avdl_project_windows
cd ..

:: avdl cache
cd ..
