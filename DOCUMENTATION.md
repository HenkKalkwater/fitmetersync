# IR
The Fit Meter seems to send data via IrDA-SIR with a bit rate of 115200 bits per second.

# Packet description
The name is not official in any means, but I will use them in the rest of the documentation

| Name          | offset (bytes) | lenght (bytes) | Description                                                                       |
|---------------|----------------|----------------|-----------------------------------------------------------------------------------|
| Magic?        | 0x0            | 0x1            | Always `a5`?                                                                      |
| ConnectionID  | 0x1            | 0x1            | Connection ID, `00` if there is no connection made yet.                           |
| SetupFlag?    | 0x2            | 0x1            | First bit: flag of some sort? 1 if no connection was made yet or wants to stop the connection? |
| BigFlag?      | ^              | ^              | Second bit: flag of some sort? 1 if the packet size is bigger than 63 bytes?      |
| PacketSize    | ^              | ^              | Remaing 6 bits: length of the `Data` (in bytes) if `BigFlag` is not set.          |
| PacketSize    | 0x3 (Not present if `BigFlag` was not set) | 0x1 | Size (in the case that `BigFlag` was set, otherwise data starts here.) |
| Data          | 0x3 (0x4 if `BigFlag` was set) | Y              | Data (see below)                                                  |
| Checksum      | 0x3 + Y (0x4 + Y if `BigFlag` was set) | 0x1            | 8-bit CRC over all the previous bytes                     |

## Data
* `02   `: Accept connection?
* `03 04`: announcing presence?
* `04 04`: polling devices?

# Making a connection

When wanting to make a connection, the Fit Meter sends the following data with a `ConnectionID` of `0x00`:  

    01 03 04 xx

where `xx` is the `ConnectionID a device should use when it wants to communicate with the Fit Meter. All the following
communications should use that `ConnectionID`, otherwise the device will not respond.  

Once the Fit Meter sends `01 04 04 xx` (still with a `ConnectionID` of `00`), you are allowed to connect with it (?). 
To connect, you have to send `02` back (?).  
// TODO
