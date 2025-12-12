section .data
    className db "WindowClass", 0
    windowTitle db "You shall not pass", 0
    buttonClass db "BUTTON", 0
    buttonText db "Enter", 0
    editClass db "EDIT", 0
    iconPath db "sources\\window.ico", 0
    cursorPath db "sources\\Gandalf.cur", 0
    password db "qwix", 0
    secretFmt db "Magicnumber: %d", 0
    magicNum dd 0
    errTitle db "Error", 0
    errFmt db "[Error] Code: %d", 0
    
section .bss
    wc resb 48
    msg resb 28
    hEdit resd 1
    editMsg resd 128
    secretMsg resd 64
    errCode resd 1
    errStr resb 12
    errMsg resd 64
    
section .text
    global _start
    extern _printf, strcmp, srand, GetTickCount, gets, _atoi, _GetModuleHandleA@4, _RegisterClassExA@4, _CreateWindowExA@48, _ShowWindow@8, _UpdateWindow@4, _DefWindowProcA@16, _GetMessageA@16, _TranslateMessage@4, _DispatchMessageA@4, _PostQuitMessage@4, _MessageBoxA@16, _LoadIconA@8, _LoadCursorA@8, _LoadImageA@24, _GetLastError@0, _SendMessageA@16, _sprintf, randint, morse, wincopy, _ExitProcess@4


%define WM_COMMAND 0x0111
%define WM_DESTROY 0x0002
%define WS_OVERLAPPEDWINDOW 0x00CF0000
%define WS_BORDER 0x00800000
%define ES_LEFT 0x0000
%define SW_SHOWNORMAL 1
%define BUTTON_ID 1001
%define WS_CHILD 0x40000000
%define WS_VISIBLE 0x10000000
%define BS_PUSHBUTTON 0x00000000
%define WM_GETTEXT 0x000D

_start:
    push 0
    call GetTickCount
    add esp, 4
    push eax
    call srand
    add esp, 4
    push 0
    call _GetModuleHandleA@4
    mov esi, eax
    mov dword [wc], 48
    mov dword [wc+4], 0
    mov dword [wc+8], wndProc
    mov dword [wc+12], 0
    mov dword [wc+16], 0
    mov [wc+20], eax
    push 0x10
    push 32
    push 32
    push 1
    push iconPath
    push 0
    call _LoadImageA@24
    test eax, eax
    je checkE
    mov dword [wc+24], eax
    mov dword [wc+44], eax
    push 0x10
    push 32
    push 32
    push 2
    push cursorPath
    push 0
    call _LoadImageA@24
    test eax, eax
    je checkE
    mov dword [wc+28], eax
    mov dword [wc+32], 6
    mov dword [wc+36], 0
    mov dword [wc+40], className
    push wc
    call _RegisterClassExA@4
    test eax, eax
    je checkE
    push 0
    push esi
    push 0
    push 0
    push 300
    push 400
    push 100
    push 100
    push WS_OVERLAPPEDWINDOW
    push windowTitle
    push className
    push 0
    call _CreateWindowExA@48
    mov ebx, eax
    push SW_SHOWNORMAL
    push ebx
    call _ShowWindow@8
    push ebx
    call _UpdateWindow@4
    push 0
    push esi
    push BUTTON_ID
    push ebx
    push 50
    push 50
    push 100
    push 100
    push WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON
    push buttonText
    push buttonClass
    push 0
    call _CreateWindowExA@48
    push 0
    push esi
    push 0
    push ebx
    push 20
    push 200
    push 50
    push 50
    push WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT
    push 0
    push editClass
    push 0
    call _CreateWindowExA@48
    mov [hEdit], eax
    jmp msgloop
    jmp exit8


msgloop:
    push 0
    push 0
    push 0
    push msg
    call _GetMessageA@16
    test eax, eax
    je checkE
    push msg
    call _TranslateMessage@4
    push msg
    call _DispatchMessageA@4
    jmp msgloop
    
wndProc:
    mov eax, [esp+8]
    cmp eax, WM_COMMAND
    je event
    cmp eax, WM_DESTROY
    jne defproc
    push 0
    call _PostQuitMessage@4
    xor eax, eax
    ret 16
    
event:
    mov eax, [esp+12]
    cmp eax, BUTTON_ID
    je buttonCmd
    jne defproc
    xor eax, eax
    ret 16
    
buttonCmd:
    push editMsg
    push 128
    push WM_GETTEXT
    push dword [hEdit]
    call _SendMessageA@16
    push editMsg
    push password
    call strcmp
    add esp, 8
    cmp eax, 0
    je secret
    push editMsg
    call morse
    add esp, 4
    mov ebx, eax
    push 0
    push windowTitle
    push ebx
    push 0
    call _MessageBoxA@16
    push ebx
    call wincopy
    xor eax, eax
    ret 16
    
secret:
    push 10
    push 1
    call randint
    add esp, 8
    mov [magicNum], eax
    push dword [magicNum]
    push secretFmt
    push secretMsg
    call _sprintf
    add esp, 12
    push 0
    push windowTitle
    push secretMsg
    push 0
    call _MessageBoxA@16
    
defproc:
    mov eax, [esp+4]
    mov ecx, [esp+8]
    mov edx, [esp+12]
    mov ebx, [esp+16]
    push ebx
    push edx
    push ecx
    push eax
    call _DefWindowProcA@16
    ret 16
    
checkE:
    call _GetLastError@0
    mov [errCode], eax
    cmp eax, 0
    jne error
    jmp exit8
    
error:
    push dword [errCode]
    push errFmt
    call _printf
    add esp, 8
    
    push dword [errCode]
    push  errFmt
    push  errMsg
    call _sprintf
    add esp, 12
    push 0x00000010
    push errTitle
    push errMsg
    push 0
    call _MessageBoxA@16
    jmp exit8
    
exit8:push 0
    call _ExitProcess@4
