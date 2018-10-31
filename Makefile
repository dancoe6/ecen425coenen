#####################################################################
# ECEn 425 Lab 4b Makefile - Conor Bodily, Daniel Coenen 10/8/2018

lab4d.bin: lab4dfinal.s
		nasm lab4dfinal.s -o lab4d.bin -l lab4d.lst

lab4dfinal.s: clib.s isr.s intrpt.s lab4d_app.s yakc.s yaks.s
		cat clib.s isr.s intrpt.s lab4d_app.s yakc.s yaks.s > lab4dfinal.s

yakc.s:	yakc.c
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

intrpt.s: intrpt.c
		cpp intrpt.c intrpt.i
		c86 -g intrpt.i intrpt.s

lab4d_app.s: lab4d_app.c
		cpp lab4d_app.c lab4d_app.i
		c86 -g lab4d_app.i lab4d_app.s

clean:
		rm lab4d.bin lab4d.lst lab4dfinal.s intrpt.s intrpt.i \
		lab4d_app.i lab4d_app.s yakc.s yakc.i 

