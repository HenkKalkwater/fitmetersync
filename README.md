# Fit Meter Sync 

> **Warning**
> This software is not ready for use yet. It is a non-functional prototype at most.
> I am not responsible for data loss, bricked 3DSs, dead SD cards, your cat being lost,
> or a thermonuclear crisis.

Implementation and documentation of the Wii U Fit Meter protocol.

## Documentation
See the [docs/](docs/overview.md) folder for the protocol documentation.

## Contributing
If you have a Wii Fit Meter and a 3DS (or another way to read data from the fit meter), could you create an Issue with the only issue template right now?

Suggestions about the protocol are welcome, as well as pull requests as long as they are somehow relevant to the roadmap or 

## Requirements
* A 3DS or a USB to Serial to IrDA adapter capable of 115200 baud rate
* A Wii U Fit Meter

## Current features
* [x] Retrieving identity, current datetime, METs/min, Altitude/min, Activity tag/min, kcals/day and steps/day from Fit Meter
* [x] Simple command-line application for retrieving data
* [ ] Decompressing the data from the Fit Meter

## Planned features
* Web application for viewing the data from the Fit Meter
* 3DS application for viewing the data from the Fit Meter
  * Perhaps even emulating a Wii Fit Meter

# Directory overview
```
├── crates             # Rust source code root
│   ├── cli            # Command line application
│   ├── irc            # IrDA communication library (IRC and IRCU protocols)
│   ├── samu           # Wii Fit Meter data library (command definitions, decompressions, …)
│   └── wasm           # WASM library (so the code can run in a browser)
├── LICENSES           # Text of used licenses
├── misc               # Other code (WireShark dissectors et cetera)
└── testing            # Dumps of communications between Wii U and Fit Meter
```

## Building
This project uses Cargo

```sh
cargo build
```

### WASM

wasm-pack

## Thanks to:
* The creator of the new-hblauncher for the Makefile
* HenkKalkwater (creator)
* mrbob312 for modifying it

## Documentation gathered from
* [This page on iFixit telling the Wii Fit Meter uses IrDA](https://www.ifixit.com/Answers/View/205720/Fit+Meter+IR+emitter+freq#answer205742). This was the spark that leaded me to write this project.
* [The contributers to this project](https://github.com/RedInquisitive/3DS-Remote), because I lost a lot of hope until I encountered that project.
* [This guy who is reverse engineering the Pokewalker, which seems to be similar to the Wii Fit Meter](https://gbatemp.net/threads/pokewalker-hacking.419462/)
* The [3DBREW page about IR](https://www.3dbrew.org/wiki/IR_Services)

## Included code from
* [This CRC-implementation](http://www.rajivchakravorty.com/source-code/uncertainty/multimedia-sim/html/crc8_8c-source.html)
