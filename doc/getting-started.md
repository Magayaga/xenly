# Getting started with Xenly

You'll need to install our build dependencies (git, gcc, clang) and check out the Xenly repository, for example on **Windows**, **Linux**, or **macOS**:

1. ## Windows operating system:

   ```bash
   # Download the Git and GCC
   $ winget install --id Git.Git -e --source winget
   $ winget install MSYS2.MSYS2
   ```

   ```bash
   # Xenly's source code
   $ git clone https://github.com/magayaga/xenly.git
   $ cd xenly
   ```

   Build and run the application
   
   ```bash
   # Windows
   $ ./make.bat
   $ bash main.sh
   ```

   To run code in a file non-interactively, you can give it as the first argument to the **Xenly** command:

   ```shell
   $ ./xenly examples/print/main.xe
   $ ./xenly examples/math/operations.xe
   ```

   You can pass additional arguments to **Xenly**, and to your program `main.xe` or `operations.xe`.

2. ## Linux or macOS operating systems:

   ```bash
   # Download the Git and GCC
   $ sudo apt-get install git gcc

   # or Download the Git and Clang
   $ sudo apt-get install git clang
   ```

   ```bash
   # Xenly's source code
   $ git clone https://github.com/magayaga/xenly.git
   $ cd xenly
   ```

   Build and run the application
   
   ```bash
   # Linux or macOS
   $ make
   $ bash main.sh
   ```

   To run code in a file non-interactively, you can give it as the first argument to the **Xenly** command:

   ```shell
   $ ./xenly examples/print/main.xe
   $ ./xenly examples/math/operations.xe
   ```

   You can pass additional arguments to **Xenly**, and to your program `main.xe` or `operations.xe`.

## Copyright

Copyright (c) 2023-2024 [Cyril John Magayaga](https://github.com/magayaga). All rights reserved.
