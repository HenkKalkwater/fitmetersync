# IR
The Fit Meter seems to send data via IrDA-SIR with a bit rate of 115200 bits per second.

# Handshake?
When pressing the send data button, the fit meter immediately sends 32 bytes.  
The 3th byte seems to always be 03, except when I reply back with the same data the fit meter sent. Instead, it will be 04 then.
The 7th byte seems to increase by one each time an attempt to send data is made.  
The 8th byte seems to change, but I have no idea by which amount and why.  
~~Bytes 9-32 seem to be always zero?~~ This seemed to be an error in my program, I forgot to divide by 4.  
The rest of the bytes seem to stay the same on the same fit meter.

When replying with the same information
