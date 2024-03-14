import os

def compile_with_clang():
    compile_command = "clang src/xenly.c src/print_info.c src/color.c -o xenly -lm"
    compile_status = os.system(compile_command)
    
    # Check if compilation was successful
    if compile_status == 0:
        print("Compilation successful. Running xenly programming language")
    else:
        print("Compilation failed")

compile_with_clang()
