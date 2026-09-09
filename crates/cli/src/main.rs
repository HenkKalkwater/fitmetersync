use clap::{Arg, Command};
use futures_lite::future::block_on;
use serialport_stream::{new as new_port, DataBits, FlowControl, SerialPortStream};
use fms_samu::FitMeterSync;

fn open_port(port_name: String) -> Option<SerialPortStream> {
    new_port(port_name, 115_200)
        .data_bits(DataBits::Eight)
        .flow_control(FlowControl::None)
        //.timeout(Duration::from_secs(5))
        .open().ok()
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
async fn async_main() -> Result<(), Box<dyn std::error::Error>> {
    let port_arg = Arg::new("port")
        .required(true)
        .help("Serial port to use");
    let matches = Command::new("fms-cli")
        .disable_version_flag(true)
        .arg(port_arg)
        .get_matches();

    let port_name = matches.get_one::<String>("port").unwrap().to_string();
    let port = open_port(port_name).expect("No serial port available");
    let mut con = FitMeterSync::new(port);

    println!("Connected to IR adapter, prepare Wii Fit U meter to send");

    con.connect().await?;
    println!("Wii Fit U meter connected");

    let identity = con.get_identity().await?;
    println!("Wii Fit U meter identity: {:?}", identity);

    let time = con.get_time().await?;
    println!("Wii Fit U meter time: {}", time);

    let mets = con.get_mets().await?;
    println!("Wii Fit U meter mets:");
    println!("{:?}", mets);

    let altitude = con.get_altitude().await?;
    println!("Wii Fit U meter altitude:");
    println!("{:?}", altitude);

    let tags = con.get_activity_tag().await?;
    println!("Wii Fit U meter activity tags:");
    println!("{:?}", tags);

    let kcal = con.get_kcals().await?;
    println!("Wii Fit U meter kcal:");
    print_weekly(&kcal);

    let steps = con.get_steps().await?;
    println!("Wii Fit U meter steps:");
    print_weekly(&steps);


    //println!("Wii Fit U meter steps: {:?}", steps);

    con.disconnect().await?;

    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>>{
    block_on(async {
        async_main().await
    })
}