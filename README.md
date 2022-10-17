# liblifx

A **very work-in-progress** C library for interacting with LIFX smart devices. It aims to eventually be compatible with desktop operating systems as well as embedded platforms. Currently will only build as-is for macOS, but should work on Linux and Windows with minor changes to the Makefile.

## Example

See samples/discovery/discovery.c for an example of searching for devices and getting the state of lights.

## TODO

### Library-related

* add a functioning cross-platform (Mac/Linux/Windows) build script/Makefile/whatever.
    * (would be nice to output an .a/.lib file for static linking)
* write docs.
* support for running on big endian platforms.
* properly test on embedded platforms.
* support older C standards, rely on less stdlib functions.

### Device-related

* adjusting device labels.
* reading/setting group and location information.
* support for adjusting light zones on multi-zone devices.
* support for LIFX Switch.


(and probably more that i forgot)
