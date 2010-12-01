@echo off

if exist %1.obj del %1.obj

\masm32\bin\ml /c /coff %1.asm