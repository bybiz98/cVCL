
.386  
.MODEL FLAT
.CODE

X86WndProcTemplateStart:
	pop eax				;save return address
	add esp, 4			;pop first arg HWND
	push eax			;push return address for return from object method
	mov ecx, 11223344h	;assign object address to ecx, for this pointer in method body
X86ThisLabel:
	mov eax, 11223344h	;obtain method address
X86OffsetLabel:
	jmp eax				;and jmp to method address; method must has 3 args Msg, wParam, lParam and return value LRESULT
X86WndProcTemplateEnd:
	nop

X86WndProcTemplateStart1:
	mov ecx, 11223344h
X86ThisLabel1:
	mov eax, 11223344h
X86OffsetLabel1:
	jmp eax
X86WndProcTemplateEnd1:
	nop

_objWndProcX86StartAddr proc
	lea eax, X86WndProcTemplateStart
	ret
_objWndProcX86StartAddr endp

_objWndProcX86Length proc
	push ebx
	lea eax, X86WndProcTemplateEnd
	lea ebx, X86WndProcTemplateStart
	sub eax, ebx
	pop ebx
	ret
_objWndProcX86Length endp

_objWndProcX86ThisAddr proc
	lea eax, X86ThisLabel
	sub eax, 4
	ret
_objWndProcX86ThisAddr endp

_objWndProcX86OffsetAddr proc
	lea eax, X86OffsetLabel
	sub eax, 4
	ret
_objWndProcX86OffsetAddr endp

end
