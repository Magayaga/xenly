extern crate cc;

fn main() {
  cc::Build::new().file("src/xenly.c").compile("xenly");
}
