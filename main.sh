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
    echo "Error: No suitable C compiler found (gcc or clang)"
    exit 1
fi

# Compile xenly.c with the selected compiler
$compiler src/main.c src/ast.c src/interpreter.c src/lexer.c src/modules.c src/parser.c -o xenly -lm

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running xenly programming language"
    ./xenly
else
    echo "Compilation failed."
fi
