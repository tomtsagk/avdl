
# output directory
if [ -z "$1" ]
then
        echo "avdl: dependencies_linux.sh: first argument is required as the output folder"
	exit
fi

# get inside cache directory
mkdir .avdl_cache
cd .avdl_cache

# temp directory
mkdir avdl_build_dependencies
cd avdl_build_dependencies

set -e

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

out_directory="../../$1"

# prepare directories
mkdir ${out_directory}
mkdir ${out_directory}/linux
mkdir ${out_directory}/linux/lib
mkdir ${out_directory}/linux/include
mkdir ${out_directory}/windows
mkdir ${out_directory}/windows/lib
mkdir ${out_directory}/windows/include

# freetype
git clone https://gitlab.freedesktop.org/freetype/freetype freetype
cd freetype
git checkout ${FREETYPE_VERSION_LINUX}
sh autogen.sh
./configure --prefix=$(pwd)/../dependencies/
make -j6
make -j6 install
make clean
./configure --prefix=/usr
make -j6
make -j6 install
cd ..
cp dependencies/lib/${FREETYPE_VERSION_LINUX_REAL_NAME} ${out_directory}/linux/lib/${FREETYPE_VERSION_LINUX_SONAME}

# libpng
git clone git://git.code.sf.net/p/libpng/code libpng
cd libpng
git checkout v${PNG_VERSION_LINUX}
./configure --prefix=$(pwd)/../dependencies
make -j6
make -j6 install
./configure
make -j6
make -j6 install
cd ..
cp dependencies/lib/${PNG_VERSION_LINUX_REAL_NAME} ${out_directory}/linux/lib/${PNG_VERSION_LINUX_SONAME}

# ogg
git clone https://gitlab.xiph.org/xiph/ogg.git ogg
cd ogg
git checkout v${OGG_VERSION_LINUX}
./autogen.sh
./configure --prefix=$(pwd)/../dependencies
make -j6
make -j6 install
./autogen.sh
./configure
make -j6
make -j6 install
cd ..
cp dependencies/lib/${OGG_VERSION_LINUX_REAL_NAME} ${out_directory}/linux/lib/${OGG_VERSION_LINUX_SONAME}

# vorbis
git clone https://gitlab.xiph.org/xiph/vorbis.git vorbis
cd vorbis
git checkout v${VORBIS_VERSION_LINUX}
./autogen.sh
./configure --prefix=$(pwd)/../dependencies
make
make install
cd ..
cp dependencies/lib/${VORBIS_VERSION_LINUX_REAL_NAME} ${out_directory}/linux/lib/${VORBIS_VERSION_LINUX_SONAME}

# vorbis system
git clone https://gitlab.xiph.org/xiph/vorbis.git vorbis2
cd vorbis2
git checkout v${VORBIS_VERSION_LINUX}
./autogen.sh
./configure
make
make install
cd ..

# sdl
export C_INCLUDE_PATH=$(pwd)/${out_directory}/linux/include
export LIBRARY_PATH=$(pwd)/${out_directory}/lib:$(pwd)/${out_directory}/lib64
git clone https://github.com/libsdl-org/SDL SDL
cd SDL
git checkout release-$SDL_VERSION_LINUX
mkdir build
cd build
../configure --prefix=$(pwd)/../../dependencies
make -j6
make -j6 install
../configure
make -j6
make -j6 install
cd ../..
cp dependencies/lib/${SDL_VERSION_LINUX_REAL_NAME} ${out_directory}/linux/lib/${SDL_VERSION_LINUX_SONAME}

# sdl_mixer
git clone https://github.com/libsdl-org/SDL_mixer SDL_mixer
cd SDL_mixer
git checkout release-$SDL_MIXER_VERSION_LINUX
mkdir build
cd build
../configure --prefix=$(pwd)/../../dependencies --enable-music-ogg-vorbis
make -j6
make -j6 install
../configure --enable-music-ogg-vorbis
make -j6
make -j6 install
cd ../..
cp dependencies/lib/${SDL_MIXER_VERSION_LINUX_REAL_NAME} ${out_directory}/linux/lib/${SDL_MIXER_VERSION_LINUX_SONAME}

# glew
wget https://github.com/nigels-com/glew/releases/download/glew-${GLEW_VERSION_LINUX}/glew-${GLEW_VERSION_LINUX}.tgz
tar xf glew-${GLEW_VERSION_LINUX}.tgz
cd glew-${GLEW_VERSION_LINUX}
make GLEW_DEST=$(pwd)/../dependencies
make GLEW_DEST=$(pwd)/../dependencies install
make clean
make
make install
cd ..
cp dependencies/lib64/${GLEW_VERSION_LINUX_REAL_NAME} ${out_directory}/linux/lib/${GLEW_VERSION_LINUX_SONAME}

#    - name: steamworks
#      if: ${{ steps.cache.outputs.cache-hit != 'true' }}
#      env:
#        steamworks_api_link: ${{ secrets.STEAMWORKS_API_LINK }}
#      run: |
#        wget ${steamworks_api_link} --quiet
#        unzip steamworks_sdk.zip
#        mv sdk/redistributable_bin/linux64/libsteam_api.so ${out_directory}/linux/lib
#        mv sdk/redistributable_bin/steam_api.dll ${out_directory}/windows/lib
#        mv sdk/redistributable_bin/steam_api.lib ${out_directory}/windows/lib
#        mv sdk/redistributable_bin/win64/steam_api64.dll ${out_directory}/windows/lib
#        mv sdk/redistributable_bin/win64/steam_api64.lib ${out_directory}/windows/lib
#        cp sdk/public/steam/*.h ${out_directory}/linux/include
#        cp sdk/public/steam/*.h ${out_directory}/windows/include

# collect headers
cp -d -r dependencies/include/* ${out_directory}/linux/include

# temp directory
cd ..

# avdl cache
cd ..
