# Checkers

This work-in-progress 3D checkers implementation aims to be really portable.
Person vs. computer does not (yet) exist.  Rendering uses OpenGL (ES) on most
platforms.

## Installation

Download
[prebuilt binaries](https://github.com/jeandrek/ultra-crossplatform-checkers/releases)
or build from source :)

### Building

For POSIX/X11, Windows, and PSP, edit variables if necessary in `Makefile`,
`Makefile.win`, or `Makefile.psp` respectively and run `make` on the makefile.
Building for PSP requires [PSPDEV](https://github.com/pspdev/pspdev) and custom
firmware.

For Android, edit the variables at the top of `Makefile.android` to point to
the Android SDK tools (and `javac` if not in PATH), and supply a keystore path
in `KEYSTORE`; then run `make -f Makefile.android`.

## Note

Most platforms presently expect `assets` to be in the working directory.
