@echo off
rem assembles all samples (no link!)
..\HJWasm32 -nologo -bin -Fo Dos1.com Dos1.asm
..\HJWasm32 -nologo -mz Dos2.asm
..\HJWasm32 -nologo -mz Dos3.asm
..\HJWasm32 -nologo -mz DOS64.asm
..\HJWasm32 -nologo -elf gtk01.asm
..\HJWasm32 -nologo -coff html2txt.asm
..\HJWasm32 -nologo -coff jfc.asm
..\HJWasm32 -nologo -elf64 -Fo=Lin64_1.o Lin64_1.asm
..\HJWasm32 -nologo -Fo=Linux1.o Linux1.asm
..\HJWasm32 -nologo -elf -zcw -Fo=Linux2.o Linux2.asm
..\HJWasm32 -nologo -elf -Fo=Linux3.o Linux3.asm
..\HJWasm32 -nologo -elf -Fo=Linux4a.o Linux4a.asm
..\HJWasm32 -nologo -elf -Fo=Linux4d.o Linux4d.asm
..\HJWasm32 -nologo -q -bin -Fo=Linux5. Linux5.asm
..\HJWasm32 -nologo -coff masm2htm.asm
..\HJWasm32 -nologo -coff bin2inc.asm
..\HJWasm32 -nologo -coff res2inc.asm
..\HJWasm32 -nologo -coff Math1.asm
..\HJWasm32 -nologo Mixed116.asm
..\HJWasm32 -nologo -coff Mixed132.asm
..\HJWasm32 -nologo Mixed216.asm
..\HJWasm32 -nologo -coff Mixed232.asm
..\HJWasm32 -nologo -elf -zcw -Fo ncurs1.o ncurs1.asm
..\HJWasm32 -nologo os216.asm
..\HJWasm32 -nologo os232.asm
..\HJWasm32 -nologo Win16_1.asm
..\HJWasm32 -nologo Win16_2d.asm
..\HJWasm32 -nologo Win32_1.asm
if exist "..\WinInc" ..\HJWasm32 -nologo Win32_2.asm
if exist "..\Masm32" ..\HJWasm32 -nologo -coff Win32_3.asm
..\HJWasm32 -nologo -coff Win32_4a.asm
..\HJWasm32 -nologo -coff Win32_4d.asm
..\HJWasm32 -nologo -bin -Fo Win32_5.exe Win32_5.asm
..\HJWasm32 -nologo -coff -DUNICODE Win32_6.asm
if exist "..\WinInc" ..\HJWasm32 -nologo -coff -DUNICODE Win32_6w.asm
..\HJWasm32 -nologo -coff -Fd Win32_7.asm
..\HJWasm32 -nologo -pe Win32_8.asm
if exist "..\Masm32" ..\HJWasm32 -nologo -pe Win32_8m.asm
..\HJWasm32 -nologo -coff Win32Drv.asm
if exist "..\WinInc" ..\HJWasm32 -nologo -coff Win32DrvA.asm
..\HJWasm32 -nologo -coff JWasmDyn.asm
..\HJWasm32 -nologo -coff Win32Tls.asm
..\HJWasm32 -nologo -coff ComDat.asm
..\HJWasm32 -nologo -zf1 owfc16.asm
..\HJWasm32 -nologo -zf1 owfc32.asm
..\HJWasm32 -nologo -win64 Win64_1.asm
..\HJWasm32 -nologo -win64 Win64_2.asm
..\HJWasm32 -nologo -win64 Win64_3.asm
..\HJWasm32 -nologo -win64 Win64_3e.asm
if exist "..\WinInc" ..\HJWasm32 -nologo -win64 -Zp8 -I\WinInc\Include Win64_4.asm
if exist "..\WinInc" ..\HJWasm32 -nologo -win64 Win64_5.asm
..\HJWasm32 -nologo -win64 Win64_6.asm
..\HJWasm32 -nologo -pe Win64_8.asm
..\HJWasm32 -nologo -win64 Win64_9a.asm
..\HJWasm32 -nologo -win64 Win64_9d.asm
if exist "..\WinInc" ..\HJWasm32 -nologo -coff -I\WinInc\Include WinXX_1.asm
