use std::cmp::min;
use std::io::{IoSlice, IoSliceMut, Read, Write};
use crate::error::{IrcError, IrcResult, IrcuError, IrcuResult};
use crate::irc::Irc;


struct ConnectionId(u8);
const ANY_CONNECTION: ConnectionId = ConnectionId(0);
const MAX_PAYLOAD_LENGTH: u16 = 255;

#[repr(u8)]
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
enum CommandMode {
    Receive  = 0,
    Transmit = 0x80
}

impl From<CommandMode> for u8 {
    fn from(value: CommandMode) -> u8 {
        value as u8
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
struct Command {
    mode: CommandMode,
    command: u8,
    address: u16
}

const MAX_HEADER_LENGTH: usize = 4;
const RETRIES: usize = 3;

#[repr(u8)]
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
enum IrcuPacketType {
    /// Ack: send more data
    WaitAck      = 0xF0,
    /// Ack: sending last data
    WaitAckFinal = 0xF1,
    /// Ack: more data to follow
    Ack          = 0xF2,
    /// Final ack: all data is received / ready for next command
    AckFinal     = 0xF3,
    /// Request data transmission
    Send         = 0xF4,
    /// Request retransmission of the last packet
    Retransmit   = 0xF5,
}

impl From<IrcuPacketType> for u8 {
    fn from(value: IrcuPacketType) -> Self {
        value as u8
    }
}

impl From<IrcuPacket> for IrcuPacketType {
    fn from(value: IrcuPacket) -> Self {
        match value {
            IrcuPacket::WaitAck      => IrcuPacketType::WaitAck,
            IrcuPacket::WaitAckFinal => IrcuPacketType::WaitAckFinal,
            IrcuPacket::Ack          => IrcuPacketType::Ack,
            IrcuPacket::AckFinal     => IrcuPacketType::AckFinal,
            IrcuPacket::Send { .. }  => IrcuPacketType::Send,
            IrcuPacket::Retransmit   => IrcuPacketType::Retransmit,
        }
    }
}

impl TryFrom<u8> for IrcuPacketType {
    type Error = ();

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            0xF0 => Ok(IrcuPacketType::WaitAck),
            0xF1 => Ok(IrcuPacketType::WaitAckFinal),
            0xF2 => Ok(IrcuPacketType::Ack),
            0xF3 => Ok(IrcuPacketType::AckFinal),
            0xF4 => Ok(IrcuPacketType::Send),
            0xF5 => Ok(IrcuPacketType::Retransmit),
            _ => Err(())
        }
    }
}

impl IrcuPacketType {
    const fn header_length(&self) -> usize {
        match self {
            IrcuPacketType::WaitAck      => 1,
            IrcuPacketType::WaitAckFinal => 1,
            IrcuPacketType::Ack          => 1,
            IrcuPacketType::AckFinal     => 1,
            IrcuPacketType::Send         => 4,
            IrcuPacketType::Retransmit   => 1,
        }
    }

    fn expected_replies(&self) -> &'static[IrcuPacketType] {
        match self {
            IrcuPacketType::WaitAck => &[IrcuPacketType::Retransmit, IrcuPacketType::Ack],
            IrcuPacketType::WaitAckFinal => &[IrcuPacketType::Retransmit, IrcuPacketType::AckFinal],
            IrcuPacketType::Ack => &[IrcuPacketType::Retransmit, IrcuPacketType::WaitAck, IrcuPacketType::WaitAckFinal],
            IrcuPacketType::AckFinal => &[IrcuPacketType::AckFinal, IrcuPacketType::Retransmit, IrcuPacketType::Send],
            IrcuPacketType::Send => &[IrcuPacketType::Retransmit, IrcuPacketType::WaitAck, IrcuPacketType::WaitAckFinal],
            IrcuPacketType::Retransmit => &[IrcuPacketType::WaitAck, IrcuPacketType::WaitAckFinal,
                                            IrcuPacketType::Ack, IrcuPacketType::WaitAck, IrcuPacketType::Send, IrcuPacketType::Retransmit],
        }
    }
}

#[repr(u8)]
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
enum IrcuPacket {
    /// Ack: send last data
    WaitAckFinal  = 0xF0,
    /// Ack: sending more data
    WaitAck       = 0xF1,
    /// Ack: more data to follow
    Ack           = 0xF2,
    /// Final ack: all data is received / ready for next command
    AckFinal      = 0xF3,
    /// Request data transmission
    Send(Command) = 0xF4,
    /// Request retransmission of the last packet
    Retransmit    = 0xF5,
}

impl IrcuPacket {
    fn from_buf(buf: &[u8]) -> IrcuResult<IrcuPacket> {
        if buf.len() < 1 {
            return Err(IrcuError::ProtocolError)
        }
        let packet_type = buf[0];
        if let Some(header_length) = Self::header_length_by_type(packet_type) {
            if buf.len() < header_length {
                return Err(IrcuError::BufferOverflow)
            }
            match packet_type {
                0xF0 => Ok(IrcuPacket::WaitAck),
                0xF1 => Ok(IrcuPacket::WaitAckFinal),
                0xF2 => Ok(IrcuPacket::Ack),
                0xF3 => Ok(IrcuPacket::AckFinal),
                0xF4 => {

                    let mode = match buf[1] >> 7 {
                        0 => CommandMode::Receive,
                        1 => CommandMode::Transmit,
                        _ => return Err(IrcuError::ProtocolError)
                    };
                    let command = buf[1] & 0x7F;
                    let address = u16::from_be_bytes([buf[2], buf[3]]);

                    Ok(IrcuPacket::Send(Command { mode, command, address }))
                },
                0xF5 => Ok(IrcuPacket::Retransmit),
                _ => Err(IrcuError::ProtocolError),
            }
        } else {
            Err(IrcuError::ProtocolError)
        }

    }

    fn header_length_by_type(message_type: u8) -> Option<usize> {
        match message_type {
            0xF0 /* WaitAck      */ => Some(1),
            0xF1 /* WaitAckFinal */ => Some(1),
            0xF2 /* Ack          */ => Some(1),
            0xF3 /* AckFinal     */ => Some(1),
            0xF4 /* Send         */ => Some(4),
            0xF5 /* Retransmit   */ => Some(1),
            _ => None,
        }
    }

    fn header_length(&self) -> usize {
        Self::header_length_by_type((*self).into()).unwrap()
    }

    fn to_buf(&self, buf: &mut[u8]) {
        debug_assert!(buf.len() >= self.header_length());
        buf[0] = (*self).into();
        match self {
            IrcuPacket::Send(command) => {
                buf[1] = command.mode.into();
                buf[1] |= command.command & 0x7F;
                buf[2..4].copy_from_slice(&command.address.to_be_bytes());
            }
            _ => {}
        }
    }

    pub fn packet_type(&self) -> IrcuPacketType {
        (*self).into()
    }

}

impl From<IrcuPacket> for u8 {
    fn from(value: IrcuPacket) -> Self {
        match value {
            IrcuPacket::WaitAckFinal => 0xF0,
            IrcuPacket::WaitAck      => 0xF1,
            IrcuPacket::Ack          => 0xF2,
            IrcuPacket::AckFinal     => 0xF3,
            IrcuPacket::Send { .. }  => 0xF4,
            IrcuPacket::Retransmit   => 0xF5,
        }
    }
}

struct IrcuCommon<Transport: Read + Write> {
    connection_id: ConnectionId,
    connected: bool,
    irc: Irc<Transport>
}

impl<Transport: Read + Write> IrcuCommon<Transport> {
    pub fn new(transport: Transport) -> Self {
        IrcuCommon {
            connection_id: ANY_CONNECTION,
            connected: false,
            irc: Irc::new(transport)
        }
    }

    fn receive_packet(&mut self, payload_bufs: &mut [IoSliceMut], expected_replies: &[IrcuPacketType]) -> IrcResult<(IrcuPacket, usize)> {
        let mut header = [0u8; MAX_HEADER_LENGTH];
        let header_slice = IoSliceMut::new(&mut header[0..1]);
        let mut slices = Vec::with_capacity(1 + payload_bufs.len());
        slices.push(header_slice);
        slices.extend(payload_bufs.iter_mut().map(|b| IoSliceMut::new(&mut **b)));

        let irc_packet = match self.irc.receive(&mut slices) {
            Err(IrcError::BufferOverflow) => Err(IrcError::ProtocolError),
            result => result,
        }?;
        let ircu_packet = IrcuPacket::from_buf(&header)?;

        if !expected_replies.contains(&ircu_packet.into()) {
            debug_println!("Received packet {:?} does not match expected replies {:?}", ircu_packet, expected_replies);
            return Err(IrcError::ProtocolError);
        }

        Ok((ircu_packet, irc_packet.payload_length as usize - ircu_packet.header_length()))
    }

    fn send_packet(&mut self, header: IrcuPacket, payload: &mut [IoSlice], response_length: u16) -> IrcuResult<()> {
        let mut header_buf = [0u8; MAX_HEADER_LENGTH];
        header.to_buf(&mut header_buf);

        let mut io_slices = Vec::with_capacity(1 + payload.len());
        io_slices.push(IoSlice::new(&header_buf[..header.header_length()]));
        io_slices.extend(payload.iter());
        self.irc.send_payload(&mut io_slices, response_length + 1)?;

        Ok(())
    }

    pub fn send_ack(last: bool) {
        todo!()
    }
}

pub struct IrcuMaster<Transport: Read + Write> {
    common: IrcuCommon<Transport>,
}

impl<Transport: Read + Write> IrcuMaster<Transport> {
    pub fn new(transport: Transport) -> Self {
        IrcuMaster {
            common: IrcuCommon::new(transport)
        }
    }

    pub fn connect(&mut self) -> IrcuResult<()> {
        self.common.irc.wait_connection()?;
        debug_println!("IRC connected");
        self.common.receive_packet(&mut [], &IrcuPacketType::AckFinal.expected_replies())?;
        debug_println!("IRCU connected");
        Ok(())
    }

    pub fn disconnect(&mut self) -> IrcuResult<()> {
        self.common.irc.disconnect()?;
        Ok(())
    }

    pub fn receive(&mut self, mut payload_bufs: &mut [IoSliceMut], command: u8, address: u16, response_size: u16) -> IrcuResult<()> {
        let command_struct = Command { mode: CommandMode::Receive, command, address };

        let mut bytes_left = response_size;
        let mut next_packet = IrcuPacket::Send(command_struct);
        let mut next_response_size = min(bytes_left, MAX_PAYLOAD_LENGTH);
        let mut expected_replies = next_packet.packet_type().expected_replies();

        debug_println!("RX {command:02X} {address:04X}");

        let mut retries = RETRIES;
        let mut error = None;
        loop {
            if let Some(e) = error {
                if retries == 0 {
                    return Err(e)
                }
                retries -= 1;
            }

            next_response_size = min(bytes_left, MAX_PAYLOAD_LENGTH);

            self.common.send_packet(next_packet, &mut [], next_response_size)?;
            let received_packet = self.common.receive_packet(payload_bufs, expected_replies);
            (next_packet, expected_replies, bytes_left, error) = match received_packet {
                Ok((IrcuPacket::WaitAck, size))      => {
                    debug_println!("Received wait ACK");
                    IoSliceMut::advance_slices(&mut payload_bufs, size);
                    (
                        IrcuPacket::Ack,
                        IrcuPacketType::Ack.expected_replies(),
                        bytes_left - size as u16,
                        None
                    )
                },
                Ok((IrcuPacket::WaitAckFinal, size)) => {
                    debug_println!("Received wait final ACK");
                    IoSliceMut::advance_slices(&mut payload_bufs, size);
                    (
                        IrcuPacket::AckFinal,
                        IrcuPacketType::AckFinal.expected_replies(),
                        bytes_left - size as u16,
                        None
                    )
                },
                Ok((IrcuPacket::AckFinal, _))     => {
                    debug_println!("Received final ACK");
                    return Ok(())
                },
                Ok((IrcuPacket::Retransmit, _))   => {
                    debug_println!("Retransmission requested");
                    (
                        next_packet,
                        expected_replies,
                        next_response_size,
                        Some(IrcuError::Timeout)
                    )
                },
                Ok(p)               => {
                    debug_println!("Strange packet: {p:?}");
                    return Err(IrcuError::ProtocolError)
                },
                Err(e @ IrcError::CorruptPacket | e @ IrcError::Timeout) => {
                    debug_println!("Error: {e:?}");
                    (
                        IrcuPacket::Retransmit,
                        expected_replies,
                        next_response_size,
                        Some(e.into())
                    )
                },
                Err(e) => {
                    return Err(e.into())
                }
            }
        }
    }

    pub fn send_data(&self, command_id: u8, address: u16, data: &[u8]) -> Result<(), IrcuError> {
        todo!();
    }

    pub fn send_command(&self, command_id: u8, address: u16) -> Result<(), IrcuError>{
        todo!();
    }

}