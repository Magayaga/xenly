#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in Python programming language.
#
import os
import platform

def compile_with_compiler(compiler):
    # Define the source files and output file for xenly executable
    source_files = ["src/xenly.c", "src/print_info.c", "src/color.c", "src/project.c", "src/error.c"]
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
    source_file = "src/libm/xenly_math.c"
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
    # Choose compiler: "gcc" or "clang"
    compiler = input("Enter the compiler you want to use (gcc/clang): ").strip()

    if compiler not in ["gcc", "clang"]:
        print("Unsupported compiler. Please choose 'gcc' or 'clang'.")
        return

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
