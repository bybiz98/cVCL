
.CODE

X64WndProcTemplateStart:
	;sub rsp, 20h
	mov rcx, 1122334455667788h			;overwrite the first arg HWND as object address[this pointer]
X64ThisLabel:
	mov rax, 1122334455667788h			;obtain the method address
X64OffsetLabel:
	jmp rax								;and jmp to method, the method must has 3 arg Msg, wParam, lParam and return value LRESULT
	;add rsp, 20h
	;ret ;0xc3
X64WndProcTemplateEnd:
	nop

X64WndProcTemplateStart1:
	push r9 ;0x5141
	mov r9, r8 ;0xc88b4d
	mov r8, rdx ;0xc28b4c
	mov rdx, rcx ;0xd18b48
	sub rsp, 20h ;0x20ec8348
	mov rcx, 1122334455667788h ;0xB948 0x8877665544332211
X64ThisLabel1:
	mov rax, 1122334455667788h ;0xB848 0x8877665544332211
X64OffsetLabel1:
	call rax
	add rsp, 28h ;0x28c48348
	ret ;0xc3
X64WndProcTemplateEnd1:
	nop

objWndProcX64StartAddr PROC
	lea rax, X64WndProcTemplateStart
	ret
objWndProcX64StartAddr ENDP

objWndProcX64Length proc
	push rbx
	lea rax, X64WndProcTemplateEnd
	lea rbx, X64WndProcTemplateStart
	sub rax, rbx
	pop rbx
	ret
objWndProcX64Length endp

objWndProcX64ThisAddr proc
	lea rax, X64ThisLabel
	sub rax, 8
	ret
objWndProcX64ThisAddr endp

objWndProcX64OffsetAddr proc
	lea rax, X64OffsetLabel
	sub rax, 8
	ret
objWndProcX64OffsetAddr endp

end