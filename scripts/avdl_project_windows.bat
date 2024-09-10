
:: required first argument
if "%1"=="" (
	echo "avdl_project_windows.bat: first argument is required as the output folder"
	exit
)

set start_directory=%cd%
set "start_directory=%start_directory:\=/%"

:: prepare avdl
if not exist "avdl_windows.zip" (
	if not "%2"=="" powershell.exe -Command "Invoke-WebRequest %2 -OutFile avdl_windows.zip"
)
if not exist AVDL_BUILD mkdir AVDL_BUILD
unzip avdl_windows.zip -d AVDL_BUILD

.\AVDL_BUILD\bin\avdl.exe --cmake

:: get into the cache directory
if not exist ".avdl_cache" mkdir ".avdl_cache"
cd ".avdl_cache"

if not exist "avdl_project_windows" mkdir "avdl_project_windows"
cd "avdl_project_windows"

:: imagemagick ?

mkdir ..\..\cengine
xcopy /e "..\..\AVDL_BUILD\share\avdl\cengine\" "..\..\cengine"
xcopy /e "..\..\AVDL_BUILD\include\" "..\..\cengine"
if "%3"=="" (
	cmake ../../ . -DCMAKE_INSTALL_PREFIX="GAME_BUILD" -DAVDL_DIRECTORY="%start_directory%/AVDL_BUILD/"
)
else (
	cmake ../../ . -DCMAKE_INSTALL_PREFIX="GAME_BUILD" -DAVDL_DIRECTORY="%start_directory%/AVDL_BUILD/" -DAVDL_STEAM=1
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
