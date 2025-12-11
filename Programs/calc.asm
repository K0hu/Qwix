section .data
    a dd 2
    
section .text
    global _start
    extern _printf, strcmp, srand, GetTickCount, gets, _atoi, _ExitProcess@4



_start:
    mov eax, a
    add eax, 3
    jmp exit8


exit8:
    push 0
    call _ExitProcess@4