#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in C programming language.
#
#!/bin/bash

# Directory where the Go source files are located
SRC_DIR="src"

# Output directory for the shared library
OUT_DIR="build"

# Name of the shared library
LIB_NAME="goxenly.dll"

# Create the output directory if it doesn't exist
mkdir -p "$OUT_DIR"

# Build the Go shared library
go build -o "$OUT_DIR/$LIB_NAME" -buildmode=c-shared "$SRC_DIR/main.go"
