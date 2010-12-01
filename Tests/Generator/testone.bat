..\..\Debug\compiler.exe %1.in -gen
bin\ml /c /coff %1.asm
bin\link /SUBSYSTEM:CONSOLE %1.obj
%1 > %1.out
fc %1.ans %1.out >> report.txt
