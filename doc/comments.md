# Comments in Xenly

Comments can be used to explain code, and to make it more readable. It can also be used to prevent execution when testing alternative code.

Comments can be **singled-lined** or **multi-lined**.

## Example code

### Single-line comments
Single-line comments start with two forward slashes (`//`). Any text between `//` and the end of the line is ignored by the compiler (will not be executed). This example uses a single-line comment before a line of code:

```swift
// This is singled-lined
print("Hello, World")
```

### Multi-line comments
Multi-line comments start with `/*` and ends with `*/`. Any text between `/*` and `*/` will be ignored by the compiler:

```swift
/*
  This is multi-lined
  "Hello, World" program
*/
print("Hello, World!")
```
