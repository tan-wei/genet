use std::{error, fmt, str};
use string::SafeString;

#[repr(C)]
#[derive(Clone, PartialEq)]
pub struct Error {
    desc: SafeString,
}

impl Error {
    pub fn new(desc: &str) -> Error {
        Self {
            desc: SafeString::from(desc),
        }
    }
}

impl fmt::Debug for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Error: {}", error::Error::description(self))
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", error::Error::description(self))
    }
}

impl error::Error for Error {
    fn description(&self) -> &str {
        &self.desc
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::error;

    #[test]
    fn description() {
        let msg = "out of bounds";
        let err = Error::new(msg);
        assert_eq!(error::Error::description(&err), msg);
    }
}
