# Overview

The Wii U Fit Meter communicates with the Wii U over InfraRed. It is split up into three layers:

* **IrDA-SIR** @ 115 200 baud: Physical layer
* **[IRC: Data link layer](irc.md)**: frame synchronisation, error detection and connection id negotiation
* **[IRCU: Tranport layer](ircu.md)**: segmentation and acknowledgement
* **[SAMU](samu.md)**: application layer data transfer


## Reverse Engineering
Most of this information is found by reverse engineering Wii Fit U Quick Check, which is a freely available build of
Wii Fit U, which only can do the Body Check and the Fit Meter Sync. It handily contains debug symbols.

I use [Ghidra](https://github.com/NationalSecurityAgency/ghidra) with the [RPX Loader Plugin](https://github.com/Maschell/GhidraRPXLoader).

