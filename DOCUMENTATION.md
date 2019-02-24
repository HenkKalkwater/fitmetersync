# IR
The Fit Meter seems to send data via IrDA-SIR with a bit rate of 115200 bits per second.

# Handshake?
When pressing the send data button, the fit meter immediately sends 32 bytes.
The 4th byte seems to increase, but I have no idea by which amount.
The 5th byte seems to increase by one each time an attempt to send data is made.
Bytes 9-32 seem to be always zero?

It is also sending some bytes afterwards, but I have no idea what they mean.
