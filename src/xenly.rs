/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Rust programming language.
 * 
 * It is available for Linux and Windows operating systems.
 *
 */
use std::str::FromStr;
use std::collections::HashMap;

const MAX_VARIABLES: usize = 100;

#[derive(Debug, Clone)]
enum VariableValue {
    String(String),
    Integer(i32),
    Float(f32),
    Boolean(bool),
}

#[derive(Debug, Clone)]
struct Variable {
    name: String,
    value: VariableValue,
}

struct Interpreter {
    variables: HashMap<String, Variable>,
    in_multi_line_comment: bool,
}

impl Interpreter {
    fn new() -> Self {
        Self {
            variables: HashMap::new(),
            in_multi_line_comment: false,
        }
    }

    fn process_line(&mut self, line: &str) {
        if let Some(processed_line) = self.remove_comments(line) {
            self.interpret_line(&processed_line);
        }
    }

    fn remove_comments(&mut self, line: &str) -> Option<String> {
        let mut cleaned_line = String::new();
        let mut chars = line.chars().peekable();

        while let Some(&ch) = chars.peek() {
            if self.in_multi_line_comment {
                if ch == '*' && chars.clone().nth(1) == Some('/') {
                    self.in_multi_line_comment = false;
                    chars.next();
                    chars.next();
                }
                
                else {
                    chars.next();
                }
            }
            
            else if ch == '/' && chars.clone().nth(1) == Some('/') {
                break;  // Ignore rest of the line
            }
            
            else if ch == '/' && chars.clone().nth(1) == Some('*') {
                self.in_multi_line_comment = true;
                chars.next();
                chars.next();
            }
            
            else {
                cleaned_line.push(ch);
                chars.next();
            }
        }

        if cleaned_line.is_empty() {
            None
        } else {
            Some(cleaned_line)
        }
    }

    fn interpret_line(&mut self, line: &str) {
        let trimmed_line = line.trim();

        if trimmed_line.starts_with("nota(") {
            let args = &trimmed_line[5..trimmed_line.len() - 1]; // Remove "nota(" and ")"
            self.print_function(args);
        }
        
        else if trimmed_line.starts_with("var ") {
            self.var_declaration(&trimmed_line[4..]);
        }
        
        else if trimmed_line.starts_with("bool ") {
            self.bool_declaration(&trimmed_line[5..]);
        }
    }

    fn print_function(&self, args: &str) {
        let trimmed_args = args.trim();

        if trimmed_args.starts_with('"') && trimmed_args.ends_with('"') {
            // String literal
            println!("{}", &trimmed_args[1..trimmed_args.len() - 1]);
        }
        
        else if let Some(value) = self.get_variable_value(trimmed_args) {
            // Variable
            println!("{}", value);
        }
        
        else {
            // Assume it's a numeric expression
            let result = self.evaluate_expression(trimmed_args);
            println!("{}", result);
        }
    }

    fn var_declaration(&mut self, args: &str) {
        let parts: Vec<&str> = args.split('=').collect();
        if parts.len() != 2 {
            eprintln!("Error: Invalid variable declaration");
            return;
        }

        let name = parts[0].trim();
        let value = parts[1].trim();

        if self.variables.len() >= MAX_VARIABLES {
            eprintln!("Error: Maximum number of variables reached.");
            return;
        }

        let variable = if value.starts_with('"') && value.ends_with('"') {
            Variable {
                name: name.to_string(),
                value: VariableValue::String(value[1..value.len() - 1].to_string()),
            }
        }
        
        else if value.contains('.') {
            Variable {
                name: name.to_string(),
                value: VariableValue::Float(f32::from_str(value).unwrap()),
            }
        }
        
        else {
            Variable {
                name: name.to_string(),
                value: VariableValue::Integer(i32::from_str(value).unwrap()),
            }
        };

        self.variables.insert(name.to_string(), variable);
    }

    fn bool_declaration(&mut self, args: &str) {
        let parts: Vec<&str> = args.split('=').collect();
        if parts.len() != 2 {
            eprintln!("Error: Invalid boolean declaration");
            return;
        }

        let name = parts[0].trim();
        let value = parts[1].trim();

        if self.variables.len() >= MAX_VARIABLES {
            eprintln!("Error: Maximum number of variables reached.");
            return;
        }

        let variable = Variable {
            name: name.to_string(),
            value: VariableValue::Boolean(self.parse_bool(value)),
        };

        self.variables.insert(name.to_string(), variable);
    }

    fn get_variable_value(&self, name: &str) -> Option<String> {
        self.variables.get(name).map(|var| match &var.value {
            VariableValue::String(s) => s.clone(),
            VariableValue::Integer(i) => i.to_string(),
            VariableValue::Float(f) => f.to_string(),
            VariableValue::Boolean(b) => b.to_string(),
        })
    }

    fn evaluate_expression(&self, expr: &str) -> f64 {
        let mut result = 0.0;
        let mut tokens = expr.split_whitespace();

        if let Some(first) = tokens.next() {
            result = f64::from_str(first).unwrap_or(0.0);
        }

        while let Some(op) = tokens.next() {
            if let Some(num) = tokens.next() {
                let num = f64::from_str(num).unwrap_or(0.0);
                match op {
                    "+" => result += num,
                    "-" => result -= num,
                    "*" => result *= num,
                    "/" => result /= num,
                    _ => (),
                }
            }
        }

        result
    }

    fn parse_bool(&self, value: &str) -> bool {
        match value {
            "true" => true,
            "false" => false,
            _ => {
                eprintln!("Error: Invalid boolean value '{}'. Defaulting to false.", value);
                false
            }
        }
    }
}

pub fn process_line(line: &str) {
    static mut INTERPRETER: Option<Interpreter> = None;
    unsafe {
        if INTERPRETER.is_none() {
            INTERPRETER = Some(Interpreter::new());
        }

        if let Some(interpreter) = &mut INTERPRETER {
            interpreter.process_line(line);
        }
    }
}
