section .data
    d dd 0, 1, 2
    fmt db "Deine Zahl ist: %d", 10, 0
    
section .text
    global _start
    extern _printf, strcmp, srand, GetTickCount, gets, _atoi, _ExitProcess@4



_start:
    push dword [d+2*4]
    push dword [d+1*4]
    call func
    mov [d+0*4], eax
    push dword [d + 0*4]
    push fmt
    call _printf
    add esp, 8
    
    jmp exit8


func:
    mov eax, [esp+8]
    add eax, 5
    mov [esp+8], eax
    mov eax, [esp+8]
    ret 8
    
exit8:
    push 0
    call _ExitProcess@4