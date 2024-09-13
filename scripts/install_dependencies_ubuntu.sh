
# make it up to date
apt-get update -y
apt-get upgrade -y

# avdl packaging
apt-get install -y gcc pkg-config ca-certificates imagemagick wget unzip zip

# sdl and sdl_mixer deps
apt-get install -y build-essential  git  make  cmake  autoconf  automake  libtool  pkg-config  libasound2-dev  libpulse-dev  libaudio-dev  libjack-dev  libx11-dev  libxext-dev  libxrandr-dev  libxcursor-dev  libxfixes-dev  libxi-dev  libxss-dev  libgl1-mesa-dev  libdbus-1-dev  libudev-dev  libgles2-mesa-dev  libegl1-mesa-dev  libibus-1.0-dev  fcitx-libs-dev  libsamplerate0-dev  libsndio-dev  libwayland-dev  libxkbcommon-dev  libdrm-dev  libgbm-dev libssl-dev

# glew dependencies
apt-get install -y build-essential libxmu-dev libxi-dev libgl-dev
