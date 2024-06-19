/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Rust programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
extern crate minifb;
use minifb::{Window, WindowOptions};

// Make sure to expose only the required functions as `pub extern "C"`
#[repr(C)]
pub struct Renderer {
    width: usize,
    height: usize,
    buffer: *mut u32,
}

// Functions to create, manipulate, and free the renderer
#[no_mangle]
pub extern "C" fn renderer_new(width: usize, height: usize) -> *mut Renderer {
    let buffer = vec![0; width * height];
    let renderer = Renderer {
        width,
        height,
        buffer: buffer.into_boxed_slice().as_mut_ptr(),
    };
    Box::into_raw(Box::new(renderer))
}

#[no_mangle]
pub extern "C" fn renderer_draw_circle(renderer: *mut Renderer, x: usize, y: usize, radius: usize, color: u32) {
    if renderer.is_null() { return; }
    let r = unsafe { &mut *renderer };
    for i in 0..r.width {
        for j in 0..r.height {
            let dx = x as isize - i as isize;
            let dy = y as isize - j as isize;
            if dx * dx + dy * dy <= (radius as isize * radius as isize) {
                unsafe { *r.buffer.add(j * r.width + i) = color };
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn renderer_update(renderer: *mut Renderer, window: *mut Window) {
    if renderer.is_null() || window.is_null() { return; }
    let r = unsafe { &mut *renderer };
    let w = unsafe { &mut *window };
    let buffer_slice = unsafe { std::slice::from_raw_parts(r.buffer, r.width * r.height) };
    w.update_with_buffer(buffer_slice, r.width, r.height).unwrap();
}

#[no_mangle]
pub extern "C" fn renderer_free(renderer: *mut Renderer) {
    if !renderer.is_null() {
        unsafe { Box::from_raw(renderer) }; // Will drop the Box and free the memory
    }
}

#[no_mangle]
pub extern "C" fn window_new(width: usize, height: usize) -> *mut Window {
    let window = Window::new(
        "Rust Graphics Window",
        width,
        height,
        WindowOptions::default(),
    ).unwrap_or_else(|_| std::ptr::null_mut());
    Box::into_raw(Box::new(window))
}

#[no_mangle]
pub extern "C" fn window_free(window: *mut Window) {
    if !window.is_null() {
        unsafe { Box::from_raw(window) }; // Will drop the Box and free the memory
    }
}

#[no_mangle]
pub extern "C" fn window_is_open(window: *mut Window) -> bool {
    if window.is_null() { return false; }
    let w = unsafe { &mut *window };
    w.is_open()
}

#[no_mangle]
pub extern "C" fn window_is_key_down(window: *mut Window, key: minifb::Key) -> bool {
    if window.is_null() { return false; }
    let w = unsafe { &mut *window };
    w.is_key_down(key)
}