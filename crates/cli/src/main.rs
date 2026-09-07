use std::io::{IoSlice, IoSliceMut};
use std::time::Duration;
use serialport::{available_ports, new as new_port, SerialPortBuilder, DataBits, FlowControl, SerialPort};
use fms_irc::irc::Irc;

fn choose_port() -> Option<Box<dyn SerialPort>> {
    for p in available_ports().ok()? {
        println!("{} {:#?}", p.port_name, p.port_type);
        if let Ok(port) = new_port(p.port_name, 115_200)
            .data_bits(DataBits::Eight)
            .flow_control(FlowControl::None)
            .timeout(Duration::from_secs(10))
            .open() {
            return Some(port);
        }
    }
    None
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("Hello, world!");

    let port = choose_port().expect("No serial port available");
    let mut irc = Irc::new(port);

    println!("Connected to IR adapter, prepare Wii Fit U meter to send");

    irc.wait_connection()?;
    println!("Wii Fit U meter connected");

    let mut recv_buf = [0u8; 32];

    {
        let mut recv_slice = [IoSliceMut::new(&mut recv_buf)];
        let packet1 = irc.receive(&mut recv_slice)?;
        println!("Wii Fit U meter received packet: {:#?}:\n{:#?}", packet1, &recv_buf);
    }

    let cmd = [0xF4, 0x01, 0x00, 0x00];
    let mut cmd_slice = [IoSlice::new(&cmd)];

    irc.send_payload(&mut cmd_slice, 0x0A)?;
    println!("Wii Fit U meter sent command");

    {
        let mut recv_slice = [IoSliceMut::new(&mut recv_buf)];
        let packet2 = irc.receive(&mut recv_slice)?;
        println!("Wii Fit U meter received packet: {:#?}:\n{:#?}", packet2, &recv_buf);
    }

    Ok(())
}
