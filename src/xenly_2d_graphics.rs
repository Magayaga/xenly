/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Rust programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
use minifb::{Key, Window, WindowOptions};

pub struct Renderer {
    width: usize,
    height: usize,
    buffer: Vec<u32>,
}

impl Renderer {
    pub fn new(width: usize, height: usize) -> Renderer {
        Renderer {
            width,
            height,
            buffer: vec![0; width * height],
        }
    }

    pub fn draw_circle(&mut self, x: usize, y: usize, radius: usize, color: u32) {
        for i in 0..self.width {
            for j in 0..self.height {
                let dx = x as isize - i as isize;
                let dy = y as isize - j as isize;
                if dx * dx + dy * dy <= (radius as isize * radius as isize) {
                    self.buffer[j * self.width + i] = color;
                }
            }
        }
    }

    pub fn update(&mut self, window: &mut Window) {
        window
            .update_with_buffer(&self.buffer, self.width, self.height)
            .unwrap();
    }
}