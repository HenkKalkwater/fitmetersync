# IR
The Fit Meter seems to send data via IrDA-SIR with a bit rate of 115200 bits per second.

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
