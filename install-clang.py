# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in Python programming language.
#
import os

def compile_with_clang():
    compile_command = "clang src/xenly.c src/print_info.c src/color.c src/project.c src/error.c -o xenly -lm"
    compile_status = os.system(compile_command)
    
    # Check if compilation was successful
    if compile_status == 0:
        print("Compilation successful. Running xenly programming language")
    else:
        print("Compilation failed")

compile_with_clang()
