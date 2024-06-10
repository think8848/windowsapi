..\hjwasm -win64 -c -Zp8 win64_1.asm
..\hjwasm -win64 -c -Zp8 win64_2.asm
..\hjwasm -win64 -c -Zp8 win64_3.asm
..\hjwasm -win64 -c -Zp8 win64_3e.asm
..\hjwasm -win64 -c -Zp8 win64_4.asm
..\hjwasm -win64 -c -Zp8 win64_5.asm
..\hjwasm -win64 -c -Zp8 win64_5m.asm
..\hjwasm -win64 -c -Zp8 win64_5x.asm
..\hjwasm -win64 -c -Zp8 win64_6.asm
..\hjwasm -pe win64_6p.asm
..\hjwasm -win64 -c -Zp8 win64_6x.asm
..\hjwasm -pe win64_8.asm
..\hjwasm -win64 -c -Zp8 win64_9a.asm
..\hjwasm -win64 -c -Zp8 win64_9d.asm

link /subsystem:windows /entry:main /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_1.obj
link /subsystem:windows /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_2.obj kernel32.lib user32.lib
link /subsystem:windows /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_3.obj 
link /subsystem:windows /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_3e.obj 
link /subsystem:console /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_4.obj 
link /subsystem:console /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_5.obj 
link /subsystem:console /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_5m.obj 
link /subsystem:console /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_5x.obj 
link /subsystem:console /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_6.obj 
link /subsystem:console /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_6x.obj 
link /dll /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_9d.obj 
link /subsystem:console /machine:x64 /debug /Libpath:"%WINSDK%\v7.1\Lib\x64" win64_9a.obj win64_9d.lib 
