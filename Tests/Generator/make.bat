FOR %%f IN (*.asm) DO (
bin\ml /c /coff %%f
bin\link /SUBSYSTEM:CONSOLE %%f.obj)
pause