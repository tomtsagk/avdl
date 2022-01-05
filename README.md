# avdl - Abstract video-game development language

![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/tomtsagk/avdl)
![Travis (.com)](https://img.shields.io/travis/com/tomtsagk/avdl)
[![Coverage Status](https://coveralls.io/repos/github/tomtsagk/avdl/badge.svg?branch=github)](https://coveralls.io/github/tomtsagk/avdl?branch=github)

This is a high level programming language for making video games.

The idea behind it, is that a game is described using the language in an abstract
way. Then it's the compiler's responsibility to compile that into an
actual executable game for specific platforms.

Published under the GPL-3 license, this project can be used for free.
Any game created with `avdl` fully belongs to the person that made it,
and they can do what they like with it.

## Games made with this language

These are games made by me, using `avdl`, to get an idea of what
the language can do.

* Rue -
[Itch.io](https://darkdimension.itch.io/rue.html) |
[GameJolt](https://gamejolt.com/games/rue/632453) |
[Google Play](https://play.google.com/store/apps/details?id=org.darkdimension.rue)

* The king is gone -
[Steam](https://store.steampowered.com/app/1468820/) |
[Itch.io](https://darkdimension.itch.io/the-king-is-gone) |
[GameJolt](https://gamejolt.com/games/the-king-is-gone/518056) |
[Google Play](https://play.google.com/store/apps/details?id=org.darkdimension.the_king_is_gone)

* Shuffled Nightmares -
[Steam](https://store.steampowered.com/app/1289510/) |
[Itch.io](https://darkdimension.itch.io/shuffled-nightmares) |
[GameJolt](https://gamejolt.com/games/shuffled_nightmares/484001) |
[Google Play](https://play.google.com/store/apps/details?id=org.darkdimension.shuffled_nightmares)

## Available on these Linux Distros

You can find this project packaged for the following Linux distributions:

### Ubuntu 20.04.2 LTS

Add the Personal Package Archive (PPA) to your system,
then update repositories and install:

    add-apt-repository ppa:darkdimension/avdl
    apt-get update
    apt-get install avdl

Note: These commands require `root` permissions, usually aquired with
the `sudo` command.

### Arch Linux - AUR

[![AUR version](https://img.shields.io/aur/version/avdl)](https://aur.archlinux.org/packages/avdl/)

You can find this project in the Arch User Repository (AUR). Make sure
to install the dependencies listed here:

[https://aur.archlinux.org/packages/avdl/](https://aur.archlinux.org/packages/avdl/)

Using the command line, move to an empty directory and run:

    git clone https://aur.archlinux.org/avdl.git
    makepkg
    pacman -U avdl-<version>-<arch>.pkg.tar.zst

Note: The command `pacman -U` needs `root` permissions, as it is
used to install packaged from a local file.

### Fedora 34, Fedora 35, Fedora rawhide

[![Copr build status](https://copr.fedorainfracloud.org/coprs/darkdimension/avdl/package/avdl/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/darkdimension/avdl/package/avdl/)

Enable the repository using the package manager, and install:

    dnf copr enable darkdimension/avdl
    dnf install avdl

Note: These commands may require `root` permissions.

## How to compile and install manually

This project currently supports only compilation for Linux out of the box.
I've managed to compile it manually for Windows, but the process is not
automated yet.

### Dependencies

To compile this project, you will need the following dependencies:

* `make` - to build the project
* `gcc` - used to compile this project
	and to aid the compilation process of `avdl` projects
* `sdl2` and `sdl2_mixer` - used for windowing and audio
* `glew` - used for advanced opengl functionality

There's a different process to get the dependencies, depending on your
operating system. Here's some examples:

#### Ubuntu 20.04 LTS

On a clean install, you will need to install the following packages
to get everything you need:

    apt install git make gcc libglew-dev libsdl2-dev libsdl2-mixer-dev

#### Arch Linux

Use the following command to get the required packages:

    pacman -S glew sdl2 sdl2_mixer make gcc

### Fedora 34, Fedora 35, Fedora rawhide

Use the following command to get the required packages:

    dnf install make gcc glew-devel SDL2-devel SDL2_mixer-devel

### Alpine Linux

Use the following command to get the required packages:

    apk add make git gcc musl-dev glew-dev sdl2-dev sdl2_mixer-dev

### Compilation - Linux

On a linux system, simply execute the following lines from the terminal,
while you are at the project root directory:

    make
    make install

The default install location is `/usr/local`. To change that, apply a custom `prefix`
value like below:

    make prefix=/usr
    make prefix=/usr install

## Documentation

There is a new tutorial that explains the basics here:
[https://darkdimension.org/avdl.html](https://darkdimension.org/avdl.html)

For how to use `avdl` through the command line, together
with arguments, see the man page at `manual/avdl.1`. You can do
so with `man manual/avdl.1` from the project's root directory,
or if you have the project installed, you can run `man avdl` instead.
