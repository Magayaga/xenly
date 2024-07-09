#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in Bash Script.
#
#!/bin/bash

# Directory where the Go source files are located
SRC_DIR="src/libm"

# Output directory for the shared library
OUT_DIR="bin"

# Determine the output file extension based on the Operating system
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    LIB_EXT="so"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    # For macOS, .dylib is typically used for shared libraries
    LIB_EXT="dylib"
elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]] || [[ "$OSTYPE" == "win64" ]]; then
    LIB_EXT="dll"
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

# Create the output directory if it doesn't exist
mkdir -p "$OUT_DIR"

# Build the Go shared library
rustc --crate-type cdylib -o "math.${LIB_EXT}" "$SRC_DIR/math/xenly_math.rs"
rustc --crate-type cdylib -o "binary_math.${LIB_EXT}" "$SRC_DIR/binary_math/xenly_binary_math.rs"

# Check if the build was successful
if [[ $? -eq 0 ]]; then
    echo "Library built successfully"
else
    echo "Build failed."
    exit 1
fi
