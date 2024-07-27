# Comments in Xenly

Comments can be used to explain code, and to make it more readable. It can also be used to prevent execution when testing alternative code.

Comments can be **singled-lined** or **multi-lined**.

## Example code

### Single-line comments
Single-line comments start with two forward slashes (`//`). Any text between `//` and the end of the line is ignored by the compiler (will not be executed). This example uses a single-line comment before a line of code:

```swift
// This is singled-lined
nota("Hello, World")
```

### Multi-line comments
Multi-line comments start with `/*` and ends with `*/`. Any text between `/*` and `*/` will be ignored by the compiler:

```swift
/*
  This is multi-lined
  "Hello, World" program
*/
nota("Hello, World!")
```


On June 13, 2024, [@magayaga](https://github.com/magayaga) announced that multi-line comments was not working for Xenly's `nanopreview` version now.

On July 22, 2024, @magayaga announced that multi-line comments was working for Xenly's `preview` version now.

## Copyright

Copyright (c) 2023-2024 [Cyril John Magayaga](https://github.com/magayaga). All rights reserved.
