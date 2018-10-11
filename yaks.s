; What kernal functions need to be written in assembly??

	CPU 8086
	align 2

global asm_save_context
global asm_load_context
global asm_mutex
global asm_unmutex

asm_save_context:
	mov	si, word [YKCurrentTask]
	mov	[si], word sp
	add si, 2
	mov [si], word  0; pushing flags
	add si, 2
	mov	[si], word ax
	add si, 2
	mov	[si], word bx
	add si, 2
	mov	[si], word cx
	add si, 2
	mov	[si], word dx
	add si, 2
	mov	[si], word si
	add si, 2
	mov	[si], word di
	add si, 2
	mov	[si], word bp
	add si, 2
	mov	[si], word es
	add si, 2
	mov	[si], word ds
	sub si, 18
	mov bp, sp 		;
	mov di, [bp] 	; save ip
	mov	[si], di 	; 
	ret
	

asm_load_context:
	mov	si, word [YKRdyList]
	mov	sp, word [si]
	add si, 2
	push word [si] ; pushing old ip
	add si, 2
	push cs ;make sure to change this
	push word [si] ; pushing flags
	add si, 2
	mov	ax, word [si]
	add si, 2
	mov	bx, word [si]
	add si, 2
	mov	cx, word [si]
	add si, 2
	mov	dx, word [si]
	add si, 4
	mov	di, word [si]
	add si, 2
	mov	bp, word [si]
	add si, 2
	mov	es, word [si]
	add si, 2
	mov	ds, word [si]
	sub si, 8
	mov	si, word [si]
	iret

asm_mutex:
	cli
	ret

asm_unmutex:
	sti
	ret

	

