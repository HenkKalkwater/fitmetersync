# IR
The Fit Meter seems to send data via IrDA-SIR with a bit rate of 115200 bits per second.

# Handshake?
When pressing the send data button, the fit meter immediately sends 32 bytes.  
The 4th byte seems to change, but I have no idea by which amount and why.  
The 5th byte seems to increase by one each time an attempt to send data is made.  
~~Bytes 9-32 seem to be always zero?~~ This seemed to be an error in my program, I forgot to divide by 4.  
The rest of the bytes seem to stay the same on the same fit meter.

It is also sending some Irda afterwards, but I have no idea what they mean.
