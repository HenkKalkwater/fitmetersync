use crate::crc::{irc_crc, irc_crc_ioslice_continue};
use crate::error::{IrcError, IrcResult};
use std::io::{Cursor, IoSlice, IoSliceMut, Read, Write};

pub struct Irc<Transport: Read + Write> {
    transport: Transport,
    connection_id: u8,
    receive_buffer: Box<[u8; 0x20]>,
    /// Whether the transport has just read the MAGIC byte
    synced: bool
}

const RETRY_COUNT: usize = 3;

const CONNECTION_ID_ANY: u8 = 0x00;
const PACKET_HEADER_SIZE_SMALL: usize = 3;
const PACKET_HEADER_SIZE_LARGE: usize = 4;
const PACKET_HEADER_RECEIVE_SIZE: usize = 2;
const PACKET_TRAILER_SIZE: usize = 1;

const PAYLOAD_THRESHOLD: usize = 0x3F;
const PACKET_MAGIC: u8 = 0xA5;

#[repr(u8)]
#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum Target {
    WiiU     = 0x03,
    FitMeter = 0x04
}

impl From<Target> for u8 {
    fn from(t: Target) -> u8 {
        t as u8
    }
}

impl TryFrom<u8> for Target {
    type Error = ();

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            0x03 => Ok(Target::WiiU),
            0x04 => Ok(Target::FitMeter),
            _ => Err(())
        }
    }
}

#[derive(Debug, Eq, PartialEq)]
pub struct PacketBody {
    pub response_length: u16,
    pub payload_length: u16
}

/// Represents the header of an IRC packet.
///
/// # Byte layout:
///
/// | Name           | Offset (bytes)                            | Length (bytes) | Value description                                                                 |
/// |----------------|-------------------------------------------|----------------|-----------------------------------------------------------------------------------|
/// | Magic          | 0x0                                       | 0x1            | Always `A5`?                                                                      |
/// | ConnectionID   | 0x1                                       | 0x1            | Connection ID, `00` if there is no connection made yet.                           |
/// | ControlFlag    | 0x2                                       | 0x1            | First bit: 1 if it is a control packet (no payload)                               |
/// | LargeFlag      | ^                                         | ^              | Second bit: 1 if the payload size > 0x3F: lower bits of size stored in next byte  |
/// | PacketSize     | ^                                         | ^              | Remaing 6 bits: length of the `Data` (in bytes) (High bits if `LargeFlag`         |
/// | PacketSize     | 0x3 (Only present if `LargeFlag` was set) | 0x1            | Low bitsSize (in the case that `LargeFlag`)                                       |
/// | Payload        | 0x3 (0x4 if `LargeFlag` was set)          | Y              | Data (see below)                                                                  |
/// | Checksum       | 0x3 + Y (0x4 + Y if `LargeFlag` was set)  | 0x1            | 8-bit CRC over all the previous bytes                                             |
///
/// ## Payload when the control flag is set
/// | Name         | Offset (bytes) | Length (bytes) | Value description                               |
/// |--------------|----------------|----------------|-------------------------------------------------|
/// | Command      | 0x0            | 0x1            | `0x01` to initiate a connection                 |
/// | Command      | ^              | ^              | `0x02` to acknowledge a connection initiation   |
/// | Command      | ^              | ^              | `0x0F` to close the current connection          |
///
/// If the `Command` is `0x01`, the payload contains the following data as well.
///
/// | Name         | Offset (bytes) | Length (bytes) | Value description                                           |
/// |--------------|----------------|----------------|-------------------------------------------------------------|
/// | Unknown      | 0x1            | 0x2            | Unknown. The fit meter always sends `0x03 0x04`             |
/// | Unknown      | 0x1            | 0x2            | Unknown. The fit meter always sends `0x03 0x04`             |
/// | ConnectionId | 0x3            | 0x1            | The ConnectionID to use when replying to this advertisement |
///
/// ## Payload when the control flag is not set
/// | Name            | Offset (bytes) | Length (bytes) | Value description                               |
/// | Response length | 0x0            | 0x2            | Expected reply size                             |
/// | Payload         | 0x2            | Y              | Data (see below)                                |
///
#[derive(Debug, Eq, PartialEq)]
pub enum PacketHeader {
    /// Control packet used to initiate a connection.
    CreateConnection(Target, Target, u8),
    /// Control packet used to accept a connection, initiated by a CreateConnection packet.
    AcceptConnection,
    /// Control packet used to close the connection.
    CloseConnection,
    /// Data packet
    Payload(PacketBody)
}

const PACKET_TYPE_CREATE_CONNECTION : u8 = 0x01;
const PACKET_TYPE_ACCEPT_CONNECTION : u8 = 0x02;
const PACKET_TYPE_CLOSE_CONNECTION  : u8 = 0x0F;

impl<'a> PacketHeader {
    pub fn control_flag(&self) -> bool {
        match self {
            PacketHeader::CreateConnection(_, _, _) => true,
            PacketHeader::AcceptConnection          => true,
            PacketHeader::CloseConnection           => true,
            PacketHeader::Payload(_)                => false
        }
    }

    pub fn large_flag(&self) -> bool {
        self.payload_length() > PAYLOAD_THRESHOLD
    }

    pub fn header_length(&self) -> usize {
        let header_size = if self.large_flag() {
            PACKET_HEADER_SIZE_LARGE
        } else {
            PACKET_HEADER_SIZE_SMALL
        };

        let payload_length = if self.control_flag() {
            self.payload_length()
        } else {
            // Length field for payload
            PACKET_HEADER_RECEIVE_SIZE
        };

        header_size + payload_length
    }

    pub fn payload_length(&self) -> usize {
        match self {
            PacketHeader::CreateConnection(_, _, _) => 4,
            PacketHeader::AcceptConnection          => 1,
            PacketHeader::CloseConnection           => 1,
            PacketHeader::Payload (body)            => PACKET_HEADER_RECEIVE_SIZE + body.payload_length as usize
        }
    }

    pub fn fill_header(&self, header: &mut [u8]) {
        debug_assert!(header.len() >= if self.control_flag() { self.payload_length() } else { PACKET_HEADER_RECEIVE_SIZE });
        match self {
            PacketHeader::CreateConnection(source_device, target_device, connection_id) => {
                header[0] = PACKET_TYPE_CREATE_CONNECTION;
                header[1] = (*source_device).into();
                header[2] = (*target_device).into();
                header[3] = *connection_id;

            },
            PacketHeader::AcceptConnection => {
                header[0] = PACKET_TYPE_ACCEPT_CONNECTION;
            },
            PacketHeader::CloseConnection => {
                header[0] = PACKET_TYPE_CLOSE_CONNECTION;
            },
            PacketHeader::Payload (body) => {
                let receive_bytes = body.response_length.to_be_bytes();
                header[..PACKET_HEADER_RECEIVE_SIZE].copy_from_slice(&receive_bytes)
            }
        }

    }
}

impl<Transport: Read + Write> Irc<Transport> {

    /// Create a new IRC connection with the underlying transport
    pub fn new(transport: Transport) -> Self {
        Irc {
            transport,
            connection_id: CONNECTION_ID_ANY,
            receive_buffer: Box::new([0; 32]),
            synced: true
        }
    }

    /// Copies packet_header to the given buffer. `buffer` must be at least
    /// `packet_header.header_length()` bytes long.
    fn fill_header_base(&self, buffer: &mut [u8], packet_header: &PacketHeader) {
        debug_assert!(buffer.len() >= packet_header.header_length());

        let payload_len = packet_header.payload_length();
        let is_large = packet_header.large_flag();
        let is_special = packet_header.control_flag();

        buffer[0] = PACKET_MAGIC;
        buffer[1] = self.connection_id;
        buffer[2] = (is_special as u8) << 7
            | (is_large   as u8) << 6;

        let next: usize = if is_large {
            buffer[2] |= ((payload_len >> 8) & 0x3F) as u8;
            buffer[3]  = ( payload_len       & 0xFF) as u8;
            4
        } else {
            buffer[2] |= ( payload_len       & 0xFF) as u8;
            3
        };

        packet_header.fill_header(&mut buffer[next..]);
    }

    /// Receives a packet from the transport layer
    ///
    /// # Arguments:
    /// * `bufs`: The buffers to receive into. Will be filled with the packet payload if a
    ///           non-control packet is received
    ///
    /// # Returns:
    /// * `Ok(PacketHeader)` The received packet header
    /// * `Err(IrcError::CorruptPacket)` if the packet is corrupt (e.g. invalid magic number, crc mismatch)
    /// * `Err(IrcError::ProtocolError)` if an unexpected packet was received
    /// * `Err(IrcError::NotConnected)` if no connection is yet established
    /// * `Err(IrcError::ConnectionClosed)` if the connection was closed by the peer
    /// * `Err(IrcError::IoError)` if the underlying transport layer returned an error
    pub fn receive(&mut self, bufs: &mut [IoSliceMut]) -> IrcResult<PacketBody> {
        let result = self.receive_internal(bufs);
        match result {
            Ok(PacketHeader::Payload(body)) => Ok(body),
            Ok(PacketHeader::CloseConnection) => {
                self.connection_id = 0;
                Err(IrcError::ConnectionClosed)
            },
            Ok(_) => Err(IrcError::ProtocolError),
            Err(e) => Err(e),
        }
    }

    /// Receiving without any connection handling
    ///
    /// # Returns
    /// * `Ok(PacketHeader)` on success, with the parsed packet
    /// * `Err(IrcError::ProtocolError)` if a packet with an unexpected connection id was received
    /// * `Err(IrcError::CorruptPacket)` if the packet is corrupt (e.g. invalid magic number, crc mismatch)
    /// * `Err(IrcError::IoError)` if the underlying transport layer returned an error
    fn receive_internal(&mut self, bufs: &mut [IoSliceMut]) -> IrcResult<PacketHeader> {
        let first_read = 4;

        if self.synced {
            self.receive_buffer.fill(0);
            self.transport.read_exact(self.receive_buffer[0..first_read].as_mut())?;
        } else {
            // Sync will fill in the first 4 bytes of the receive buffer
            debug_println!("Syncing to MAGIC…");
            self.sync(first_read)?;
            debug_println!("Synced to MAGIC: {:02X}", self.receive_buffer[0]);
        }

        if self.receive_buffer[0] != PACKET_MAGIC {
            debug_println!("Received packet with invalid magic number: {:2X}", self.receive_buffer[0]);
            self.synced = false;
            return Err(IrcError::CorruptPacket);
        }

        if self.receive_buffer[1] != self.connection_id {
            return Err(IrcError::ProtocolError);
        }

        let is_control = (self.receive_buffer[2] >> 7 & 0x01) == 1;
        let is_large   = (self.receive_buffer[2] >> 6 & 0x01) == 1;
        let mut payload_length: u16 = (self.receive_buffer[2] & 0x3F) as u16;

        let next= if is_large {
            payload_length <<= 8;
            payload_length |= (self.receive_buffer[3] & 0xFF) as u16;
            PACKET_HEADER_SIZE_LARGE
        } else {
            PACKET_HEADER_SIZE_SMALL
        };

        if is_control {
            let end = next + payload_length as usize;

            // CRC check
            self.transport.read_exact(self.receive_buffer[first_read..end + 1].as_mut())?;
            let crc = irc_crc(&self.receive_buffer[..end]);
            if crc != self.receive_buffer[end] {
                debug_println!("CRC check failed");
                return Err(IrcError::CorruptPacket);
            }

            match self.receive_buffer[next] {
                PACKET_TYPE_CREATE_CONNECTION => {
                    let source = self.receive_buffer[next + 1].try_into()
                        .map_err(|_| IrcError::CorruptPacket)?;
                    let target = self.receive_buffer[next + 2].try_into()
                        .map_err(|_| IrcError::CorruptPacket)?;
                    let connection_id = self.receive_buffer[next + 3];
                    Ok(PacketHeader::CreateConnection(source, target, connection_id))
                },
                PACKET_TYPE_ACCEPT_CONNECTION => {
                    Ok(PacketHeader::AcceptConnection)
                },
                PACKET_TYPE_CLOSE_CONNECTION => {
                    Ok(PacketHeader::CloseConnection)
                },
                _ => Err(IrcError::CorruptPacket)
            }
        } else {
            self.transport.read_exact(self.receive_buffer[first_read..next + PACKET_HEADER_RECEIVE_SIZE].as_mut())?;

            let mut receive_length_bytes: [u8; 2] = [0; 2];
            receive_length_bytes.copy_from_slice(&self.receive_buffer[next..next + PACKET_HEADER_RECEIVE_SIZE]);

            let mut crc = irc_crc(&self.receive_buffer[..next + PACKET_HEADER_RECEIVE_SIZE]);

            let response_length = u16::from_be_bytes(receive_length_bytes);

            let mut to_read=  payload_length as usize - PACKET_HEADER_RECEIVE_SIZE;
            let total_read = to_read;
            let mut internal_bufs = bufs;
            while to_read > 0 {
                let read = (&mut self.transport).take(to_read as u64).read_vectored(internal_bufs)?;

                if read == 0 {
                    debug_println!("Unexpected EOF (to_read: {to_read}/{total_read})");
                    return Err(IrcError::CorruptPacket);
                }

                let read_slices = Self::current_read_slices(internal_bufs, read);
                crc = irc_crc_ioslice_continue(read_slices.as_slice(), crc);

                IoSliceMut::advance_slices(&mut internal_bufs, read);
                to_read = to_read.saturating_sub(read);
            }

            // CRC check
            let mut expected_crc = [0u8];
            self.transport.read_exact(&mut expected_crc)?;

            if expected_crc[0] != crc {
                debug_println!("CRC check failed");
                return Err(IrcError::CorruptPacket);
            }

            Ok(PacketHeader::Payload(
                PacketBody {
                    response_length,
                    payload_length: payload_length - PACKET_HEADER_RECEIVE_SIZE as u16
                }
            ))
        }
    }

    /// Constructs a temporary list of immutable IoSlice views from mutable IoSlices, with the
    /// given `byte_length`.
    fn current_read_slices<'a>(slices: &'a [IoSliceMut<'a>], mut byte_length: usize) -> Vec<IoSlice<'a>> {
        let mut result = Vec::new();
        for slice in slices {
            if byte_length == 0 {
                break;
            }
            let min_len = std::cmp::min(slice.len(), byte_length);

            let view = &slice[..min_len];
            result.push(IoSlice::new(view));

            byte_length -= min_len;
        }
        result
    }

    fn sync(&mut self, transport_buffer_length: usize) -> IrcResult<()> {
        let marker_length = 2;
        debug_assert!(transport_buffer_length >= marker_length);

        let mut tries = 512;

        let mut receive_copy= vec![0; transport_buffer_length];
        receive_copy.copy_from_slice(&self.receive_buffer[..transport_buffer_length]);
        let mut cursor = Cursor::new(&receive_copy)
            .chain(&mut self.transport);

        let mut buf = [0u8; 1];

        while tries > 0 {
            cursor.read_exact(&mut buf)?;
            if buf[0] == PACKET_MAGIC {
                cursor.read_exact(&mut buf)?;
                if buf[0] == self.connection_id {
                    self.receive_buffer[0..marker_length].copy_from_slice(&[PACKET_MAGIC, self.connection_id]);
                    cursor.read_exact(&mut self.receive_buffer[marker_length..transport_buffer_length])?;
                    return Ok(())
                }
            }
            tries -= 1;
        }
        Err(IrcError::CorruptPacket)
    }

    /// Sends a packet to the IR port
    ///
    /// # Arguments:
    /// * `header`: The packet header to send
    /// * `payload_bufs`: The payload buffers to send (only use if `header` is a `PacketHeader::Payload`)
    ///
    /// # Returns:
    /// * `Ok(())`: The packet was sent successfully
    /// * `Err(IrcError::ConnectionClosed)`: The connection is closed or not opened yet
    /// * `Err(IrcError::IoError)`: The underlying transport encountered an error
    fn send(&mut self, header: PacketHeader, payload_bufs: &mut [IoSlice]) -> IrcResult<()> {
        match (self.connection_id, &header) {
            (0, PacketHeader::CreateConnection(_, _, _)) =>  self.send_internal(header, payload_bufs),
            (0, _) => Err(IrcError::ConnectionClosed),
            (_, _) => self.send_internal(header, payload_bufs)
        }
    }

    /// Sends a packet to the IR port, without
    ///
    /// # Arguments:
    /// * `header`: The packet header to send
    /// * `payload_bufs`: The payload buffers to send (only use if `header` is a `PacketHeader::Payload`)
    fn send_internal(&mut self, header: PacketHeader, payload_bufs: &mut [IoSlice]) -> IrcResult<()> {
        if let PacketHeader::Payload {..} = header {
            debug_assert!(!payload_bufs.is_empty());
        } else {
            debug_assert!(payload_bufs.is_empty());
        }

        let header_size = header.header_length();
        let mut header_buf = vec![0u8; header_size + PACKET_TRAILER_SIZE];
        self.fill_header_base(&mut header_buf, &header);

        let mut crc = irc_crc(&header_buf[..header_size]);
        crc = irc_crc_ioslice_continue(payload_bufs, crc);
        header_buf[header_size] = crc;

        let header_slice = IoSlice::new(&header_buf[..header_size]);
        let trailer_slice = IoSlice::new(&header_buf[header_size..]);

        let mut slices = Vec::with_capacity(2 + payload_bufs.len());
        slices.push(header_slice);
        slices.extend(payload_bufs.iter());
        slices.push(trailer_slice);

        self.transport.write_all_vectored(&mut slices)?;
        Ok(())
    }

    /// Waits for a connection
    ///
    /// # Returns
    /// * `Ok(())` if connection was established
    /// * `Err(IrcError::ProtocolError)` if an unexpected packet was received
    /// * `Err(IrcError::NoConnectionFound)` if no connection was found
    /// * Other errors if applicable
    pub fn wait_connection(&mut self) -> IrcResult<()> {
        if self.connection_id > 0 {
            return Err(IrcError::AlreadyConnected)
        }

        for _ in 0..RETRY_COUNT {
            match self.receive_internal(&mut []) {
                Ok(PacketHeader::CreateConnection(_, _, connection_id)) => {
                    self.connection_id = connection_id;
                    self.send_internal(PacketHeader::AcceptConnection, &mut [])?;
                    debug_println!("Connection accepted: {connection_id}");
                    return Ok(())
                },
                Ok(_) => return Err(IrcError::ProtocolError),
                Err(IrcError::CorruptPacket) => continue,
                // Timeouts are expected, so we keep on trying
                Err(IrcError::Timeout) => continue,
                Err(error) => return Err(error)
            }
        }

        Err(IrcError::NoConnectionFound)
    }

    pub fn disconnect(&mut self) -> IrcResult<()> {
        if self.connection_id == 0 {
            return Err(IrcError::ConnectionClosed);
        }
        self.send_internal(PacketHeader::CloseConnection, &mut [])?;
        self.connection_id = 0;
        debug_println!("Disconnected");
        Ok(())
    }

    /// Sends data to the IR port.
    ///
    /// # Params
    /// * payload: The payload to send
    /// * response_length: The length of the response to expect
    pub fn send_payload<'a>(&mut self, payload: &'a mut [IoSlice<'a>], response_length: u16) -> IrcResult<()> {
        let payload_length = payload.iter().map(|e| e.len()).sum::<usize>() as u16;

        let irc_response_length = response_length
            + PACKET_TRAILER_SIZE as u16
            + PACKET_HEADER_RECEIVE_SIZE as u16
            + if response_length > PAYLOAD_THRESHOLD as u16 { PACKET_HEADER_SIZE_LARGE } else { PACKET_HEADER_SIZE_SMALL } as u16;

        let header = PacketHeader::Payload (PacketBody { response_length: irc_response_length, payload_length });
        self.send(header, payload)
    }
}


#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    fn test_send_packet(packet: PacketHeader, payload: &mut [IoSlice], connection_id: u8, expected_bytes: &[u8]) {
        let mut packet_bytes = Vec::with_capacity(expected_bytes.len());

        {
            let c = Cursor::new(&mut packet_bytes);
            let mut irc = Irc::new(c);
            irc.connection_id = connection_id;
            irc.send(packet, payload).unwrap();
        }

        assert_eq!(packet_bytes.as_slice(), expected_bytes);
    }

    fn receive_packet(packet_bytes: &mut [u8], connection_id: u8, internal: bool) -> Result<(PacketHeader, Box<[u8]>), IrcError> {
        let mut payload = [0u8; 0x200];

        let packet = {
            let mut payload_ioslice = [IoSliceMut::new(&mut payload)];
            let c = Cursor::new(packet_bytes);

            let mut irc = Irc::new(c);
            irc.connection_id = connection_id;

            if internal {
                irc.receive_internal(&mut payload_ioslice)?
            } else {
                PacketHeader::Payload(irc.receive(&mut payload_ioslice)?)
            }
        };

        if let PacketHeader::Payload(body) = &packet {
            let payload_length = body.payload_length;
            return Ok((packet, Box::from(&payload[..payload_length as usize])));
        }

        Ok((packet, Box::from([])))
    }

    fn test_receive_packet(packet_bytes: &mut [u8], expected_packet: PacketHeader, expected_payload: &[u8], connection_id: u8, ignore_link_state: bool) {
        let (packet, payload) = receive_packet(packet_bytes, connection_id, ignore_link_state).unwrap();

        assert_eq!(packet, expected_packet);
        if let PacketHeader::Payload(body) = packet {
            assert_eq!(&payload[..body.payload_length as usize], expected_payload);
        }
    }

    fn test_receive_error_packet(packet_bytes: &mut [u8], connection_id: u8, ignore_link_state: bool, expected_error: IrcError) {
        let packet = receive_packet(packet_bytes, connection_id, ignore_link_state);
        assert!(packet.is_err());
        assert_eq!(packet.err().unwrap(), expected_error);
    }

    #[test]
    fn send_connect_packet() {
        let packet = PacketHeader::CreateConnection(Target::WiiU, Target::FitMeter, 0xF2);
        let expected_bytes= [0xA5u8, 0x0, 0x84, 0x1, 0x3, 0x4, 0xf2, 0xb6];
        test_send_packet(packet, &mut [], CONNECTION_ID_ANY, &expected_bytes);
    }

    #[test]
    fn send_small_payload_packet() {
        let mut payload = [IoSlice::new(&[0x0F1, 0x59, 0x05, 0xB4, 0x1A, 0xBD, 0x58, 0xF3, 0xCF, 0xAA])];
        let packet = PacketHeader::Payload(PacketBody { response_length: 0x0010, payload_length: payload[0].len() as u16 });
        let expected_bytes= [0xA5, 0xF2, 0x0C, 0x00, 0x10, 0xF1, 0x59, 0x5, 0xB4, 0x1A, 0xBD, 0x58, 0xF3, 0xCF, 0xAA, 0x70];
        test_send_packet(packet, &mut payload, 0xF2, &expected_bytes);
    }

    #[test]
    fn receive_connect_packet() {
        let packet_bytes = &mut [0xA5u8, 0x00, 0x84, 0x01, 0x03, 0x04, 0xEA, 0xFE];
        let header = PacketHeader::CreateConnection(Target::WiiU, Target::FitMeter, 0xEA);
        test_receive_packet(packet_bytes, header, &[], 0x00, true);
    }

    #[test]
    fn receive_corrupt_connect_packet() {
        let packet_bytes = &mut [0xA5u8, 0x00, 0x84, 0x01, 0x03, 0x04, 0xEA, 0xFF];
        test_receive_error_packet(packet_bytes, 0x00, false, IrcError::CorruptPacket);
    }

    #[test]
    fn receive_payload_packet_small() {
        let packet_bytes = &mut [0xA5, 0x87, 0x03, 0x00, 0x00, 0xF3, 0x09];
        let header = PacketHeader::Payload(PacketBody { response_length: 0x0000, payload_length: 0x0001 });
        test_receive_packet(packet_bytes, header, &[0xF3], 0x87, false);
    }

    #[test]
    fn receive_corrupt_payload_packet_small() {
        let packet_bytes = &mut [0xA5, 0x87, 0x03, 0x00, 0x00, 0xF3, 0x0A];
        test_receive_error_packet(packet_bytes, 0x87, false, IrcError::CorruptPacket);
    }

    #[test]
    fn receive_connection_closed() {
        let packet_bytes = &mut [0xA5, 0xB1, 0x81, 0x0F, 0xBE];
        test_receive_error_packet(packet_bytes, 0xB1, false, IrcError::ConnectionClosed);
    }


    #[test]
    fn receive_payload_packet_large() {
        let payload_bytes = [ 0xF0, 0xA9, 0xAE, 0xAA, 0xAE, 0xA2, 0xAA, 0xAB, 0xA9, 0xA8, 0xA9, 0xAE, 0xAA, 0xAB, 0xAB, 0xA3, 0xAC, 0xAC, 0xAB, 0xAA, 0xAE, 0xAB, 0xAB, 0xA9, 0xBA, 0xAA, 0xAB, 0xAE, 0xAE, 0xAA, 0xAB, 0xA2, 0xA8, 0xAA, 0xA9, 0xAF, 0xAA, 0xAE, 0xA8, 0xAA, 0xA9, 0xAB, 0xAF, 0xAD, 0xAE, 0xA8, 0xAA, 0xA9, 0xAF, 0xAA, 0xAB, 0xAD, 0xAB, 0xAD, 0xAE, 0xAA, 0xA8, 0xAF, 0xAC, 0xA9, 0xA9, 0xAE, 0xAE, 0xAE, 0xAE, 0xAA, 0xA8, 0xA8, 0xA8, 0xAA, 0xAB, 0xAE, 0xAD, 0xA1, 0xA9, 0xAD, 0xAA, 0xA9, 0xA1, 0xA1, 0xA0, 0xAD, 0xBC, 0xA4, 0xBD, 0xBC, 0xBC, 0xBC, 0xA4, 0xB9, 0xA3, 0xAE, 0xAE, 0xA8, 0xAE, 0xAE, 0xA9, 0xAE, 0xAE, 0xA9, 0xAE, 0xAE, 0xA9, 0xA2, 0xA0, 0xAB, 0xAA, 0xBF, 0xB9, 0xA8, 0xA9, 0xAB, 0xA8, 0xA9, 0xA8, 0xA7, 0xB8, 0xAF, 0xAB, 0xA8, 0xA8, 0xA8, 0xAB, 0xAA, 0xA8, 0xA0, 0xAB, 0xAA, 0xAB, 0xA8, 0xA3, 0xA9, 0xA9, 0xAB, 0xA9, 0xA8, 0xA9, 0xAC, 0xA6, 0xB2, 0xA6, 0xA8, 0xA8, 0xAD, 0xA3, 0xBA, 0xB1, 0xA6, 0xB6, 0xB5, 0xB7, 0xB2, 0xA5, 0xAD, 0xBB, 0xA7, 0xAF, 0xA9, 0xAA, 0xA8, 0xA5, 0xAA, 0xA8, 0xAC, 0xAC, 0xAA, 0xAE, 0xBC, 0xAA, 0xA9, 0xA2, 0xAA, 0xAF, 0xBE, 0xAA, 0xA8, 0xA9, 0xAA, 0xAD, 0xA1, 0xAA, 0xAF, 0xA0, 0xAA, 0xA3, 0xBB, 0xA8, 0xAB, 0xAA, 0xAF, 0xAC, 0xA3, 0xAA, 0xAC, 0xAE, 0xAA, 0xAB, 0xAF, 0xAC, 0xAA, 0xAB, 0xAB, 0xA8, 0xAA, 0xAF, 0xA8, 0xAA, 0xA8, 0xA8, 0xAE, 0xAC, 0xAA, 0xAD, 0xAD, 0xAA, 0xAF, 0xA8, 0xAA, 0xA8, 0xA9, 0xAA, 0xAB, 0xA9, 0xAA, 0xAC, 0xA4, 0xAA, 0xA9, 0xAB, 0xAA, 0xAB, 0xA9, 0xAE, 0xAA, 0xAF, 0xAF, 0xAA, 0xAE, 0xAD, 0xAA, 0xAB, 0xA8, 0xAA, 0xAD, 0xA0, 0xAA, 0xAD, 0xAB, 0xAE, 0xAC, 0xA0, 0xAA, 0xAD, 0xA8, 0xAA, 0xAD, 0xAB, 0xAA, 0xA8, 0xAB, 0xA2, 0xB8, 0xAA, 0xAB, 0xA9, 0xAB, 0xAA, 0xA8, 0xA7, 0xA0, 0xAA, 0xAD, 0xA2, 0xAA, 0xAE, 0xAE, 0xAA, 0xA8, 0xA8, 0xA9, 0xA2, 0xAC, 0xA6, 0xA8, 0xAE, 0xAA, 0xA8, 0xAC, 0xAB, 0xAA, 0xA9, 0xAE, 0xAA, 0xAF, 0xA8, 0xAA, 0xAB, 0xA9, 0xA8, 0xAA, 0xAE, 0xA8, 0xAA, 0xA2, 0xA9, 0xAA, 0xA2, 0xAB, 0xAA, 0xAF, 0xA0, 0xAA, 0xAB, 0xAE, 0xAB, 0xAE, 0xAB, 0xAA, 0xAE, 0xAB, 0xAA, 0xAD, 0xA2, 0xAA, 0xA8, 0xA9, 0xB3, 0xA5, 0xA5, 0xA0, 0xAC, 0xAB, 0xAA, 0xA9, 0xAE, 0xAA, 0xA8, 0xA3, 0xAA, 0xAF, 0xA5, 0xAD, 0xAE, 0xAF, 0xAD, 0xAD, 0xB8, 0xA4, 0xBA, 0xA1, 0xAA, 0xAB, 0xAB, 0xA1, 0xAC, 0xAA, 0xAB, 0xAB, 0xAA, 0xA8, 0xAF, 0xAA, 0xA8, 0xA8, 0xAA, 0xAB, 0xA4, 0xA0, 0xAA, 0xA9, 0xA9, 0xA8, 0xAA, 0xAB, 0xAB, 0xA2, 0xAB, 0xAA, 0xA9, 0xA8, 0xA8, 0xAA, 0xAB, 0xA2, 0xA8, 0xAA, 0xA9, 0xAE, 0xAA, 0xA3, 0xA9, 0xAA, 0xAC, 0xAB, 0xAA, 0xAF, 0xAC, 0xAA, 0xA8, 0xA8, 0xAD, 0xA9, 0xAA, 0xA8, 0xA9, 0xA1, 0xA5, 0xA2, 0x8E, 0xB4, 0xA6, 0xA5, 0xA9, 0xA8, 0xAB, 0xAE, 0xA8, 0xAE, 0xA3, 0xA9, 0xA8, 0xAA, 0xAB, 0xA9, 0xAF, 0xAE, 0xA8, 0xAA, 0xAB, 0xBB, 0xA1, 0xAD, 0xA9, 0xAC, 0xAE, 0xA2, 0xAF, 0xBB, 0xAA, 0xAF, 0xA0, 0xA2, 0xBF, 0xA7, 0xB2, 0xB0, 0xB3, 0xB3, 0xB1, 0xB6, 0x8A, 0xBC, 0xB1, 0xB7, 0xB2, 0xB9, 0xAC, 0xAC, 0xAF, 0xA9, 0xAF, 0xAF, 0xAE, 0xA6, 0xA8, 0xA8, 0xAA, 0xA1, 0xB5, 0x80, 0xBA, 0xA8, 0xBE, 0xA4, 0xA6, 0xA1, 0xAF, 0xA9, 0xAA, 0xAB, 0xA8, 0xAF, 0xAA, 0xAE, 0x79, 0xAA, 0x53, 0xAA, 0xA2, 0xB2, 0xAA, 0x86, 0xAA, 0xA8, 0xAD, 0xAA, 0xA9, 0x85, 0xAA, 0xAB, 0xA9, 0xA9, 0xAA, 0xAF, 0xAC, 0xA6, 0xAA, 0xAF, 0xA5, 0xAA ];
        let packet_bytes = &mut [0xA5, 0x87, 0x42, 0x00, 0x05,  0xF4, 0xF0, 0xA9, 0xAE, 0xAA, 0xAE, 0xA2, 0xAA, 0xAB, 0xA9, 0xA8, 0xA9, 0xAE, 0xAA, 0xAB, 0xAB, 0xA3, 0xAC, 0xAC, 0xAB, 0xAA, 0xAE, 0xAB, 0xAB, 0xA9, 0xBA, 0xAA, 0xAB, 0xAE, 0xAE, 0xAA, 0xAB, 0xA2, 0xA8, 0xAA, 0xA9, 0xAF, 0xAA, 0xAE, 0xA8, 0xAA, 0xA9, 0xAB, 0xAF, 0xAD, 0xAE, 0xA8, 0xAA, 0xA9, 0xAF, 0xAA, 0xAB, 0xAD, 0xAB, 0xAD, 0xAE, 0xAA, 0xA8, 0xAF, 0xAC, 0xA9, 0xA9, 0xAE, 0xAE, 0xAE, 0xAE, 0xAA, 0xA8, 0xA8, 0xA8, 0xAA, 0xAB, 0xAE, 0xAD, 0xA1, 0xA9, 0xAD, 0xAA, 0xA9, 0xA1, 0xA1, 0xA0, 0xAD, 0xBC, 0xA4, 0xBD, 0xBC, 0xBC, 0xBC, 0xA4, 0xB9, 0xA3, 0xAE, 0xAE, 0xA8, 0xAE, 0xAE, 0xA9, 0xAE, 0xAE, 0xA9, 0xAE, 0xAE, 0xA9, 0xA2, 0xA0, 0xAB, 0xAA, 0xBF, 0xB9, 0xA8, 0xA9, 0xAB, 0xA8, 0xA9, 0xA8, 0xA7, 0xB8, 0xAF, 0xAB, 0xA8, 0xA8, 0xA8, 0xAB, 0xAA, 0xA8, 0xA0, 0xAB, 0xAA, 0xAB, 0xA8, 0xA3, 0xA9, 0xA9, 0xAB, 0xA9, 0xA8, 0xA9, 0xAC, 0xA6, 0xB2, 0xA6, 0xA8, 0xA8, 0xAD, 0xA3, 0xBA, 0xB1, 0xA6, 0xB6, 0xB5, 0xB7, 0xB2, 0xA5, 0xAD, 0xBB, 0xA7, 0xAF, 0xA9, 0xAA, 0xA8, 0xA5, 0xAA, 0xA8, 0xAC, 0xAC, 0xAA, 0xAE, 0xBC, 0xAA, 0xA9, 0xA2, 0xAA, 0xAF, 0xBE, 0xAA, 0xA8, 0xA9, 0xAA, 0xAD, 0xA1, 0xAA, 0xAF, 0xA0, 0xAA, 0xA3, 0xBB, 0xA8, 0xAB, 0xAA, 0xAF, 0xAC, 0xA3, 0xAA, 0xAC, 0xAE, 0xAA, 0xAB, 0xAF, 0xAC, 0xAA, 0xAB, 0xAB, 0xA8, 0xAA, 0xAF, 0xA8, 0xAA, 0xA8, 0xA8, 0xAE, 0xAC, 0xAA, 0xAD, 0xAD, 0xAA, 0xAF, 0xA8, 0xAA, 0xA8, 0xA9, 0xAA, 0xAB, 0xA9, 0xAA, 0xAC, 0xA4, 0xAA, 0xA9, 0xAB, 0xAA, 0xAB, 0xA9, 0xAE, 0xAA, 0xAF, 0xAF, 0xAA, 0xAE, 0xAD, 0xAA, 0xAB, 0xA8, 0xAA, 0xAD, 0xA0, 0xAA, 0xAD, 0xAB, 0xAE, 0xAC, 0xA0, 0xAA, 0xAD, 0xA8, 0xAA, 0xAD, 0xAB, 0xAA, 0xA8, 0xAB, 0xA2, 0xB8, 0xAA, 0xAB, 0xA9, 0xAB, 0xAA, 0xA8, 0xA7, 0xA0, 0xAA, 0xAD, 0xA2, 0xAA, 0xAE, 0xAE, 0xAA, 0xA8, 0xA8, 0xA9, 0xA2, 0xAC, 0xA6, 0xA8, 0xAE, 0xAA, 0xA8, 0xAC, 0xAB, 0xAA, 0xA9, 0xAE, 0xAA, 0xAF, 0xA8, 0xAA, 0xAB, 0xA9, 0xA8, 0xAA, 0xAE, 0xA8, 0xAA, 0xA2, 0xA9, 0xAA, 0xA2, 0xAB, 0xAA, 0xAF, 0xA0, 0xAA, 0xAB, 0xAE, 0xAB, 0xAE, 0xAB, 0xAA, 0xAE, 0xAB, 0xAA, 0xAD, 0xA2, 0xAA, 0xA8, 0xA9, 0xB3, 0xA5, 0xA5, 0xA0, 0xAC, 0xAB, 0xAA, 0xA9, 0xAE, 0xAA, 0xA8, 0xA3, 0xAA, 0xAF, 0xA5, 0xAD, 0xAE, 0xAF, 0xAD, 0xAD, 0xB8, 0xA4, 0xBA, 0xA1, 0xAA, 0xAB, 0xAB, 0xA1, 0xAC, 0xAA, 0xAB, 0xAB, 0xAA, 0xA8, 0xAF, 0xAA, 0xA8, 0xA8, 0xAA, 0xAB, 0xA4, 0xA0, 0xAA, 0xA9, 0xA9, 0xA8, 0xAA, 0xAB, 0xAB, 0xA2, 0xAB, 0xAA, 0xA9, 0xA8, 0xA8, 0xAA, 0xAB, 0xA2, 0xA8, 0xAA, 0xA9, 0xAE, 0xAA, 0xA3, 0xA9, 0xAA, 0xAC, 0xAB, 0xAA, 0xAF, 0xAC, 0xAA, 0xA8, 0xA8, 0xAD, 0xA9, 0xAA, 0xA8, 0xA9, 0xA1, 0xA5, 0xA2, 0x8E, 0xB4, 0xA6, 0xA5, 0xA9, 0xA8, 0xAB, 0xAE, 0xA8, 0xAE, 0xA3, 0xA9, 0xA8, 0xAA, 0xAB, 0xA9, 0xAF, 0xAE, 0xA8, 0xAA, 0xAB, 0xBB, 0xA1, 0xAD, 0xA9, 0xAC, 0xAE, 0xA2, 0xAF, 0xBB, 0xAA, 0xAF, 0xA0, 0xA2, 0xBF, 0xA7, 0xB2, 0xB0, 0xB3, 0xB3, 0xB1, 0xB6, 0x8A, 0xBC, 0xB1, 0xB7, 0xB2, 0xB9, 0xAC, 0xAC, 0xAF, 0xA9, 0xAF, 0xAF, 0xAE, 0xA6, 0xA8, 0xA8, 0xAA, 0xA1, 0xB5, 0x80, 0xBA, 0xA8, 0xBE, 0xA4, 0xA6, 0xA1, 0xAF, 0xA9, 0xAA, 0xAB, 0xA8, 0xAF, 0xAA, 0xAE, 0x79, 0xAA, 0x53, 0xAA, 0xA2, 0xB2, 0xAA, 0x86, 0xAA, 0xA8, 0xAD, 0xAA, 0xA9, 0x85, 0xAA, 0xAB, 0xA9, 0xA9, 0xAA, 0xAF, 0xAC, 0xA6, 0xAA, 0xAF, 0xA5, 0xAA, 0x8A ];
        let header = PacketHeader::Payload(PacketBody { response_length: 0x5F4, payload_length: payload_bytes.len() as u16 });

        test_receive_packet(packet_bytes, header, &payload_bytes, 0x87, false)
    }
}
