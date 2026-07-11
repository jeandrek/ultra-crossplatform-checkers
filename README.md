# Checkers

This 3D checkers implementation aims to be really portable.  You can play
against another person locally, against the computer, or over the network.
Rendering uses OpenGL (ES) on most platforms.

## Installation

Download
[prebuilt binaries](https://github.com/jeandrek/ultra-crossplatform-checkers/releases)
or build from source :)

### Building

For POSIX/X11, Windows, and PSP, edit variables if necessary in `Makefile`,
`Makefile.win`, or `Makefile.psp` respectively, and run `make` or
`make -f Makefile.<platform>`.  To build with Bonjour enabled, add
`USE_DNS_SD=1` to the command; you may need to specify its include and library
directories, e.g.

```
CFLAGS='-I../bonjour-sdk' LDFLAGS='-L../bonjour-sdk' make -f Makefile.win USE_DNS_SD=1
```

Building for PSP requires [PSPDEV](https://github.com/pspdev/pspdev) and custom
firmware.

For Android, first edit the variables at the top of `Makefile.android` to
change the Android SDK tool paths if necessary.  You will also need a keystore
for signing.  Then run `make -f Makefile.android KEYSTORE=<path to keystore>`.

For OS X, build with `make -f Makefile.osx`; Bonjour is enabled by default.

## Note

Most platforms presently expect `assets` to be in the working directory.
