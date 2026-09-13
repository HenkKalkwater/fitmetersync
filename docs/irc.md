# IRC
Handles connection setup, frame synchronisation and frame integrity.

This protocol seems to be built in the Wii U OS in [nsysccr](https://wut.devkitpro.org/group__nsysccr__cdc__irda.html) 
and [WUT libirc](https://github.com/devkitPro/wut/tree/master/libraries/libirc/include/irc). On the 3DS, it is implemented
in [the ir:USER system service](https://www.3dbrew.org/wiki/IR_Services#IR_Service_%22ir:USER%22)

## Frame layout

| Name           | Offset (bytes)                            | Length (bytes) | Value description                                                                 |
|----------------|-------------------------------------------|----------------|-----------------------------------------------------------------------------------|
| Magic          | 0x0                                       | 0x1            | Always `A5`                                                                       |
| ConnectionID   | 0x1                                       | 0x1            | Connection ID, `00` if there is no connection made yet.                           |
| ControlFlag    | 0x2                                       | 0x1            | First bit: 1 if it is a control frame (no payload)                                |
| LargeFlag      | ^                                         | ^              | Second bit: 1 if the payload size > 0x3F: lower bits of size stored in next byte  |
| FrameSize      | ^                                         | ^              | Remaing 6 bits: length of the `Data` (in bytes) (High bits if `LargeFlag`)        |
| FrameSize      | 0x3 (Only present if `LargeFlag` was set) | 0x1            | Low bitsSize (in the case that `LargeFlag`)                                       |
| Payload        | 0x3 (0x4 if `LargeFlag` was set)          | Y              | Data (see below)                                                                  |
| Checksum       | 0x3 + Y (0x4 + Y if `LargeFlag` was set)  | 0x1            | 8-bit CRC over all the previous bytes                                             |

### Payload when the control flag is set
The first byte determines what frame is sent: [`0x01` to create a connection](#createconnection),
[`0x02` to accept a connection](#acceptconnection) or [`0x0F` to close the current connection](#closeconnection).

#### CreateConnection
| Name         | Offset (bytes) | Length (bytes) | Value description                                                  |
|--------------|----------------|----------------|--------------------------------------------------------------------|
| Command      | 0x0            | 0x1            | `0x01` to initiate a connection                                    |
| TargetType   | 0x1            | 0x1            | Target device type (see [device types table below](#device-types)) |
| SourceType   | 0x1            | 0x1            | Source device type (see [device types table below](#device-types)) |
| ConnectionId | 0x3            | 0x1            | The ConnectionID to use when replying to this advertisement        |

#### AcceptConnection

| Name         | Offset (bytes) | Length (bytes) | Value description                               |
|--------------|----------------|----------------|-------------------------------------------------|
| Command      | 0x0            | 0x1            | `0x02` to acknowledge a connection initiation   |

### CloseConnection
| Name         | Offset (bytes) | Length (bytes) | Value description                               |
|--------------|----------------|----------------|-------------------------------------------------|
| Command      | 0x0            | 0x1            | `0x0F` to close the current connection          |

#### Device types

| Value | Description |
|-------|-------------|
| 0x03  | Wii U       |
| 0x04  | Fit Meter   |

### Payload when the control flag is not set
| Name            | Offset (bytes) | Length (bytes) | Value description                               |
|-----------------|----------------|----------------|-------------------------------------------------|
| Response length | 0x0            | 0x2            | Expected reply size                             |
| Payload         | 0x2            | Y              | Data (see below)                                |

## Connection flow

1. After holding the middle button on the Fit Meter, it sends a CreateConnection control frame.
2. Wii U replies with an AcceptConnection control frame.
3. Devices exchange data [see ircu.md](irc.md#connection-flow)
4. Wii U sends a CloseConnection control frame.

## Related documentation
* [3dbrew IRUSER Shared Memory/Packet Structure](https://www.3dbrew.org/wiki/IRUSER_Shared_Memory#Packet_structure):
  contains an incomplete frame layout.
* [DevkitPro Nsysccr_cdc_irda](https://wut.devkitpro.org/group__nsysccr__cdc__irda.html#structCCRCDCIrdaSmallPacketHeader):
  contains an incomplete frame layout and the functions which are called by the Wii U to send and receive data.