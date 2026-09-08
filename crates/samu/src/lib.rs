pub mod date;
pub mod compression;

use std::io::{IoSliceMut, Read, Write};
use fms_irc::error::{IrcResult, IrcuError, IrcuResult};
use fms_irc::ircu::IrcuMaster;
use crate::compression::{decompress_altitude, decompress_u16, decompress_u8};
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
    Identity    = 0x00,
    Time        = 0x01,
    Mets        = 0x02,
    Altitude    = 0x03,
    ActivityTag = 0x04,
    Kcals       = 0x05,
    Steps       = 0x06
}

impl ReceiveCommand {
    const fn response_size(self) -> usize {
        match self {
            ReceiveCommand::Identity    => 9,
            ReceiveCommand::Time        => 6,
            ReceiveCommand::Mets        => 0x2760,
            ReceiveCommand::Altitude    => 0x2760,
            ReceiveCommand::ActivityTag => 0x2760,
            ReceiveCommand::Kcals       => 120,
            ReceiveCommand::Steps       => 120
        }
    }

    const fn address(self) -> u16 {
        match self {
            ReceiveCommand::Identity    => 0x0000,
            ReceiveCommand::Time        => 0x0000,
            ReceiveCommand::Mets        => 0x2760,
            ReceiveCommand::Altitude    => 0x2760,
            ReceiveCommand::ActivityTag => 0x2760,
            ReceiveCommand::Kcals       => 0x0078,
            ReceiveCommand::Steps       => 0x0078
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

    /// Connects to the Fit Meter
    pub fn connect(&mut self) -> IrcuResult<()> {
        self.con.connect()
    }

    /// Closes the connection to the Fit Meter
    pub fn disconnect(&mut self) -> IrcuResult<()> {
        self.con.disconnect()
    }

    /// Receives data from the Fit Meter
    ///
    /// # Arguments
    /// * `command`: The command to send to the Fit Meter (determines which data you will receive)
    /// * `recv_buf`: The buffer to receive data into
    /// * `response_size`: The size of the response to expect from the Fit Meter. Must be less than
    ///                     or equal to the length of `recv_buf`. If not set, will use the maximum
    ///                     based on `command`.
    fn receive(&mut self, command: ReceiveCommand, mut recv_buf: &mut [u8], response_size: Option<u16>) -> IrcuResult<()> {
        let response_size = response_size.unwrap_or(command.response_size() as u16);
        debug_assert!(response_size <= recv_buf.len() as u16);
        let mut recv_slice = [IoSliceMut::new(&mut recv_buf)];

        self.con.receive(&mut recv_slice, command.into(), command.address(), response_size)?;

        for x in recv_buf[..response_size as usize].iter_mut() {
            *x ^= 0xAAu8
        }
        // debug_println!("Response received: {:?}", &recv_buf[0..response_size]);
        Ok(())
    }

    pub fn get_identity(&mut self) -> IrcuResult<Identity> {
        let mut identity_buf = [0u8; ReceiveCommand::Identity.response_size()];
        self.receive(ReceiveCommand::Identity, &mut identity_buf, None)?;
        let identity = Identity::from_bytes(&identity_buf)?;
        Ok(identity)
    }

    pub fn get_time(&mut self) -> IrcuResult<DateTime> {
        let mut time_buf = [0u8; ReceiveCommand::Time.response_size()];
        self.receive(ReceiveCommand::Time, &mut time_buf, None)?;

        DateTime::from_bytes(&time_buf).ok_or(IrcuError::ProtocolError)
    }

    pub fn get_mets(&mut self) -> IrcResult<Vec<Option<u8>>> {
        let mut mets_buf = [0u8; ReceiveCommand::Mets.address() as usize];
        self.receive(ReceiveCommand::Mets, &mut mets_buf, None)?;
        let mets = decompress_u8(&mets_buf);
        Ok(mets)
    }

    pub fn get_altitude(&mut self) -> IrcResult<Vec<i32>> {
        let mut altitude_buf = [0u8; ReceiveCommand::Altitude.address() as usize];
        self.receive(ReceiveCommand::Altitude, &mut altitude_buf, None)?;
        let altitude = decompress_altitude(&altitude_buf);
        Ok(altitude)
    }

    pub fn get_activity_tag(&mut self) -> IrcResult<Vec<Option<u8>>> {
        let mut activity_buf = [0u8; ReceiveCommand::ActivityTag.address() as usize];
        self.receive(ReceiveCommand::ActivityTag, &mut activity_buf, None)?;
        let activity = decompress_u8(&activity_buf);
        Ok(activity)
    }

    pub fn get_kcals(&mut self) -> IrcResult<Vec<Option<u16>>> {
        let mut kcals_buf = [0u8; ReceiveCommand::Kcals.response_size()];
        self.receive(ReceiveCommand::Kcals, &mut kcals_buf, None)?;
        let kcals = decompress_u16(&kcals_buf);
        Ok(kcals)
    }

    pub fn get_steps(&mut self) -> IrcResult<Vec<Option<u16>>> {
        let mut steps_buf = [0u8; ReceiveCommand::Steps.response_size()];
        self.receive(ReceiveCommand::Steps, &mut steps_buf, None)?;
        let steps = decompress_u16(&steps_buf);
        Ok(steps)
    }
}