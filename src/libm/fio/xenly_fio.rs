/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Rust programming language.
 *
 * `xenly_fio.rs` is the similar to the `xenly_fio.c` in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_double, c_int};
use std::io::Write;
use std::ptr::null_mut;
use libc::{FILE, fputs};

// print
#[no_mangle]
pub extern "C" fn print(format: *const c_char, ...) {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        let args: Vec<&dyn std::fmt::Display> = vec![];
        // Call the Rust function with an empty args list.
        print_function(format, &args);
    }
}

fn print_function(format: &str, args: &[&dyn std::fmt::Display]) {
    let mut formatted_string = String::new();
    let mut fmt_iter = format.chars().peekable();
    let mut args_iter = args.iter();

    while let Some(ch) = fmt_iter.next() {
        if ch == '%' {
            if let Some(&'%') = fmt_iter.peek() {
                fmt_iter.next(); // Skip the second '%'
                formatted_string.push('%');
            }
            
            else {
                if let Some(arg) = args_iter.next() {
                    formatted_string.push_str(&arg.to_string());
                }
            }
        }
        
        else {
            formatted_string.push(ch);
        }
    }

    print!("{}", formatted_string);
}

// sprint
#[no_mangle]
pub extern "C" fn sprint(buffer: *mut c_char, format: *const c_char, ...) {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        let mut buf = CString::new(Vec::with_capacity(1024)).unwrap();
        let args: Vec<&dyn std::fmt::Display> = vec![];
        sprint_function(&mut buf, format, &args);
        std::ptr::copy_nonoverlapping(buf.as_ptr(), buffer, buf.to_bytes().len() + 1);
    }
}

fn sprint_function(buffer: &mut CString, format: &str, args: &[&dyn std::fmt::Display]) {
    let mut fmt_iter = format.chars().peekable();
    let mut args_iter = args.iter();

    while let Some(ch) = fmt_iter.next() {
        if ch == '%' {
            if let Some(&'%') = fmt_iter.peek() {
                fmt_iter.next(); // Skip the second '%'
                buffer.push('%').unwrap();
            }
            
            else {
                if let Some(arg) = args_iter.next() {
                    buffer.push_str(&arg.to_string()).unwrap();
                }
            }
        }
        
        else {
            buffer.push(ch).unwrap();
        }
    }
}


// input
#[no_mangle]
pub extern "C" fn input(format: *const c_char, ...) -> c_int {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        // Read input from stdin
        let mut input = String::new();
        std::io::stdin().read_line(&mut input).expect("Failed to read line");
        // Handle input parsing according to format
        0 // Return 0 to indicate success
    }
}

// println
#[no_mangle]
pub extern "C" fn println(format: *const c_char, ...) {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        let args: Vec<&dyn std::fmt::Display> = vec![];
        println_function(format, &args);
    }
}

fn println_function(format: &str, args: &[&dyn std::fmt::Display]) {
    let mut formatted_string = String::new();
    let mut fmt_iter = format.chars().peekable();
    let mut args_iter = args.iter();

    while let Some(ch) = fmt_iter.next() {
        if ch == '%' {
            if let Some(&'%') = fmt_iter.peek() {
                fmt_iter.next(); // Skip the second '%'
                formatted_string.push('%');
            }
            
            else {
                if let Some(arg) = args_iter.next() {
                    formatted_string.push_str(&arg.to_string());
                }
            }
        }
        
        else {
            formatted_string.push(ch);
        }
    }

    println!("{}", formatted_string);
}

// scanln
#[no_mangle]
pub extern "C" fn scanln(buffer: *mut c_char, size: usize) {
    unsafe {
        let mut buf = vec![0u8; size];
        std::io::stdin().read_exact(&mut buf).expect("Failed to read line");
        let s = String::from_utf8_lossy(&buf).trim_end().to_string();
        std::ptr::copy_nonoverlapping(s.as_ptr(), buffer as *mut u8, s.len());
        *buffer.add(s.len()) = 0; // Null-terminate the C string
    }
}


// sprintln
#[no_mangle]
pub extern "C" fn sprintln(buffer: *mut c_char, format: *const c_char, ...) {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        let mut buf = CString::new(Vec::with_capacity(1024)).unwrap();
        let args: Vec<&dyn std::fmt::Display> = vec![];
        sprintln_function(&mut buf, format, &args);
        std::ptr::copy_nonoverlapping(buf.as_ptr(), buffer, buf.to_bytes().len() + 1);
    }
}

fn sprintln_function(buffer: &mut CString, format: &str, args: &[&dyn std::fmt::Display]) {
    sprint_function(buffer, format, args);
    buffer.push('\n').unwrap();
}

#[no_mangle]
pub extern "C" fn error(format: *const c_char, ...) {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        let args: Vec<&dyn std::fmt::Display> = vec![];
        error_function(format, &args);
    }
}

// error
fn error_function(format: &str, args: &[&dyn std::fmt::Display]) {
    let mut formatted_string = String::new();
    let mut fmt_iter = format.chars().peekable();
    let mut args_iter = args.iter();

    while let Some(ch) = fmt_iter.next() {
        if ch == '%' {
            if let Some(&'%') = fmt_iter.peek() {
                fmt_iter.next(); // Skip the second '%'
                formatted_string.push('%');
            }
            
            else {
                if let Some(arg) = args_iter.next() {
                    formatted_string.push_str(&arg.to_string());
                }
            }
        }
        
        else {
            formatted_string.push(ch);
        }
    }

    eprint!("{}", formatted_string);
}

// errorln
#[no_mangle]
pub extern "C" fn errorln(format: *const c_char, ...) {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        let args: Vec<&dyn std::fmt::Display> = vec![];
        errorln_function(format, &args);
    }
}

fn errorln_function(format: &str, args: &[&dyn std::fmt::Display]) {
    error_function(format, args);
    eprintln!();
}

// write
#[no_mangle]
pub extern "C" fn write(file: *mut FILE, format: *const c_char, ...) -> c_int {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        let mut buffer = String::new();
        write_function(&mut buffer, format, std::ptr::null());
        let c_str = CString::new(buffer).unwrap();
        fputs(c_str.as_ptr(), file)
    }
}

// writeln
#[no_mangle]
pub extern "C" fn writeln(file: *mut FILE, format: *const c_char, ...) -> c_int {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        let mut buffer = String::new();
        write_function(&mut buffer, format, std::ptr::null());
        buffer.push('\n');
        let c_str = CString::new(buffer).unwrap();
        fputs(c_str.as_ptr(), file)
    }
}

fn write_function(buffer: &mut String, format: &str, _args: *const c_void) {
    let mut fmt_iter = format.chars().peekable();
    while let Some(ch) = fmt_iter.next() {
        if ch == '%' {
            if let Some(&'%') = fmt_iter.peek() {
                fmt_iter.next();
                buffer.push('%');
            }
            
            else {
                buffer.push_str("<format>");
            }
        }
        
        else {
            buffer.push(ch);
        }
    }
}

// vfprint
#[no_mangle]
pub extern "C" fn vfprint(file: *mut FILE, format: *const c_char, args: *mut libc::va_list) -> c_int {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        let mut buffer = String::new();
        vfprint_function(&mut buffer, format, args);
        let c_str = CString::new(buffer).unwrap();
        fputs(c_str.as_ptr(), file)
    }
}

fn vfprint_function(buffer: &mut String, format: &str, _args: *mut libc::va_list) {
    let mut fmt_iter = format.chars().peekable();
    while let Some(ch) = fmt_iter.next() {
        if ch == '%' {
            if let Some(&'%') = fmt_iter.peek() {
                fmt_iter.next(); // Skip the second '%'
                buffer.push('%');
            }
            
            else {
                // Handle the variadic argument
                // Note: This is a simplified version; real implementation needs detailed handling.
                buffer.push_str("<formatted>");
            }
        }
        
        else {
            buffer.push(ch);
        }
    }
}

// vfprintln
#[no_mangle]
pub extern "C" fn vfprintln(file: *mut FILE, format: *const c_char, args: *mut libc::va_list) -> c_int {
    unsafe {
        let format = CStr::from_ptr(format).to_str().unwrap();
        let mut buffer = String::new();
        vfprint_function(&mut buffer, format, args);
        buffer.push('\n');
        let c_str = CString::new(buffer).unwrap();
        fputs(c_str.as_ptr(), file)
    }
}
