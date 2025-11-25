section .data
    a dd 0
    b dd 0
    c dd 3
    fmt db "Deine Zahl ist: %d", 10, 0
    fmt2 db "Deine neue Zahl: %d", 0
    
section .text
    global _start
    extern _printf, strcmp, srand, GetTickCount, gets, _atoi, _ExitProcess@4



_start:
    mov eax, [a]
    add eax, 5
    mov [a], eax
    mov eax, [a]
    mov ebx, 3
    mul ebx
    mov [b], eax
    mov eax, [b]
    mov ebx, [c]
    xor edx, edx
    div ebx
    mov [a], eax
    push dword [a]
    push fmt
    call _printf
    add esp, 8
    
    mov eax, 3
    mov eax, [a]
    mov [a], eax
    push dword [a]
    push fmt2
    call _printf
    add esp, 8
    
    jmp exit8


exit8:
    push 0
    call _ExitProcess@4