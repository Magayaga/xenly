#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
#
import os

# Compile xenly.c using gcc
compile_command = "gcc src/xenly.c -o xenly -lm"
compile_status = os.system(compile_command)

# Check if compilation was successful
if compile_status == 0:
    print("Compilation successful. Running xenly programming language")
else:
    print("Compilation failed")
    