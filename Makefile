#####################################################################
# ECEn 425 Lab 4b Makefile - Conor Bodily, Daniel Coenen 10/8/2018

lab3.bin: lab4bfinal.s
		nasm lab4bfinal.s -o lab4b.bin -l lab4b.lst

lab4bfinal.s: clib.s isr.s intrpt.s lab4b_app.s yakc.s yaks.s
		cat clib.s isr.s intrpt.s lab4b_app.s yakc.s yaks.s > lab4bfinal.s

yakc.s:	yakc.c
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

intrpt.s: intrpt.c
		cpp intrpt.c intrpt.i
		c86 -g intrpt.i intrpt.s

lab4b_app.s: lab4b_app.c
		cpp lab4b_app.c lab4b_app.i
		c86 -g lab4b_app.i lab4b_app.s

clean:
		rm lab4b.bin lab4b.lst lab4bfinal.s intrpt.s intrpt.i \
		lab4b_app.i lab4b_app.s yakc.s yakc.i 

