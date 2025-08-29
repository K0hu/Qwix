section .bss
    hMem resd 1

section .text
    global wincopy
    extern _OpenClipboard@4
    extern _EmptyClipboard@0
    extern _SetClipboardData@8
    extern _GlobalAlloc@8
    extern _GlobalLock@4
    extern _GlobalUnlock@4
    extern _CloseClipboard@0
    extern _ExitProcess@4
    extern _strlen

wincopy:
    mov esi, [esp+4]
    push 0
    call _OpenClipboard@4

    call _EmptyClipboard@0

    push esi
    call _strlen
    add esp, 4
    add eax, 1
    push eax 
    push 2 
    call _GlobalAlloc@8
    mov dword [hMem], eax

    push dword [hMem]
    call _GlobalLock@4
    mov edi, eax

.copy_loop:
    lodsb
    stosb
    cmp al, 0
    jne .copy_loop

    push dword [hMem]
    call _GlobalUnlock@4

    push dword [hMem]
    push 1 
    call _SetClipboardData@8

    call _CloseClipboard@0

    ret 4