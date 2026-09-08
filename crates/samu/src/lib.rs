pub mod date;
pub mod compression;

use std::io::{IoSliceMut, Read, Write};
use fms_irc::error::{IrcResult, IrcuError, IrcuResult};
use fms_irc::ircu::IrcuMaster;
use crate::date::DateTime;

macro_rules! debug_println {
    ($($arg:tt)*) => {
        #[cfg(debug_assertions)]
        {
            // file!() provides the path to the current file (e.g., "src/main.rs")
            println!("[{}:{}] {}", file!(), line!(), format_args!($($arg)*));
        }
    };
}

#[derive(Debug)]
pub struct Identity {
    pub user_id: u32,
    pub fitmeter_id: u32,
    pub flag: u8
}

impl Identity {
    pub fn from_bytes(bytes: &[u8]) -> IrcuResult<Identity> {
        let user_id = u32::from_be_bytes(bytes[0..4].try_into().unwrap());
        let fitmeter_id = u32::from_be_bytes(bytes[4..8].try_into().unwrap());
        let flag = bytes[8];
        Ok(Identity { user_id, fitmeter_id, flag })
    }
}

pub struct FitMeterSync<Transport: Read + Write> {
    con: IrcuMaster<Transport>
}

#[repr(u8)]
#[derive(Debug, Copy, Clone)]
enum ReceiveCommand {
    Identity = 0x00,
    Time = 0x01,
    Steps = 0x05
}

impl ReceiveCommand {
    const fn response_size(self) -> usize {
        match self {
            ReceiveCommand::Identity => 9,
            ReceiveCommand::Time => 6,
            ReceiveCommand::Steps => 120
        }
    }

    const fn address(self) -> u16 {
        match self {
            ReceiveCommand::Identity => 0x0000,
            ReceiveCommand::Time     => 0x0000,
            ReceiveCommand::Steps    => 0x0078
        }
    }
}

impl From<ReceiveCommand> for u8 {
    fn from(cmd: ReceiveCommand) -> Self {
        cmd as u8
    }
}

impl<Transport: Read + Write> FitMeterSync<Transport> {
    pub fn new(transport: Transport) -> Self {
        FitMeterSync {
            con: IrcuMaster::new(transport)
        }
    }

    pub fn connect(&mut self) -> IrcuResult<()> {
        self.con.connect()
    }

    pub fn disconnect(&mut self) -> IrcuResult<()> {
        self.con.disconnect()
    }

    fn receive(&mut self, command: ReceiveCommand, mut recv_buf: &mut [u8]) -> IrcuResult<()> {
        let response_size = command.response_size();
        let mut recv_slice = [IoSliceMut::new(&mut recv_buf)];

        self.con.receive(&mut recv_slice, command.into(), command.address(), response_size as u16)?;

        for x in recv_buf[..response_size].iter_mut() {
            *x ^= 0xAAu8
        }
        debug_println!("Response received: {:?}", &recv_buf[0..response_size]);
        Ok(())
    }

    pub fn get_identity(&mut self) -> IrcuResult<Identity> {
        let mut identity_buf = [0u8; ReceiveCommand::Identity.response_size()];
        self.receive(ReceiveCommand::Identity, &mut identity_buf)?;
        let identity = Identity::from_bytes(&identity_buf)?;
        Ok(identity)
    }

    pub fn get_time(&mut self) -> IrcuResult<DateTime> {
        let mut time_buf = [0u8; ReceiveCommand::Time.response_size()];
        self.receive(ReceiveCommand::Time, &mut time_buf)?;

        DateTime::from_bytes(&time_buf).ok_or(IrcuError::ProtocolError)
    }

    pub fn get_steps(&mut self) -> IrcResult<()> {
        let mut steps_buf = [0u8; ReceiveCommand::Steps.response_size()];
        self.receive(ReceiveCommand::Steps, &mut steps_buf)?;
        Ok(())
    }
}