#![feature(write_all_vectored)]
macro_rules! debug_println {
    ($($arg:tt)*) => {
        #[cfg(debug_assertions)]
        {
            // file!() provides the path to the current file (e.g., "src/main.rs")
            println!("[{}:{}] {}", file!(), line!(), format_args!($($arg)*));
        }
    };
}

mod crc;
pub mod irc;
pub mod ircu;
pub mod error;
pub mod async_util;
