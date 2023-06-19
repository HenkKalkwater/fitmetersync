# Fit Meter Sync 

> **Warning**
> This software is not ready for use yet. It is a non-functional prototype at most.
> I am not responsible for data loss, bricked 3DSs, dead SD cards, your cat being lost,
> or a thermonuclear crisis.

This tool syncs your 3DS step meter with the Fit Meter via the IR port of the 3DS.

# Contributing
If you have a Wii Fit Meter and a 3DS (or another way to read data from the fit meter), could you create an Issue with the only issue template right now?

Suggestions about the protocol are welcome, as well as pull requests as long as they are somehow relevant to the roadmap or 

# Requirements
* A 3DS
* A Wii U Fit Meter

# Current features
* [x] Recording infrared signals and saving them to pcap (Wireshark-compatible) files.
* [ ] TODO: add a wireshark protocol definition
* [ ] An attempt to emulate a Wii U, which fails for some reason still unknown

# Roadmap

## Before v0.1 <-- We are here
* [x] Document the outer protocol
* [ ] Document the inner protocol
	* [ ] Document a possible inner-inner protocol

## v0.1
* [ ] Emulate a wii fit meter or wii u (so they'll sync data)

## v0.2
* [ ] Sync the steps from the Fit Meter to the 3DS

## v1.0
* [ ] Make a nice, simple GUI

## v2.0
* [ ] Sync the rest of the fricking ~~owl~~ data

# Directory overview
```
├── fms-3ds            # Project directory for the old 3DS application
│   └── src
├── fms-cli            # Project directory for the command-line based Linux application
│   └── src
├── libfms             # Cross-platform library for implementing the protocol and platform abstractions
│   └── src
│       ├── core       # Code for data types used in other folders
│       ├── platform   # Platform abstractions for threads, IrDA adapters etc
│       └── protocol   # Protocol implementations
├── LICENSES           # Text of used licenses
├── misc               # Other code
└── testing            # Dumps of communications between Wii U and Fit Meter
```

# Building
This projct uses [Meson](https://mesonbuild.com/), because Makefiles are a pain in the behind to use and write.

## Build steps (not cross-compiling)

```sh
meson setup $builddir # Substitute $builddir with wherever you want to output your build files to
cd $builddir
meson compile
```

## Build steps (Cross-compiling for the 3DS)
1. Make sure [devkitpro](https://devkitpro.org/) is installed with the packages for compiling to a 3DS.
2. Edit 3ds_cross.txt and make sure that `devkitpro` is set to the path of your devkitpro installation
3. Invoke meson as follows:
  ```sh
  meson setup $builddir --cross-file=3ds_cross.txt # Substitute $builddir with wherever you want to output your build files to
  cd $builddir
  meson compile
  ```

# Thanks to:
* The creator of the new-hblauncher for the Makefile
* HenkKalkwater (creator)
* mrbob312 for modifying it

# Documentation gathered from
* [This page on iFixit telling the Wii Fit Meter uses IrDA](https://www.ifixit.com/Answers/View/205720/Fit+Meter+IR+emitter+freq#answer205742). This was the spark that leaded me to write this project.
* [The contributers to this project](https://github.com/RedInquisitive/3DS-Remote), because I lost a lot of hope until I encountered that project.
* [This guy who is reverse engineering the Pokewalker, which seems to be similar to the Wii Fit Meter](https://gbatemp.net/threads/pokewalker-hacking.419462/)
* The [3DBREW page about IR](https://www.3dbrew.org/wiki/IR_Services)

# Included code from
* [This CRC-implementation](http://www.rajivchakravorty.com/source-code/uncertainty/multimedia-sim/html/crc8_8c-source.html)
