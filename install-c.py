#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in Python programming language.
#
import os
import platform
import sys

def compile_with_compiler(compiler):
    # Define the source files and output file for xenly executable
    source_files = [
        "src/main.c",
        "src/binary_math_functions.c",
        "src/color.c",
        "src/data_structures.c",
        "src/error.c",
        "src/graphics_functions.c",
        "src/math_functions.c",
        "src/print_info.c",
        "src/project.c",
        "src/utility.c",
        "src/variables.c"
    ]
    output_file = "xenly"
    
    # Build the compile command for xenly executable
    compile_command = [compiler] + source_files + ["-o", output_file, "-lm"]

    # Join command parts into a single string for execution
    compile_command_str = " ".join(compile_command)

    # Execute compilation command for xenly executable
    compile_status = os.system(compile_command_str)
    
    # Check if compilation was successful
    if compile_status == 0:
        print(f"Compilation of xenly successful using {compiler}")
    else:
        print(f"Compilation of xenly failed with error code {compile_status} using {compiler}")

def compile_math_library(compiler):
    # Define the source file and output file for math library
    source_file = "src/libm/math/xenly_math.c"
    if platform.system() == "Windows":
        output_file = "math.dll"
        compile_command = f"{compiler} {source_file} -shared -o {output_file} -lm"
    elif platform.system() == "Linux":
        output_file = "math.so"
        compile_command = f"{compiler} {source_file} -shared -o {output_file} -fPIC -lm"
    else:
        print("Unsupported platform")
        return

    # Execute compilation command for math library
    compile_status = os.system(compile_command)
    
    # Check if compilation was successful
    if compile_status == 0:
        print(f"Compilation of {output_file} successful using {compiler}")
    else:
        print(f"Compilation of {output_file} failed with error code {compile_status} using {compiler}")

def compile_binary_math_library(compiler):
    # Define the source file and output file for binary math library
    source_file = "src/libm/binary_math/xenly_binary_math.c"
    if platform.system() == "Windows":
        output_file = "binary_math.dll"
        compile_command = f"{compiler} {source_file} -shared -o {output_file} -lm"
    elif platform.system() == "Linux":
        output_file = "binary_math.so"
        compile_command = f"{compiler} {source_file} -shared -o {output_file} -fPIC -lm"
    else:
        print("Unsupported platform")
        return

    # Execute compilation command for binary math library
    compile_status = os.system(compile_command)
    
    # Check if compilation was successful
    if compile_status == 0:
        print(f"Compilation of {output_file} successful using {compiler}")
    else:
        print(f"Compilation of {output_file} failed with error code {compile_status} using {compiler}")

def run_tests_or_program():
    # Here you can define how to test your program or simply execute it
    # For demonstration purposes, let's assume running the compiled program
    os.system("./xenly")  # Replace with actual testing commands if applicable

def main():
    if len(sys.argv) != 2:
        print("Usage: python install_c.py <compiler>")
        print("Where <compiler> is either 'gcc' or 'clang'")
        sys.exit(1)

    compiler = sys.argv[1].strip()

    if compiler not in ["gcc", "clang"]:
        print("Unsupported compiler. Please choose 'gcc' or 'clang'.")
        sys.exit(1)

    # Compile xenly program
    compile_with_compiler(compiler)

    # Compile math library
    compile_math_library(compiler)

    # Compile binary math library
    compile_binary_math_library(compiler)

    # After successful compilation, run tests or execute the program
    run_tests_or_program()

if __name__ == "__main__":
    main()
