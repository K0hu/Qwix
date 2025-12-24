section .data
    event times 24 dq 0
    
section .text
    global main
    extern printf, atoi, fgets, time, stdin, XOpenDisplay, XCreateSimpleWindow, XMapWindow, XNextEvent, XCloseDisplay, XDefaultScreen, XRootWindow, exit



main:
    sub rsp, 8
    xor rdi, rdi
    call XOpenDisplay
    test rax, rax
    je exit8
    mov r12, rax
    mov rdi, r12
    call XDefaultScreen
    mov r13d, eax
    mov rdi, r12
    mov esi, r13d
    call XRootWindow
    mov r14, rax
    sub rsp, 24
    mov qword [rsp], 1
    mov qword [rsp+8], 0
    mov qword [rsp+16], 0
    mov rdi, r12
    mov rsi, r14
    mov rdx, 100
    mov rcx, 100
    mov r8, 400
    mov r9, 300
    call XCreateSimpleWindow
    add rsp, 24
    mov r15, rax
    mov rdi, r12
    mov rsi, r15
    call XMapWindow
    jmp loop
    jmp exit8


loop:
    mov rdi, r12
    lea rsi, [event]
    call XNextEvent
    jmp loop
    
exit8:
    xor edi, edi
    call exit
section .note.GNU-stack noalloc noexec nowrite progbits
