/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Rust programming language.
 * 
 * It is available for Linux and Windows operating systems.
 *
 */
use std::env;
use std::fs::File;
use std::io::{self, BufRead};
use std::path::Path;

mod project;
mod print_info;
mod xenly;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() == 3 && args[1] == "--create-project" {
        // Create initialize project
        project::create_initialize_project(&args[2]);
        return;
    }

    if args.len() != 2 {
        eprintln!("Usage: xenly [input file]");
        return;
    }
    
    else if args[1] == "--version" || args[1] == "-v" {
        print_info::print_version();
        return;
    }
    
    else if args[1] == "--help" || args[1] == "-h" {
        print_info::print_help();
        return;
    }
    
    else if args[1] == "--operatingsystem" || args[1] == "-os" {
        print_info::print_operatingsystem();
        return;
    }
    
    else if args[1] == "--dumpmachine" || args[1] == "-dm" {
        print_info::print_dumpmachines();
        return;
    }
    
    else if args[1] == "--dumpversion" || args[1] == "-dv" {
        print_info::print_dumpversion();
        return;
    }
    
    else if args[1] == "--new-project" {
        project::initialize_project();
        return;
    }
    
    else if args[1] == "--author" {
        print_info::print_author();
        return;
    }

    let filename = &args[1];
    if let Ok(file) = File::open(filename) {
        let reader = io::BufReader::new(file);
        for line in reader.lines() {
            if let Ok(line) = line {
                xenly::process_line(&line);
            }
        }
    }
    
    else {
        eprintln!("Error: Could not open file {}", filename);
    }
}
