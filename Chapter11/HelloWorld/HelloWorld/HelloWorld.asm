; include user32.inc
; include kernel32.inc

includelib user32.lib
includelib kernel32.lib

extern MessageBoxA:proc     ; ���������user32.incͷ�ļ�������Ҫextern����ú���
extern ExitProcess:proc     ; ���������kernel32.incͷ�ļ�������Ҫextern����ú���

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

    add rsp, 28h            ; ʵ����ִ����ExitProcess�����Ժ����ﲻ��õ�ִ��
    ret
WinMain endp
end

; Ҳ����ʹ�������������
; ml64 /c HelloWorld.asm
; link /subsystem:windows /entry:WinMain HelloWorld.obj