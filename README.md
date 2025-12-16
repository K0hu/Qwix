# Qwix

### Table of content:
1. [What is Qwix?](#whatisqwix?)
2. [Declaring variables](#variables)
3. [Operations](#operations)
   1. [Tinyexpr](#tinyexpr)
   2. [Math statements](#math)
4. [Statements](#statements)
5. [Functions](#functions)
   1. [Jumps](#jumps)
   2. [Push](#push)
   3. [Return](#return)
   4. [Args](#args)
6. [Builtins](#builtins)
   1. [Print](#print)
   2. [Atoi](#atoi)
   3. [Include](#include)
   4. [Prompt](#prompt)
   5. [(Random)](#randint)
7. [Assembly](#asmembly)
8. [Compiler](#compiler)

## What is Qwix?
Qwix is ​​a minimalist (low level) programming language implemented in C. 
Code in qwix is compiled to NASM code (then with the NASM and GoLink compiled to an executeble/elf).

## Variables
- ```|var 0-9/"0-9"```: Allocation in the .bss segment. With "" can be strings reserved. <br>
- ```var. {}/0-9/""/0.9/[]```: Declarations of variables (.data). With ```{}``` [tinyexpr](#tinyexpr) is used. 
To declare number/doubles or strings you can simply use "", numbers (and ''). To declare an array ```[]```, strings can not directly defined in an array, but the variables of them can be refrenced in the array. To get an element of an array: ```(array+4*index)``` <br>
- ```define var```: Equal to %define. 

> [!NOTE]
> Strings mostly not called direct, its recommend to use variables with strings.

## Operations
- ### Tinyexpr
   [TinyExpr](https://github.com/codeplea/tinyexpr) is "a very small recursive descent parser and evaluation engine for math expressions", which is also used in Qwix.
   The ```{}``` are the identifier for TinyExpr. (Not on runtime)

- ### Math
   With ```add```, ```div```, ```sub``` and ```mul``` you can calculate on runtime. With an ```. var``` at the end you can load an var with the result.
   With an ```dq``` before the operations you can calculate with doubles (only then).
> With ```var++``` you can increase the variable (+1).

## Statements
```If``` or ```else``` (and so on) do not exist in Qwix. You can compare to values (with ```%```) or strings (with ```&```). After the compare an jump can happend with ```?``` (if true), ```!``` (if not), ```>``` (if greater), ```<``` (if less). You must after that call the [jump](#jumps). <br>
For example an ```if (a == b) {printf(msg)}``` is euqal to: <br>
```Assembly
jump.:
  print(msg)

a%b ?jump
```

## Functions
- ### Jumps
    Jumps are very important, they can be used and must be used for a lot. They are declared with ```name.:``` (.:) and to jump is an ```:name``` or called (for functions with return) with ```@name```.
- ### Push
   The push is used to give an function variables. ```+name``` is used to do that. To push variables with numbers you use ```+#(name)```. The hashtag marks an dword (number).
   Additional you can push values with ```[]```, but ```#(num)``` can not be used
- ### Return
    An return is marked as ```/``` it can have 2 args, the buffer clear value and the variable to return (```mov eax, var```: move the variable to the regester eax). Return should only be used when an function is called (```::name```).
- ### Args
   Args are the parameter that are pushed. In the function (```function.: ... ::function```) the args are refrenced with $num of the pushed value.
> [!NOTE]
> The last pushed parameter is the first in the list (so its refrenced as ```$1```).
  
## Builtins
- ### Print
    Print is used to use the console output. (```print args*```)
- ### Atoi
    ```atoi var string```, is used to refrence  ```atoi``` the first parameter is the var to load and the string is (also a variable) to be converted to an number.
- ### Include
    ```include lib``` includes extern programs. To include local programs use, ```include [path]```.
- ### Prompt
    The prompt command is a user input in the console. (```prompt var msg```)
- ### Randint
    Randint has to options on runtime or not on runtime. With the include of the lib (```include [lib/include]```) randint functions on runtime, when not included it works but not on runtime. ```randint min max```

> [!NOTE]
> With an ```*``` you can end the program.

# Assembly
How in the introduction mentioned qwix is based on nasm/assembly. 
Assembly code can be written direct in qwix with ```~asm code... (~)``` this can be done (its optional to close is). 
- ```mov```, ```try```, ```xor```: can be used like in asm but without the ```,```.
> mov can also used with an equal symbol (```par = par```) 

# Compiler
The compiler uses Nasm and GoLink to make it an executeble. It has different options:
- ```qwix.exe file.qwr```: Execute qwix code (makes executeble in the tmp folder).
- ```-nc```: No console
- ```-o```: Output an .exe file
- ```-asm```: Output an .asm file
- ```-nw```: Disable output of the warnings
- ```-gop```: GoLink output enable

---

> [!IMPORTANT]
> On this page is ```var```, ```name```, ```num```, ```msg```, ```args*``` (and so on) only placeholder and refrence to variables (```var```, ```msg```) or numbers (```num```)
