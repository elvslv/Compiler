.686
.model flat,stdcall
option casemap :none
include include\windows.inc
include include\kernel32.inc
include include\masm32.inc
includelib lib\masm32.lib
includelib lib\kernel32.lib
.data
	G_writebuf dd 256 dup(0)
.code
main:
	mov G_writebuf, 'hgds'
	mov G_writebuf + 4, 'hsjk'
	mov G_writebuf + 8, '''hkj'
	mov G_writebuf + 12, 'jkfg'
	mov G_writebuf + 16, 'sdkd'
	mov G_writebuf + 20, 'CBAh'
	mov G_writebuf + 24, 'djsg'
	mov G_writebuf + 28, 'hjhg'
	mov G_writebuf + 32, '?'''''
	invoke StdOut, offset G_writebuf
	mov G_writebuf, 10
	mov G_writebuf + 4, 0
	invoke StdOut, offset G_writebuf
	invoke ExitProcess, 0
end main
