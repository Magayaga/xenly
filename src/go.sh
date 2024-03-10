#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in Bash Script.
#
#!/bin/bash

# Name of the shared library (.dll, .so, and more)
LIB_NAME="goxenly.dll"

# Build the Go shared library into C programming language.
go build -o "$LIB_NAME" -buildmode=c-shared "goxenly.go"
