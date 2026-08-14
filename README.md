# Nand2Tetris — Knowledge Document

## Overview

This repository documents practical work on the Nand2Tetris course and therefore the construction of a computer system across multiple abstraction layers.

The repository contains project directories `p1` through `p12` and, particularly in the later projects, demonstrates implementations of translators, compiler components, and operating system components.

## Technical Focus

Nand2Tetris combines computer architecture, machine language, assembly language, virtual machines, compiler construction, and fundamental operating system concepts.

The particular value of this project is that it does not simply build an application on top of existing frameworks. Instead, it explores and implements the abstraction layers that are normally hidden underneath application development.

## Implementations in the Repository

### Assembler

Around project 6, an implementation written in C is present.

The repository contains generated Hack machine-code files such as:

- `Add.hack`
- `Max.hack`
- `Pong.hack`
- `Rect.hack`

This part of the project deals with translating symbolic Hack assembly language into binary machine code.

### VM Translator

Projects 7 and 8 contain a VM translation implementation written in C.

The implementation includes components such as:

- Parser
- Writer
- Stack pointer logic
- Utility components

Test cases cover areas including:

- Stack arithmetic
- Push/pop operations
- Pointer and static segments
- Loops
- Functions
- Fibonacci examples

This part of the project connects a stack-based virtual machine to the underlying Hack assembly language.

### Compiler Frontend

Project 10 contains work on a tokenizer and parser, as well as generated XML representations for Jack programs such as Square and SquareGame.

This demonstrates the practical implementation of lexical analysis and syntax analysis for a higher-level programming language.

### Compiler

Project 11 extends the compiler with symbol tables and code generation.

The repository contains a `symbol_table` component and generated output for programs such as:

- Average
- ComplexArrays
- Pong
- Square

This covers variable resolution, scopes, and translation from Jack into the VM target language.

### Jack OS / Runtime Library

Project 12 contains implementations of core Jack library classes, including:

- `Array`
- `Keyboard`
- `Memory`
- `Output`
- `Screen`
- `String`
- `Sys`

Corresponding tests are also included.

This demonstrates which runtime services higher-level programs require and how those services can be implemented on top of a minimal computing platform.

## Demonstrated Skills

- C and low-level programming
- Computer architecture and machine language
- Assembly and code generation
- Stack machines and virtual machines
- Parsers and tokenizers
- Compiler construction
- Symbol tables and scope management
- Memory and runtime concepts
- Debugging across multiple abstraction layers
- Understanding of the complete path from high-level language to machine code

## Project Classification

This is primarily a computer science and educational project rather than a traditional product application.

It is particularly relevant for a technical profile because it demonstrates knowledge of fundamental concepts that are often hidden by modern frameworks and high-level development environments.

## Useful Retrieval Questions

- What experience does the developer have with compiler construction?
- Has the developer worked with C and low-level programming?
- How deep is the developer's understanding of computer architecture?
- Has the developer implemented a virtual machine or VM translator?
- What experience does the developer have with parsers, tokenizers, and symbol tables?
- Has the developer implemented operating system or runtime components?

## Source

GitHub Repository: moriHe/nand2tetris
