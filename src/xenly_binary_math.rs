/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Rust programming language.
 *
 * `xenly_binary_math.rs` is the similar to the `xenly_binary_math.c` in C programming language and and
 * `xenly_binary_math.go` in Go programming language.
 *
 * It is available for Linux and Windows operating systems.
 */
use std::ffi::{CStr, CString};
use std::os::raw::c_char;

// Convert binary string to decimal
#[no_mangle]
pub extern "C" fn xenly_bindec(binary: *const c_char) -> f64 {
    if binary.is_null() {
        return 0.0;
    }

    let c_str = unsafe { CStr::from_ptr(binary) };
    if let Ok(binary_str) = c_str.to_str() {
        if let Ok(decimal) = i64::from_str_radix(binary_str, 2) {
            return decimal as f64;
        }
    }
    0.0
}

// Convert decimal to binary string
#[no_mangle]
pub extern "C" fn xenly_decbin(decimal: i32) -> *mut c_char {
    let binary_str = if decimal == 0 {
        "0".to_string()
    } else {
        format!("{:b}", decimal)
    };

    let c_string = CString::new(binary_str).unwrap();
    c_string.into_raw()
}

// Free the memory allocated by xenly_decbin
#[no_mangle]
pub extern "C" fn xenly_free_cstring(s: *mut c_char) {
    if s.is_null() { return; }
    unsafe { 
        let _ = CString::from_raw(s); // This will free the CString
    }
}
