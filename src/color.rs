pub fn red() {
    print!("\x1B[1;31m");
}

pub fn green() {
    print!("\x1B[1;32m");
}

pub fn yellow() {
    print!("\x1B[1;33m");
}

pub fn blue() {
    print!("\x1B[1;34m");
}

pub fn purple() {
    print!("\x1B[1;35m");
}

pub fn white() {
    print!("\x1B[1;37m");
}

pub fn orange() {
    print!("\x1B[1;38;5;208m");
}

pub fn cyan() {
    print!("\x1B[1;36m");
}

pub fn reset_color() {
    print!("\x1B[0m");
}

pub fn set_background_red() {
    print!("\x1B[41m");
}

pub fn set_background_green() {
    print!("\x1B[42m");
}

pub fn set_background_yellow() {
    print!("\x1B[43m");
}

pub fn set_background_blue() {
    print!("\x1B[44m");
}

pub fn set_background_purple() {
    print!("\x1B[45m");
}

pub fn set_background_white() {
    print!("\x1B[47m");
}

pub fn reset_background_color() {
    print!("\x1B[49m");
}

pub fn black_and_orange() {
    print!("\x1B[0;30;48;5;208m");
}
