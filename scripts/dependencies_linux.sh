# temp directory
mkdir avdl_build_dependencies
cd avdl_build_dependencies

SDL_VERSION_LINUX="2.0.20"
SDL_VERSION_LINUX_LINKER_NAME="libSDL2.so"
SDL_VERSION_LINUX_SONAME="libSDL2-2.0.so.0"
SDL_VERSION_LINUX_REAL_NAME="libSDL2-2.0.so.0.18.2"
SDL_VERSION_WINDOWS="2.0.22"
# SDL2_mixer
SDL_MIXER_VERSION_LINUX="2.0.4"
SDL_MIXER_VERSION_LINUX_LINKER_NAME="libSDL2_mixer.so"
SDL_MIXER_VERSION_LINUX_SONAME="libSDL2_mixer-2.0.so.0"
SDL_MIXER_VERSION_LINUX_REAL_NAME="libSDL2_mixer-2.0.so.0.2.2"
SDL_MIXER_VERSION_WINDOWS="2.6.0"
# GLEW
GLEW_VERSION_LINUX="2.2.0"
GLEW_VERSION_LINUX_LINKER_NAME="libGLEW.so"
GLEW_VERSION_LINUX_SONAME="libGLEW.so.2.2"
GLEW_VERSION_LINUX_REAL_NAME="libGLEW.so.2.2.0"
GLEW_VERSION_WINDOWS="2.2.0"
# ogg
OGG_VERSION_LINUX="1.3.5"
OGG_VERSION_LINUX_LINKER_NAME="libogg.so"
OGG_VERSION_LINUX_SONAME="libogg.so.0"
OGG_VERSION_LINUX_REAL_NAME="libogg.so.0.8.5"
OGG_VERSION_WINDOWS="X"
# vorbis
VORBIS_VERSION_LINUX="1.3.7"
VORBIS_VERSION_LINUX_LINKER_NAME="libvorbis.so"
VORBIS_VERSION_LINUX_SONAME="libvorbis.so.0"
VORBIS_VERSION_LINUX_REAL_NAME="libvorbis.so.0.4.9"
VORBIS_VERSION_WINDOWS="X"
# png
PNG_VERSION_LINUX="1.6.39"
PNG_VERSION_LINUX_LINKER_NAME="libpng16.so"
PNG_VERSION_LINUX_SONAME="libpng16.so.16"
PNG_VERSION_LINUX_REAL_NAME="libpng16.so.16.39.0"
PNG_VERSION_WINDOWS="X"
# zlib
ZLIB_VERSION_WINDOWS="1.3.1"
# freetype
FREETYPE_VERSION_LINUX="VER-2-13-1"
FREETYPE_VERSION_LINUX_LINKER_NAME="libfreetype.so"
FREETYPE_VERSION_LINUX_SONAME="libfreetype.so.6"
FREETYPE_VERSION_LINUX_REAL_NAME="libfreetype.so.6.20.0"

# prepare directories
mkdir avdl_dependencies_linux
mkdir avdl_dependencies_linux/linux
mkdir avdl_dependencies_linux/linux/lib
mkdir avdl_dependencies_linux/linux/include
mkdir avdl_dependencies_linux/windows
mkdir avdl_dependencies_linux/windows/lib
mkdir avdl_dependencies_linux/windows/include

# freetype
git clone https://gitlab.freedesktop.org/freetype/freetype freetype
cd freetype
git checkout ${FREETYPE_VERSION_LINUX}
sh autogen.sh
./configure --prefix=./dependency_freetype/
make -j6
make -j6 install
./configure
make -j6
make -j6 install
cd ..
cp freetype/dependency_freetype/freetype/lib/${FREETYPE_VERSION_LINUX_REAL_NAME} avdl_dependencies_linux/linux/lib/${FREETYPE_VERSION_LINUX_SONAME}
