import os

def compile_with_compiler(compiler):
    compile_command = f"{compiler} src/xenly.c src/print_info.c src/color.c -o xenly -lm"
    compile_status = os.system(compile_command)
    
    # Check if compilation was successful
    if compile_status == 0:
        print("Compilation successful. Running xenly programming language")
    else:
        print("Compilation failed")

# Choose compiler (GCC or Clang)
compiler_choice = input("Choose compiler (GCC or Clang): ").lower()

# Validate compiler choice
if compiler_choice == "gcc" or compiler_choice == "clang":
    compile_with_compiler(compiler_choice)
else:
    print("Invalid compiler choice. Please choose between GCC or Clang.")
