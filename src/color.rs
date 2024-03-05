/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Rust programming language.
 *
 */

pub enum Color {
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    Orange, // New color added
    // Add more colors as needed
}

pub enum BackgroundColor {
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    OrangeBackground, // New background color added
    // Add more background colors as needed
}

impl Color {
    pub fn to_ansi_code(&self) -> &'static str {
        match self {
            Color::Black => "\x1b[30m",
            Color::Red => "\x1b[31m",
            Color::Green => "\x1b[32m",
            Color::Yellow => "\x1b[33m",
            Color::Blue => "\x1b[34m",
            Color::Magenta => "\x1b[35m",
            Color::Cyan => "\x1b[36m",
            Color::White => "\x1b[37m",
            Color::Orange => "\x1b[38;5;208m", // Orange ANSI color code
        }
    }
}

impl BackgroundColor {
    pub fn to_ansi_code(&self) -> &'static str {
        match self {
            BackgroundColor::Black => "\x1b[40m",
            BackgroundColor::Red => "\x1b[41m",
            BackgroundColor::Green => "\x1b[42m",
            BackgroundColor::Yellow => "\x1b[43m",
            BackgroundColor::Blue => "\x1b[44m",
            BackgroundColor::Magenta => "\x1b[45m",
            BackgroundColor::Cyan => "\x1b[46m",
            BackgroundColor::White => "\x1b[47m",
            BackgroundColor::OrangeBackground => "\x1b[48;5;208m", // Orange background ANSI code
        }
    }
}

pub fn reset() -> &'static str {
    "\x1b[0m"
}
