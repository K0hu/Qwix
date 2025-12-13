section .data
    a dq 2.2
    b dq 5.2
    c dq 0.0
    fmt db "Value: %.2f", 0
    
section .text
    global _start
    extern _printf, strcmp, srand, GetTickCount, gets, _atoi, _ExitProcess@4



_start:
    movsd xmm0, [a]
    movsd xmm1, [b]
    addsd xmm0, xmm1
    movsd [c], xmm0
    push dword [c+4]
    push dword [c]
    push fmt
    call _printf
    add esp, 12
    
    jmp exit8


exit8:push 0
    call _ExitProcess@4
