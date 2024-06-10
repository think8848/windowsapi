; include user32.inc
; include kernel32.inc

includelib user32.lib
includelib kernel32.lib

extern MessageBoxA:proc     ; 如果包含了user32.inc头文件，则不需要extern引入该函数
extern ExitProcess:proc     ; 如果包含了kernel32.inc头文件，则不需要extern引入该函数

.data
szText      db 'Hello World!', 0
szCaption   db 'Title', 0

.code
WinMain proc
    sub rsp, 28h

    xor r9d,  r9d
    mov r8,  offset szCaption
    mov rdx, offset szText
    xor rcx, rcx
    call MessageBoxA

    xor ecx, ecx
    call ExitProcess

    add rsp, 28h            ; 实际上执行完ExitProcess函数以后，这里不会得到执行
    ret
WinMain endp
end

; 也可以使用以下命令编译
; ml64 /c HelloWorld.asm
; link /subsystem:windows /entry:WinMain HelloWorld.obj