# About this folder
* wiifitmeter.lua: This file is a Wireshark 3.0 plugin to dissect the network-layer protocol
	* You can put this in your Wireshark Plugins folder (See *Help -> About Wireshark -> Folders -> (Personal) Lua Plugins*). If you then go to to *Edit -> Preferences -> Protocols -> DLT_USER -> Edit...*, you can click on Add New Entry, set 
the *DLT* to *User 0* and *Payload Protocol* to *FitMeterLink*, it should activate this custom dissector.
