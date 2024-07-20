/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Rust programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */

use std::fs;
use std::io::{self, Write};
use std::path::Path;

pub fn create_initialize_project(project_name: &str) {
    let project_path = Path::new(project_name);

    if project_path.exists() {
        println!("Error: Project {} already exists.", project_name);
        return;
    }

    if let Err(e) = fs::create_dir(project_path) {
        println!("Error: Could not create project directory {}: {}", project_name, e);
        return;
    }

    create_file(project_path, "main.xe", "nota(\"Hello, World!\")\n\nnota(2*9-6/3*5)\n");

    println!("Project {} created successfully.", project_name);
}

pub fn initialize_project() {
    println!("Initializing a new Xenly project...");

    let mut project_name = String::new();
    print!("Enter project name: ");
    io::stdout().flush().unwrap();
    io::stdin().read_line(&mut project_name).unwrap();
    let project_name = project_name.trim();

    create_initialize_project(project_name);
}

fn create_file(project_path: &Path, file_name: &str, content: &str) {
    let file_path = project_path.join(file_name);
    let mut file = fs::File::create(file_path).expect("Error: Could not create file");
    file.write_all(content.as_bytes()).expect("Error: Could not write to file");
}
