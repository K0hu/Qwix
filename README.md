# Qwix

### Table of content:
1. [What is Qwix?](#whatisqwix?)
2. [Declaring variables](#variables)
3. [Operations](#operations)
   1. [Tinyexpr](#tinyexpr)
   2. [Math statements](#mathstatements)
4. [Statements](#statements)
5. [Functions](#functions)
   1. [Jumps](#jumps)
   2. [Call](#call)
   3. [Return](#return)
   4. [Args](#args)
6. [Builtins](#builtins)
   1. [Print](#print)
   2. [Atoi](#atoi)
   3. [Include](#include)
   4. [Prompt](#prompt)
   5. [(Random)](#randint)
7. [Assembly](#asmembly)
8. [Compiler]()

## What is Qwix?
Qwix is ​​a minimalist (low level) programming language implemented in C. 
Code in qwix is compiled to NASM code (then with the NASM and GoLink compiled to an executeble).

## Variables
```|var 0-9/"0-9"```: Allocation in the .bss segment. With "" can be strings reserved. <br>
```var. {}/0-9/""/0.9```: Declarations of variables (.data). With ```{}``` [tinyexpr](#tinyexpr) is used. 
To declare number/doubles or strings you can simply use "", numbers (and ''). <br>
```define var```: Equal to %define.

## Operations
- #### Tinyexpr
  [TinyExpr](https://github.com/codeplea/tinyexpr) is "a very small recursive descent parser and evaluation engine for math expressions", which is also used in Qwix.
  The ```{}``` are the identifier for TinyExpr. (Not on runtime)

- ### Math statements
  With ```add```, ```div```, ```sub``` and ```mul``` you can calculate on runtime.
  With an ```dq``` before the statements you can calculate with doubles (only then).

## Statements
```If``` or ```else``` (and so on) do not exist in Qwix. You can compare to values (with ```%```) or strings (with ```&```). After the compare an jump can happend with ```?``` (if true), ```!``` (if not), ```>``` (if greater), ```<``` (if less). You must after that call the [jump](#jumps). <br>
For example an ```if (a == b) {printf(msg)}``` is euqal to: <br>
```Assembly
jump.:
  print(msg)

a%b ?jump
```
