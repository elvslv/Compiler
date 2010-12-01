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
	mov G_writebuf, 'dtrt'
	mov G_writebuf + 4, 'hfjs'
	mov G_writebuf + 8, 'df''j'
	mov G_writebuf + 12, 'kgja'
	mov G_writebuf + 16, '''''g'
	invoke StdOut, offset G_writebuf
	mov G_writebuf, 10
	mov G_writebuf + 4, 0
	invoke StdOut, offset G_writebuf
	invoke ExitProcess, 0
end main
