# `xvm`

**XVM**, also known as **Xenly Virtual Machine**, is a virtual machine that enables a computer to run Xenly programs and programs written in other languages that are also compiled into Xenly bytecode. The XVM is detailed by a specification that formally describes what is required in a Xenly implementation. It was available for the **Windows**, **macOS**, and **Linux** operating systems.

## Filename extensions

* `.xe` and `.xenly` (**Xenly source file**) — It is the source file for the Xenly programming language.
* `.xebc` (**Xenly bytecode**) - a file containing Xenly bytecode that can be executed on the XVM.

## Getting started
1. Install Go 1.21+ from [go.dev/dl](https://go.dev/dl) if you don't have it:
   ```bash
   # go1.21 or later
   $ go version
   ```

2. Build the **XVM**:
   ```bash
   $ make
   ```

3. Cross-compile for other platforms:
   ```bash
   $ make cross-windows-amd64
   $ make cross-darwin-arm64
   $ make cross-all   
   ```

4. **`xvm`** can be compiled into a bytecode executable:
   ```bash
   $ ./bin/xenlybyc examples/hello.xe -o hello.xebc
   ```

5. **xvm** can be executed:
   ```bash
   $ ./bin/xenlyrun hello.xebc
   ```

## Copyright

Copyright (c) 2024-2026 Cyril John Magayaga.
