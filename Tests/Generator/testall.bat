if exist report.txt del report.txt
for /l %%i in (1,1,9) do call testone 00%%i
for /l %%i in (10,1,25) do call testone 0%%i
pause