
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
	push di
	push bp
	push es
	push ds
	call YKEnterISR
	sti
	call resetHandler
	cli
	mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
	out	0x20, al	; Write EOI to PIC (port 0x20)
	call YKExitISR
	pop ds
	pop es
	pop bp
	pop di
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
	push di
	push bp
	push es
	push ds
	call YKEnterISR
	sti
	call YKTickHandler
  	call tickHandler
	cli
	mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
	out	0x20, al	; Write EOI to PIC (port 0x20)
	call YKExitISR
	pop ds
	pop es
	pop bp
	pop di
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
	push di
	push bp
	push es
	push ds
	call YKEnterISR
	sti
	call keyboardHandler
	cli
	mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
	out	0x20, al	; Write EOI to PIC (port 0x20)
	call YKExitISR
	pop ds
	pop es
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	iret

game_over:
  push ax
  mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
  out	0x20, al	; Write EOI to PIC (port 0x20)
  pop ax
  iret

new_piece:
  push ax
  push bx
  push cx
  push dx
  push si
  push di
  push bp
  push es
  push ds
  call YKEnterISR
  sti
  call newPieceHandler
  cli
  mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
  out	0x20, al	; Write EOI to PIC (port 0x20)
  call YKExitISR
  pop ds
  pop es
  pop bp
  pop di
  pop si
  pop dx
  pop cx
  pop bx
  pop ax
  iret

received_command:
  push ax
  push bx
  push cx
  push dx
  push si
  push di
  push bp
  push es
  push ds
  call YKEnterISR
  sti
  call receivedCommandHandler
  cli
  mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
  out	0x20, al	; Write EOI to PIC (port 0x20)
  call YKExitISR
  pop ds
  pop es
  pop bp
  pop di
  pop si
  pop dx
  pop cx
  pop bx
  pop ax
  iret

touchdown:
  push ax
  mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
  out	0x20, al	; Write EOI to PIC (port 0x20)
  pop ax
  iret

line_clear:
  push ax
  mov	al, 0x20	; Load nonspecific EOI value (0x20) into register al
  out	0x20, al	; Write EOI to PIC (port 0x20)
  pop ax
  iret
