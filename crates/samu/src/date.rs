use std::fmt::{Display, Formatter};

#[derive(Debug, Eq, PartialEq, PartialOrd, Ord)]
pub struct DateTime {
    // ???
    flag: u8,
    year: u8,
    month: u8,
    day: u8,
    hour: u8,
    minute: u8,
}

impl DateTime {
    pub fn from_bytes(bytes: &[u8; 6]) -> Option<Self> {
        let [flag, year, month, day, hour, minute] = *bytes;
        if month > 0 && month <= 12 && day <= 31 && hour <= 24 && minute <= 60 {
            Some(
                DateTime {
                    flag,
                    year,
                    month,
                    day,
                    hour,
                    minute
                }
            )
        } else {
            None
        }
    }

    pub fn year(&self) -> i32 {
        self.year as i32 + 2000
    }

    pub fn month(&self) -> u8 {
        self.month
    }

    pub fn day(&self) -> u8 {
        self.day
    }

    pub fn hour(&self) -> u8 {
        self.hour
    }

    pub fn minute(&self) -> u8 {
        self.minute
    }
}

impl Display for DateTime {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:04}-{:02}-{:02} {:02}:{:02}", self.year(), self.month(), self.day(), self.hour(), self.minute())
    }
}