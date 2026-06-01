# 8080 Emulator to Play Space Invaders (OSU CS 467, Spring 2026)

This emulator covers the entire instruction set for the Intel 8080 as well as provides input, audio, and video support to play Space Invaders.

Created by:

- Brandon Arnst-Goodrich
- Joshua Benge
- Patrick Keleher
- David Rambo

## How to run

This software was developed on and tested for Linux.
We confirmed that it builds and runs on Fedora, Ubuntu, Arch, and Cent OS.

You'll need to have on your system:

- `g++`
- `SDL3` development libraries
- ideally `make`
- a copy of the Space Invaders ROM concatenated into a single file

The ROM can be found online among emulator enthusiasts as four files.
Concatenate them as follows:

```sh
cat invaders.h invaders.g invaders.f invaders.e > invaders_rom_file
```

For example, on Fedora, the following will install the SDL3 dependency, build the default emulator target, and then run the emulator:

```sh
sudo dnf install SDL3-devel
make
./emu8080 invaders_rom_file
```

## How to play

(Note: the letters may be entered lowercase. They're represented with capitals here for legibility.)

To quit the game, press either `Q` or `Esc`.

To add credits (i.e. put coins into the arcade cabinet), press `C`.

Press either `1` or `2` for one-player or two-player mode, respectively.

To move left and right, press the `left` and `right` arrow keys or `A` and `D`.

To fire a projectile, press the `up` arrow key, `W`, or `space`.
