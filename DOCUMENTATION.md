# IR
The Fit Meter seems to send data via IrDA-SIR with a bit rate of 115200 bits per second.

# Device IDs
So far the fit meter always starts messages with `a5 00 84 01` and the gamepad with `a5 70 81` (need o)

# Packet description
The name is not official in any means, but I will use them in the rest of the documentation

| Name          | offset (bytes) | lenght (bytes) | Description                                                                       |
|---------------|----------------|----------------|-----------------------------------------------------------------------------------|
| Magic?        | 0x0            | 0x1            | Always `59`?                                                                      |
| ConnectionID  | 0x1            | 0x1            | Connection ID, `00` if there is no connection made yet.                           |
| SetupFlag?    | 0x2            | 0x1            | First bit: flag of some sort? 1 if no connection was made yet?                    |
| BigFlag?      |                |                | Second bit: flag of some sort? 1 if the packet size is bigger than 63 bytes?      |
| PacketSize    |                |                | Remaing 6 bits: length of the following bytes (in bytes) if `BigFlag` is not set  |
| PacketSize    | 0x3 (Not here if `BigFlag` was not set) | 0x1            | Size (in the case that `BigFlag` was set, otherwise data starts here.)
| Data          | 0x3 (0x4 if `BigFlag` was set) | Y              | Data                                                                              |
| Checksum      | 0x3 + Y (0x4 + Y if `BigFlag` was set) | 0x1            | 8-bit CRC over the previous bytes                                                 | (

## Actions
* `03 04`: announcing presence?
* `04 04`: polling devices?

# Making a connection


# Handshake?
When pressing the send data button, the fit meter immediately sends 8 bytes.  

* The 1st-4th byte seem to always stay the same. Maybe this is an ID of the fit meter?  
* The 5th byte seems to always be `0x3`.  
* ~~The 6th byte seems to always be `0x4`.~~ No longer true after seeing other peoples data.  
* The 7th byte seems to increase by one each time an attempt to send data is made.  
* The 8th byte is the CRC-8 over all the previous bytes.

After that packet, the Fit Meter will send a burst of frames, several times per second, similar to the first frame sent. The differences are as follows:  

* The 5th byte seems to always be `0x4`  
* The 7th byte seems to increase by 1 every new frame  

Sending the same packet back, with the 5th and 6 byte both being `0x4` and the CRC being updated accordingly, will cause the Fit Meter to tell "Connection Failed" instead of "No connections found". .

Example conversation between a Fit Meter (denoted as *fit*) and the *3DS*:

    3DS -> FIT 05 00 84 01  03 04 0a 50
    3DS <- FIT 05 00 84 01  03 04 64 5d
    3DS -> FIT 05 00 84 01  04 04 f6 bc
    3DS <- FIT 05 00 84 01  04 04 fb 9f
