
:: required first argument - output directory
if "%1"=="" (
	echo "avdl_project_windows.bat: first argument is required as the output directory"
	exit
)

:: required second argument - avdl directory
if "%2"=="" (
	echo "avdl_project_windows.bat: second argument is required as the avdl directory"
	exit
)

set start_directory=%cd%
set "start_directory=%start_directory:\=/%"

:: get into the cache directory
if not exist ".avdl_cache" mkdir ".avdl_cache"
cd ".avdl_cache"

if not exist "avdl_project_windows" mkdir "avdl_project_windows"
cd "avdl_project_windows"

:: imagemagick ?

mkdir ..\..\cengine
xcopy /e "%start_directory%\%2\share\avdl\cengine\" "..\..\cengine"
xcopy /e "%start_directory%\%2\include\" "..\..\cengine"
if "%3"=="" (
	cmake ../../ . -DCMAKE_INSTALL_PREFIX="GAME_BUILD" -DAVDL_DIRECTORY="%start_directory%/%2/"
)

if not "%3"=="" (
	cmake ../../ . -DCMAKE_INSTALL_PREFIX="GAME_BUILD" -DAVDL_DIRECTORY="%start_directory%/%2/" -DAVDL_STEAM=1
)

cmake --build . --config Release
cmake --install .

set out_directory=..\..\%1
if not exist "%out_directory%" mkdir "%out_directory%"
xcopy /e "GAME_BUILD\" "%out_directory%"

:: avdl_project_windows
cd ..

:: avdl cache
cd ..
