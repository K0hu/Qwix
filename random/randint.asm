section .text
    global randint
    extern rand

randint:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]    ; min
    mov ebx, [ebp+12]   ; max

    call rand           ; EAX = rand() 0..32767

    mov ecx, ebx
    sub ecx, [ebp+8]    ; range = max - min
    inc ecx              ; range inklusiv

    xor edx, edx         ; EDX = 0 für div
    div ecx              ; EAX / range
                         ; EAX = quotient, EDX = remainder

    add edx, [ebp+8]     ; EDX = min + remainder
    mov eax, edx         ; return value

    pop ebp
    ret