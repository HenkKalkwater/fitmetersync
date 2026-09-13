use std::error::Error;
use std::fmt::{Display, Formatter};
use std::io::ErrorKind;

#[derive(Debug)]
pub enum IrcError {
    ConnectionClosed,
    AlreadyConnected,
    NoConnectionFound,
    CorruptFrame,
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
            (IrcError::CorruptFrame, IrcError::CorruptFrame) => true,
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
            IrcError::CorruptFrame => write!(f, "Corrupt frame"),
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

#[derive(Debug)]
pub enum IrcuError {
    ConnectionClosed,
    AlreadyConnected,
    NoConnectionFound,
    ProtocolError,
    Timeout,
    BufferOverflow,
    TransportError(std::io::Error)
}

impl Display for IrcuError {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            IrcuError::ConnectionClosed => write!(f, "Connection closed"),
            IrcuError::TransportError(err) => write!(f, "Transport error: {}", err),
            IrcuError::AlreadyConnected => write!(f, "Already connected"),
            IrcuError::NoConnectionFound => write!(f, "No connection found"),
            IrcuError::ProtocolError => write!(f, "Protocol error"),
            IrcuError::BufferOverflow => write!(f, "Buffer overflow"),
            IrcuError::Timeout => write!(f, "Timeout"),
        }
    }
}

impl Error for IrcuError {}

impl From<IrcError> for IrcuError {
    fn from(error: IrcError) -> IrcuError {
        match error {
            IrcError::ConnectionClosed => IrcuError::ConnectionClosed,
            IrcError::AlreadyConnected => IrcuError::AlreadyConnected,
            IrcError::NoConnectionFound => IrcuError::NoConnectionFound,
            IrcError::ProtocolError => IrcuError::ProtocolError,
            IrcError::CorruptFrame => IrcuError::ProtocolError,
            IrcError::Timeout => IrcuError::Timeout,
            IrcError::BufferOverflow => IrcuError::BufferOverflow,
            IrcError::TransportError(e) => IrcuError::TransportError(e)
        }
    }
}

impl From<IrcuError> for IrcError {
    fn from(error: IrcuError) -> IrcError {
        match error {
            IrcuError::ConnectionClosed => IrcError::ConnectionClosed,
            IrcuError::AlreadyConnected => IrcError::AlreadyConnected,
            IrcuError::NoConnectionFound => IrcError::NoConnectionFound,
            IrcuError::ProtocolError => IrcError::ProtocolError,
            IrcuError::Timeout => IrcError::Timeout,
            IrcuError::BufferOverflow => IrcError::BufferOverflow,
            IrcuError::TransportError(e) => IrcError::TransportError(e)
        }
    }
}
pub type IrcuResult<T> = Result<T, IrcuError>;
