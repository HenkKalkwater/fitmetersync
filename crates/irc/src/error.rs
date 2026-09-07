use std::error::Error;
use std::fmt::{Display, Formatter};
use std::io::ErrorKind;

#[derive(Debug)]
pub enum IrcError {
    ConnectionClosed,
    AlreadyConnected,
    NoConnectionFound,
    CorruptPacket,
    ProtocolError,
    Timeout,
    BufferOverflow,
    TransportError(std::io::Error),
}

pub type IrcResult<T> = Result<T, IrcError>;

impl PartialEq for IrcError {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (IrcError::ConnectionClosed, IrcError::ConnectionClosed) => true,
            (IrcError::AlreadyConnected, IrcError::AlreadyConnected) => true,
            (IrcError::NoConnectionFound, IrcError::NoConnectionFound) => true,
            (IrcError::CorruptPacket, IrcError::CorruptPacket) => true,
            (IrcError::ProtocolError, IrcError::ProtocolError) => true,
            (IrcError::Timeout, IrcError::Timeout) => true,
            (IrcError::BufferOverflow, IrcError::BufferOverflow) => true,
            (IrcError::TransportError(_), IrcError::TransportError(_)) => true,
            _ => false,
        }
    }
}

impl Display for IrcError {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            IrcError::ConnectionClosed => write!(f, "Connection closed"),
            IrcError::AlreadyConnected => write!(f, "Already connected"),
            IrcError::NoConnectionFound => write!(f, "No connection found"),
            IrcError::CorruptPacket => write!(f, "Corrupt packet"),
            IrcError::ProtocolError => write!(f, "Protocol error"),
            IrcError::Timeout => write!(f, "Connection timeout"),
            IrcError::BufferOverflow => write!(f, "Buffer overflow"),
            IrcError::TransportError(e) => write!(f, "I/O error: {}", e),
        }
    }
}

impl Error for IrcError {}

impl From<std::io::Error> for IrcError {
    fn from(err: std::io::Error) -> IrcError {
        match err.kind() {
            ErrorKind::TimedOut => IrcError::Timeout,
            _ => IrcError::TransportError(err),
        }
    }
}