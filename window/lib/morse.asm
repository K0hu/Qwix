section .data
    a   db ".-", 0
    b   db "-...", 0
    c   db "-.-.", 0
    d   db "-..", 0
    e   db ".", 0
    f   db "..-.", 0
    g   db "--.", 0
    h   db "....", 0
    i   db "..", 0
    j   db ".---", 0
    k   db "-.-", 0
    l   db ".-..", 0
    m   db "--", 0
    n   db "-.", 0
    o   db "---", 0
    p   db ".--.", 0
    q   db "--.-", 0
    r   db ".-.", 0
    s   db "...", 0
    t   db "-", 0
    u   db "..-", 0
    v   db "...-", 0
    w   db ".--", 0
    x   db "-..-", 0
    y   db "-.--", 0
    z   db "--..", 0
    space_code db "/", 0
    next    db " ", 0          ; separator between letters

    ; table of dword pointers to the above code strings
    codes   dd a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z

section .bss
    buffer resb 512       ; Output-Buffer

section .text
    global morse

; concat: append source (ESI) to end of dest (EDI)
; ESI = source (null-terminated), EDI = destination (null-terminated or zero to start)
; Clobbers AL; preserves nothing else (caller must save registers it needs).
concat:
    push eax                ; save EAX (we'll use AL)
find_end:
    mov al, [edi]
    cmp al, 0
    je copy_source
    inc edi
    jmp find_end
copy_source:
    mov al, [esi]
    cmp al, 0
    je done
    mov [edi], al
    inc edi
    inc esi
    jmp copy_source
done:
    mov byte [edi], 0
    pop eax
    ret

; morse: convert input string -> morse buffer
; param: [esp+4] = pointer to input string (null-terminated)
; returns: EAX = pointer to buffer
morse:
    push ebp
    mov ebp, esp

    mov esi, [ebp+8]        ; ESI = input pointer
    lea edi, [buffer]       ; EDI = output buffer (destination)
    mov dword [edi], 0      ; ensure buffer starts zero (optional)
    ; actually set EDI to buffer start address
    lea edi, [buffer]

.loop_start:
    mov al, [esi]
    cmp al, 0
    je .morse_exit

    ; handle space
    cmp al, ' '
    je .handle_space

    ; convert uppercase to lowercase if needed
    cmp al, 'A'
    jl .check_letter
    cmp al, 'Z'
    jg .check_letter
    add al, 32              ; now lowercase

.check_letter:
    cmp al, 'a'
    jb .skip_char
    cmp al, 'z'
    ja .skip_char

    ; compute index = al - 'a'
    movzx eax, al
    sub eax, 'a'            ; eax = 0..25
    ; load pointer to morse string into ecx
    mov ecx, [codes + eax*4]

    ; call concat to append the morse code (we must temporarily set ESI = source)
    push esi                ; save input pointer (we will restore after concat)
    mov esi, ecx            ; ESI = source morse string pointer
    call concat             ; concat uses ESI (source) and EDI (dest)
    pop esi                 ; restore input pointer

    ; append separator "next"
    push esi
    lea esi, [next]
    call concat
    pop esi

    jmp .advance

.handle_space:
    ; append "/" and separator (or just "/ ")
    push esi
    lea esi, [space_code]
    call concat
    pop esi

    ; append separator
    push esi
    lea esi, [next]
    call concat
    pop esi

    jmp .advance

.skip_char:

.advance:
    inc esi                 ; advance input pointer
    jmp .loop_start

.morse_exit:
    lea eax, [buffer]       ; return pointer to buffer
    pop ebp
    ret