# IR
The Fit Meter seems to send data via IrDA-SIR with a bit rate of 115200 bits per second.

# Device IDs
So far the fit meter always starts messages with `a5 00 84 01` and the gamepad with `a5 70 81` (need o)

# Handshake layout (Wii Fit Meter)

| offset (bytes) | lenght (bytes) | Description                                                                        |
|----------------|----------------|------------------------------------------------------------------------------------|
| 0x0            | 0x4            | ID of the device? `a5 00 84 01` for the Fit Meter                                  |
| 0x4            | 0x2            | Action? (See below)                                                                |
| 0x6            | 0x1            | If the action is `03 04`: goes up by one each time a connection is made.           |
|                |                | If the action is `04 04`: counts down to `00`.                                     |
| 0x7            | 0x1            | 8-bit CRC over the previous bytes                                                  |

# Handshake Layout (Wii U)
| offset (bytes) | lenght (bytes) | Description                                                                        |
|----------------|----------------|------------------------------------------------------------------------------------|
| 0x0            | 0x4            | Unknown. Observed to be either `a5 70 81 02` or `a5 70 81 0f`                      |
| 0x7            | 0x1            | 8-bit CRC over the previous bytes                                                  |
## Actions
* `03 04`: announcing presence?
* `04 04`: polling devices?


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
