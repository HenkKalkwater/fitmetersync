use std::io::{IoSlice, IoSliceMut};
use std::time::Duration;
use serialport::{available_ports, new as new_port, SerialPortBuilder, DataBits, FlowControl, SerialPort};
use fms_samu::FitMeterSync;

fn choose_port() -> Option<Box<dyn SerialPort>> {
    for p in available_ports().ok()? {
        println!("{} {:#?}", p.port_name, p.port_type);
        if let Ok(port) = new_port(p.port_name, 115_200)
            .data_bits(DataBits::Eight)
            .flow_control(FlowControl::None)
            .timeout(Duration::from_secs(5))
            .open() {
            return Some(port);
        }
    }
    None
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("Hello, world!");

    let port = choose_port().expect("No serial port available");
    let mut con = FitMeterSync::new(port);

    println!("Connected to IR adapter, prepare Wii Fit U meter to send");

    con.connect()?;
    println!("Wii Fit U meter connected");

    // let identity = con.get_identity()?;
    // println!("Wii Fit U meter identity: {:?}", identity);

    // let time = con.get_time()?;
    // println!("Wii Fit U meter time: {}", time);

    let steps = con.get_steps()?;
    println!("Wii Fit U meter steps: {:?}", steps);

    con.disconnect()?;

    Ok(())
}
