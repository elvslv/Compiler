@echo off
FOR %%f IN (*.in) DO FC %%f.out %%f.ans
pause