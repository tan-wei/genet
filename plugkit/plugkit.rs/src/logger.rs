//! Macros for logging.

use super::symbol;
use super::context::Context;
use std::ffi::CString;

pub enum Level {
    Debug = 0,
    Info  = 1,
    Warn  = 2,
    Error = 3
}

#[repr(C)]
pub(crate) struct Metadata {
    pub level: Level,
    pub file: &'static str,
    pub line: u32,
    pub column: u32
}

pub fn write_log(ctx: &mut Context, msg: String, level: Level, file: &'static str, line: u32, column: u32) {
    unsafe {
        symbol::Logger_log.unwrap()(ctx,
            CString::new(msg).unwrap().as_ptr(),
            &Metadata{
                level: level,
                file: file,
                line: line,
                column: column,
            });
    }
}

/// Write debug-level log
#[macro_export]
macro_rules! debug {
    ($ctx:ident, $fmt:expr) =>
        (::plugkit::logger::write_log($ctx, format!($fmt), ::plugkit::logger::Level::Debug,
            file!(), line!(), column!()));
    ($ctx:ident, $fmt:expr, $($arg:tt)*) =>
    (::plugkit::logger::write_log($ctx, format!($fmt, $($arg)*), ::plugkit::logger::Level::Debug,
        file!(), line!(), column!()));
}

/// Write info-level log
#[macro_export]
macro_rules! info {
    ($ctx:ident, $fmt:expr) =>
        (::plugkit::logger::write_log($ctx, format!($fmt), ::plugkit::logger::Level::Info,
            file!(), line!(), column!()));
    ($ctx:ident, $fmt:expr, $($arg:tt)*) =>
    (::plugkit::logger::write_log($ctx, format!($fmt, $($arg)*), ::plugkit::logger::Level::Info,
        file!(), line!(), column!()));
}

/// Write warn-level log
#[macro_export]
macro_rules! warn {
    ($ctx:ident, $fmt:expr) =>
        (::plugkit::logger::write_log($ctx, format!($fmt), ::plugkit::logger::Level::Warn,
            file!(), line!(), column!()));
    ($ctx:ident, $fmt:expr, $($arg:tt)*) =>
    (::plugkit::logger::write_log($ctx, format!($fmt, $($arg)*), ::plugkit::logger::Level::Warn,
        file!(), line!(), column!()));
}

/// Write error-level log
#[macro_export]
macro_rules! error {
    ($ctx:ident, $fmt:expr) =>
        (::plugkit::logger::write_log($ctx, format!($fmt), ::plugkit::logger::Level::Error,
            file!(), line!(), column!()));
    ($ctx:ident, $fmt:expr, $($arg:tt)*) =>
    (::plugkit::logger::write_log($ctx, format!($fmt, $($arg)*), ::plugkit::logger::Level::Error,
        file!(), line!(), column!()));
}
