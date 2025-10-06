# Systemsoftwaretask
# SimpleLang Compiler Documentation

## Overview
This is a complete compiler implementation for SimpleLang that translates high-level code into assembly for an 8-bit CPU.

## Compiler Components

### 1. **Lexer (Tokenizer)**
- Breaks source code into tokens
- Recognizes keywords: `int`, `if`
- Identifies operators: `+`, `-`, `=`, `==`
- Handles numbers and identifiers
- Skips whitespace and comments (`//`)

### 2. **Parser**
- Builds an Abstract Syntax Tree (AST)
- Implements recursive descent parsing
- Handles:
  - Variable declarations
  - Assignments
  - Arithmetic expressions
  - If statements with equality conditions

### 3. **Code Generator**
- Traverses the AST
- Generates assembly code for the 8-bit CPU
- Uses registers R0 and R1 for computations
- Manages variable storage in memory

## How to Compile and Run

### Step 1: Compile the Compiler
```bash
gcc -o simplelang_compiler simplelang_compiler.c
```

### Step 2: Create a SimpleLang Program
Save this as `test.sl`:

```c
// Variable declarations
int a;
int b;
int c;

// Assignments
a = 10;
b = 20;
c = a + b;

// Conditional
if (c == 30) {
    c = c + 1;
}
```

### Step 3: Run the Compiler
```bash
./simplelang_compiler test.sl output.asm
```

## Example Output (Assembly Code)

The compiler generates assembly like this:

```asm
; SimpleLang compiled code for 8-bit CPU
; Variable memory starts at address 0

; Declare variable: a
; Declare variable: b
; Declare variable: c

; Assignment: a = ...
    LDI R0, 10       ; Load immediate 10
    ST R0, [0]       ; Store to variable a

; Assignment: b = ...
    LDI R0, 20       ; Load immediate 20
    ST R0, [1]       ; Store to variable b

; Assignment: c = ...
    LD R0, [0]       ; Load variable a
    PUSH R0          ; Save left operand
    LD R0, [1]       ; Load variable b
    MOV R1, R0       ; Move right to R1
    POP R0           ; Restore left operand
    ADD R0, R1       ; Add R0 + R1
    ST R0, [2]       ; Store to variable c

; If statement
    LD R0, [2]       ; Load variable c
    PUSH R0          ; Save left side
    LDI R0, 30       ; Load immediate 30
    MOV R1, R0       ; Move right to R1
    POP R0           ; Restore left side
    CMP R0, R1       ; Compare
    JNE skip_...     ; Jump if not equal

; Assignment: c = ...
    LD R0, [2]       ; Load variable c
    PUSH R0          ; Save left operand
    LDI R0, 1        ; Load immediate 1
    MOV R1, R0       ; Move right to R1
    POP R0           ; Restore left operand
    ADD R0, R1       ; Add R0 + R1
    ST R0, [2]       ; Store to variable c
skip_...:

    HLT              ; Halt execution
```

## Language Features

### Supported Constructs
- ✅ Variable declarations (`int x;`)
- ✅ Assignment statements (`x = 5;`)
- ✅ Arithmetic operations (`+`, `-`)
- ✅ If statements with equality (`if (x == y) { ... }`)
- ✅ Comments (`// comment`)

### Limitations
- ❌ No loops (as per specification)
- ❌ No function definitions
- ❌ No multiplication/division
- ❌ Only `==` comparison operator
- ❌ No else clauses
- ❌ Variables limited to single letters (a-z)

## Assembly Instruction Set Used

| Instruction | Description |
|------------|-------------|
| `LDI R0, val` | Load immediate value into R0 |
| `LD R0, [addr]` | Load from memory address into R0 |
| `ST R0, [addr]` | Store R0 to memory address |
| `MOV R1, R0` | Move value from R0 to R1 |
| `ADD R0, R1` | Add R1 to R0 |
| `SUB R0, R1` | Subtract R1 from R0 |
| `CMP R0, R1` | Compare R0 and R1 (sets flags) |
| `JNE label` | Jump if not equal |
| `PUSH R0` | Push R0 onto stack |
| `POP R0` | Pop from stack into R0 |
| `HLT` | Halt execution |

## Testing Strategy

1. **Basic Assignment**: Test simple variable assignments
2. **Arithmetic**: Test addition and subtraction
3. **Conditionals**: Test if statements with true and false conditions
4. **Complex Expressions**: Test nested operations like `a = b + c - d;`

## Future Enhancements

For interns looking to extend this compiler:
- Add multiplication and division operators
- Support more comparison operators (`<`, `>`, `!=`, etc.)
- Add while/for loops
- Implement else clauses
- Add support for arrays
- Optimize generated code (reduce redundant loads/stores)
- Add error recovery in parser
- Implement symbol table for better variable management

## Common Errors and Solutions

**Error: "Expected semicolon"**
- Make sure every statement ends with `;`

**Error: "Expected identifier"**
- Check variable names (must start with a letter)

**Error: "Only '==' supported in conditions"**
- Currently, if statements only support equality checks

**Segmentation fault**
- Check that your SimpleLang program has proper syntax
- Ensure all variables are declared before use
