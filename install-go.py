# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in Python programming language.
#
import os

def compile_with_go():
    compile_command = "go build -o src/goxenly.dll -buildmode=c-shared src/goxenly.go"
    compile_status = os.system(compile_command)
    
    # Check if compilation was successful
    if compile_status == 0:
        print("Compilation successful. GoLang (goxenly.go) --> C programming language (goxenly.h)")
    else:
        print("Compilation failed")

compile_with_go()
