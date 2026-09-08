use std::io::{IoSlice, IoSliceMut};
use std::time::Duration;
use serialport::{available_ports, new as new_port, SerialPortBuilder, DataBits, FlowControl, SerialPort};
use fms_samu::FitMeterSync;

fn open_port(port_name: String) -> Option<Box<dyn SerialPort>> {
    new_port(port_name, 115_200)
        .exclusive(false)
        .data_bits(DataBits::Eight)
        .flow_control(FlowControl::None)
        .timeout(Duration::from_secs(5))
        .open().ok()
}

fn choose_port() -> Option<Box<dyn SerialPort>> {
    for p in available_ports().ok()? {
        println!("{} {:#?}", p.port_name, p.port_type);
        if let Some(port) = open_port(p.port_name) {
            return Some(port);
        }
    }
    None
}

fn print_weekly(data: &[Option<u16>]) {
    let mut i = 0;

    for week in data.chunks(7) {
        print!("{i:3}: ");
        i += 1;
        for day in week {
            match day {
                Some(data) => print!("{:4}  ", data),
                None => print!("      ")
            }
        }
        println!()
    }

}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("Hello, world!");

    let mut port = choose_port().expect("No serial port available");

    // Drain the port first
    let _ = port.bytes_to_read()
        .and_then(|b| {
            println!("Draining port, {} bytes to read", b);
            Ok(port.read_exact(&mut vec![0; b as usize]))
        });

    let mut con = FitMeterSync::new(port);

    println!("Connected to IR adapter, prepare Wii Fit U meter to send");

    con.connect()?;
    println!("Wii Fit U meter connected");

    let identity = con.get_identity()?;
    println!("Wii Fit U meter identity: {:?}", identity);

    let time = con.get_time()?;
    println!("Wii Fit U meter time: {}", time);

    let mets = con.get_mets()?;
    println!("Wii Fit U meter mets:");
    println!("{:?}", mets);

    let altitude = con.get_altitude()?;
    println!("Wii Fit U meter altitude:");
    println!("{:?}", altitude);

    let tags = con.get_activity_tag()?;
    println!("Wii Fit U meter activity tags:");
    println!("{:?}", tags);

    let kcal = con.get_kcals()?;
    println!("Wii Fit U meter kcal:");
    print_weekly(&kcal);

    let steps = con.get_steps()?;
    println!("Wii Fit U meter steps:");
    print_weekly(&steps);


    //println!("Wii Fit U meter steps: {:?}", steps);

    con.disconnect()?;

    Ok(())
}
