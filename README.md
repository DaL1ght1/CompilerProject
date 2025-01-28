# CompilerProject
This project is a compiler implementation for a custom programming language. The compiler is designed to parse, analyze, and generate intermediate code for a specific grammar. Below is an overview of the project, its features, and instructions on how to use it.

---

## Table of Contents
1. [Project Overview](#project-overview)
2. [Grammar](#grammar)
3. [Features](#features)
4. [How to Use](#how-to-use)
5. [Testing](#testing)
6. [License](#license)

---

## Project Overview

This compiler is designed to process a custom programming language defined by a specific grammar. It performs lexical analysis, syntax parsing, semantic analysis, and generates intermediate stack-based code. The project is implemented in C and includes functionalities such as variable declaration, arithmetic operations, conditional statements, and input/output operations.

---

## Grammar

The compiler was built for the following grammar:
```plaintext
P -->    program  id ;
         Dcl
         Begin
         Liste_inst
         End .
Dcl -->  var List_id : int ;
Liste_id --> id | id , list_id
Liste_inst --> I | I Liste_inst
I --> id := Exp ; | writeln(id) ; | readln(id) ; | if C then Liste_inst Endif
C -->Exp oprel Exp
Exp --> id | nb | EXP oparith Exp | (Exp)

Ps: 
    oprel = {+,*}
    oparith = {==,<>,<,>,<=,>=}
```
## Features

- **Lexical Analysis**: Tokenizes the input source code.
- **Syntax Parsing**: Parses the tokens according to the defined grammar.
- **Semantic Analysis**: Checks for variable declarations, initializations, and type consistency.
- **Intermediate Code Generation**: Generates stack-based intermediate code for the input program.
- **Error Handling**: Provides detailed error messages for syntax and semantic errors.
- **Symbol Table**: Maintains a table of identifiers and their attributes.
- **Intermediate Code Execution**: Simulates the execution of the generated intermediate code.

---    

# How to Use

### Prerequisites
- A C compiler (e.g., GCC)
- A text editor or IDE

### Steps to Run the Compiler
1. Clone the repository:
   ```bash
   git clone https://github.com/DaL1ght1/CompilerProject
   cd compiler-project
   ```
2. Compile the project:
   ```bash
    gcc -o compiler main.c
   ```
3.  Run the compiler with an input file:
    ```bash
    ./compiler test.txt
    ```
  Replace test.txt with the path to your input file.
  
4. View the output:
  - The compiler will display the parsed tokens, symbol table, and generated intermediate code.
  - Errors (if any) will be displayed with line numbers and descriptions.
## Testing

To ensure the compiler works as expected, you can test it using the provided `test.txt` file. This file contains sample code written in the custom language supported by the compiler. Follow the steps below to run the tests and verify the output.

---

### Steps to Test the Compiler

1. **Prepare the Input File**:
   - Ensure the `test.txt` file is in the project directory. This file should contain valid code written in the custom language. For example:
     ```plaintext
     program example;
     var x, y: int;
     begin
         x := 5;
         y := x + 10;
         writeln(y);
     end.
     ```

2. **Run the Compiler**:
   - Execute the compiler with the `test.txt` file as input:
     ```bash
     ./compiler test.txt
     ```

3. **Check the Output**:
   - The compiler will display the following outputs:
     - **Parsed Tokens**: A list of tokens extracted from the input file.
     - **Symbol Table**: A table showing all declared variables, their types, and initialization status.
     - **Intermediate Code**: The generated stack-based intermediate code.
     - **Error Messages**: If there are any syntax or semantic errors, they will be displayed with detailed descriptions.

4. **Verify the Results**:
   - Compare the output with the expected results to ensure the compiler is functioning correctly.

---

## License

This project is licensed under the **MIT License**. Below is the full text of the license:

---
For more details, see the [LICENSE](LICENSE) file in the repository.
