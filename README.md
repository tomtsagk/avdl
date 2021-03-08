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

These are games made by me, using `avdl`.
Their purpose were mainly to show what this language can do.

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

## How to install

On a linux system, simply execute the following lines from the terminal,
while you are at the project root directory:

    ./configure
    make
    make install

For the last line `make install`, the default location is `/usr` which can be changed during the
`./configure` step by using the following:

    ./configure --install-loc ./mylocaldirectory

You can then go to the `samples/` and try to compile any using the following:

    avdl src.dd

Which should produce an executable `game` file, which when run will display the sample project.

## Documentation

For how to use `avdl` through the command line, together
with arguments, see the man page at `manual/avdl.1`. You can do
so with `man manual/avdl.1` from the project's root directory,
or if you have the project installed, you can run `man avdl` instead.

About the language's syntax, there is currently some documentation in
`doc/avdl.texi` which when installed can be viewed with `info avdl`.
As this project is becoming more stable, the documentation there will
become more up to date.
