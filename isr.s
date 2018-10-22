
; ISR assembly file for lab3. Routines for reset, tick, and keyboard.
; Conor Bodily - ECEn 425


    CPU    8086
    align    2

reset_isr:
	push ax
	push bx
	push cx
	push dx
	push si
	push bp
	push es
	push ds
	sti
	call resetHandler
	cli
	mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
	out	0x20, al	; Write EOI to PIC (port 0x20)
	pop ds
	pop es
	pop bp
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	iret

tick_isr:
	push ax
	push bx
	push cx
	push dx
	push si
	push bp
	push es
	push ds
	sti
	call YKTickHandler
	cli
	mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
	out	0x20, al	; Write EOI to PIC (port 0x20)
	pop ds
	pop es
	pop bp
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	iret

keyboard_isr:
	push ax
	push bx
	push cx
	push dx
	push si
	push bp
	push es
	push ds
	sti
	call keyboardHandler
	cli
	mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
	out	0x20, al	; Write EOI to PIC (port 0x20)
	pop ds
	pop es
	pop bp
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	iret
	

	
