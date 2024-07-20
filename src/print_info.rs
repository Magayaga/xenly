/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Rust programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */

use colored::*;

#[cfg(target_os = "windows")]
use winapi::um::winnt::OSVERSIONINFOEXW;
use winapi::um::sysinfoapi::GetVersionExW;

const XENLY_VERSION: &str = "0.1.0-preview9";
const XENLY_AUTHORS: &str = "Cyril John Magayaga";

pub fn print_version() {
    println!("Xenly {} (Pre-alpha release)", XENLY_VERSION);
    println!("Copyright (c) 2023-2024 {}", XENLY_AUTHORS.blue().on_white());
}

pub fn print_dumpversion() {
    println!("{}", XENLY_VERSION);
}

pub fn print_help() {
    println!("Usage: xenly [input file]");
    println!("{}", " Options: ".blue().on_white());
    println!("  -h, --help               Display this information");
    println!("  -v, --version            Display compiler version information");
    println!("  -os, --operatingsystem   Display operating system information");
    println!("  -dm, --dumpmachine       Display machine information");
    // println!("  -drd, --dumpreleasedate  Display compiler release date information");
    println!("  -dv, --dumpversion       Display the compiler version information");
    println!("  --create-project         Create a new project");
}

#[cfg(target_os = "windows")]
pub fn print_operatingsystem() {
    let mut osvi: OSVERSIONINFOEXW = unsafe { std::mem::zeroed() };
    osvi.dwOSVersionInfoSize = std::mem::size_of::<OSVERSIONINFOEXW>() as u32;

    unsafe {
        if GetVersionExW(&mut osvi as *mut _ as *mut _) == 0 {
            eprintln!("Error: Unable to get OS version information.");
        }
        
        else {
            println!("Operating System: Windows");
            println!("Version: {}.{}.{}", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
        }
    }
}

#[cfg(target_os = "linux")]
pub fn print_operatingsystem() {
    let os_release = std::fs::read_to_string("/etc/os-release").unwrap_or_default();
    let version = os_release.lines()
        .find(|line| line.starts_with("VERSION="))
        .map(|line| line.trim_start_matches("VERSION=").trim_matches('"'))
        .unwrap_or("Unknown");

    println!("Operating System: Linux");
    println!("Version: {}", version);
}

pub fn print_dumpmachines() {
    println!("{}", std::env::consts::ARCH);
}

pub fn print_author() {
    println!("Cyril John Magayaga");
}
