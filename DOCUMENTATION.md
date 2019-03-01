# IR
The Fit Meter seems to send data via IrDA-SIR with a bit rate of 115200 bits per second.

# Device IDs
So far the fit meter always starts messages with `a5 00 84 01` and the gamepad with `a5 70 81` (need o)

# Packet description
The name is not official in any means, but I will use them in the rest of the documentation

| Name          | offset (bytes) | lenght (bytes) | Description                                                                       |
|---------------|----------------|----------------|-----------------------------------------------------------------------------------|
| Magic?        | 0x0            | 0x1            | Always `a5`?                                                                      |
| ConnectionID  | 0x1            | 0x1            | Connection ID, `00` if there is no connection made yet.                           |
| SetupFlag?    | 0x2            | 0x1            | First bit: flag of some sort? 1 if no connection was made yet or wants to stop the connection? |
| BigFlag?      |                |                | Second bit: flag of some sort? 1 if the packet size is bigger than 63 bytes?      |
| PacketSize    |                |                | Remaing 6 bits: length of the `Data` (in bytes) if `BigFlag` is not set.          |
| PacketSize    | 0x3 (Not present if `BigFlag` was not set) | 0x1 | Size (in the case that `BigFlag` was set, otherwise data starts here.) |
| Data          | 0x3 (0x4 if `BigFlag` was set) | Y              | Data (see below)                                                  |
| Checksum      | 0x3 + Y (0x4 + Y if `BigFlag` was set) | 0x1            | 8-bit CRC over all the previous bytes                     |

## Data
* `02   `: Accept connection?
* `03 04`: announcing presence?
* `04 04`: polling devices?

# Making a connection
// TODO
