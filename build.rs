extern crate cc;

fn main() {
  cc::Build::new().file("sources/xenly.c").compile("xenly");
}
