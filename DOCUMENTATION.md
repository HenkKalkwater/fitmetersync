# IR
The Fit Meter seems to send data via IrDA-SIR with a bit rate of 115200 bits per second.

# Handshake?
When pressing the send data button, the fit meter immediately sends 8 bytes.  
* The 1st-4th byte seem to always stay the same. Maybe this is an ID of the fit meter?  
* The 5th byte seems to always be `0x3`.  
* The 6th byte seems to always be `0x4`.  
* The 7th byte seems to increase by one each time an attempt to send data is made.  
* The 8th byte seems to change, but I have no idea by which amount and why.  

After that packet, the Fit Meter will send a burst of frames, several times per second, similar to the first frame sent. The differences are as follows:  
* The 5th byte seems to always be `0x4`  
* The 7th byte seems to increase by 1 every new frame  
* The 8th byte changes as well:  
    * The first 4 bits seem to count down from 1111 to 0000. It decreases everytime when the last two bits are 01  
    * The 5th bit follows a pattern of either being `0011` or `1100` over the period of 4 frames.  
    * The 6th bit after that is changing, but I have no idea on what it's based.  
    * The last 2 bits seem to count up from 00 to 11 and rollover  

The last 2 bytes (7th and 8th) in action:  

    10110100 01110101  
    10110101 01110010 
    10110110 01111011 
    10110111 01111100 
    10111000 01010001 
    10111001 01010110 
    10111010 01011111 
    10111011 01011000 
    10111100 01001101 
    10111101 01001010 
    10111110 01000011 
    10111111 01000100 
    11000000 00111110 
    11000001 00111001 
    11000010 00110000 
    11000011 00110111 
    11000100 00100010 
    11000101 00100101 
    11000110 00101100 
    11000111 00101011
