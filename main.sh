#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in Bash Script.
#
if command -v gcc >/dev/null 2>&1; then
    compiler="gcc"
elif command -v clang >/dev/null 2>&1; then
    compiler="clang"
else
    echo "Error: No suitable C compiler found (gcc, tcc, or clang)"
    exit 1
fi

# Compile xenly.c with the selected compiler
$compiler src/main.c src/binary_math_functions.c src/color.c src/error.c src/print_info.c src/project.c src/data_structures.c src/graphics_functions.c src/math_functions.c src/utility.c src/variables.c -o xenly -lm
$compiler src/libm/math/xenly_math.c -shared -o math.so -fPIC -lm
$compiler src/libm/binary_math/xenly_binary_math.c -shared -o binary_math.so -fPIC -lm
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
chmod +rx math.so

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running xenly programming language"
    ./xenly
else
    echo "Compilation failed."
fi
