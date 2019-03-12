# Fit Meter Sync 
This tool syncs your 3DS step meter with the Fit Meter via the IR port of the 3DS.

# Contributing
If you have a Wii Fit Meter and a 3DS (or another way to read data from the fit meter), could you create an Issue with the only issue template right now?

Suggestions about the protocol are welcome, as well as pull requests as long as they are somehow relevant to the roadmap or 

# Requirements
* A 3DS
* A Wii U Fit Meter

# Roadmap

## Before v0.1 <-- We are here
* Document parts of the protocol
    * Current roadblocK: I can't sniff IR data and resend them. The timing seems to be important and it is kinda hard with 1 IrDA receiver.
        * I've ordered two IrDA USB dongles, once I get them, I'll resume working on this project.

## v0.1
* Emulate a wii fit meter or wii u (so they'll sync data)

## v0.2
* Sync the steps from the Fit Meter to the 3DS

## v1.0
* Make a nice, simple GUI

## v2.0
* Sync the rest of the fricking ~~owl~~ data

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
