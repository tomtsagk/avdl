# avdl - Abstract video-game development language

This is a high level programming language for making video games.

The idea behind it, is that a game is described using the language in an abstract
way, similar to how an image file describes the pixels of an image. Then it's
the compiler's responsibility of compiling that into an actual executable
game for specific platforms.

The current plan for the license, is to keep the compiler itself and
all modifications to it free and open-source, but any game made with it will be
completely owned by the user that made it, and it's their choice to monetize
it or not.

## Platforms

`avdl` currently can only be compiled for Linux. However it is possible
to compile for Windows using Linux-emulated tools (like Cygwin).

Games made with this language can currently be compiled for:

* Linux (Not handling dependencies at the moment)

Platforms planned:

* Windows (It's possible to currently cross-compile to it, but it's not straightforward)
* Android (It's possible to compile an `.apk`, but there are a few
	steps that haven't been automated yet)

As the language is still growing, it can currently transpile source files to
the C programming language, which can then be manually compiled for the desired platform.
This is mostly for advanced users that know what they are doing.

## Games made with this language

These are games made by me, using `avdl`, to get an idea of what
the language can do.

* Rue -
[Itch.io](https://darkdimension.itch.io/rue) |
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

## How to compile and install

This project currently supports only compilation for Linux out of the box.
I've managed to compile it manually for Windows, but the process is not
automated yet.

### Dependencies

To compile this project, you will need the following dependencies:

* `make` - to build the project
* `gcc` - used to compile this project
	and to aid the compilation process of `avdl` projects
* `freeglut` - used for windowing
* `sdl2` and `sdl2_mixer` - used for audio
* `glew` - used for advanced opengl functionality

There's a different process to get the dependencies, depending on your
operating system. Here's some examples:

#### Ubuntu 20.04

On a clean install, you will need to install the following packages
to get everything you need:

    apt install git make gcc freeglut3-dev libglew-dev libsdl2-dev libsdl2-mixer-dev

#### Arch Linux

Use the following command to get the required packages:

    pacman -S freeglut glew sdl2 sdl2_mixer make gcc

### Compilation

On a linux system, simply execute the following lines from the terminal,
while you are at the project root directory:

    make
    make install

The default install location is `/usr/local`. To change that, apply a custom `prefix`
value like below:

    make prefix=/usr
    make prefix=/usr install

## Documentation

For how to use `avdl` through the command line, together
with arguments, see the man page at `manual/avdl.1`. You can do
so with `man manual/avdl.1` from the project's root directory,
or if you have the project installed, you can run `man avdl` instead.

About the language's syntax, there is currently some documentation in
`doc/avdl.texi` which when installed can be viewed with `info avdl`.
As this project is becoming more stable, the documentation there will
become more up to date.
