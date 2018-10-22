#####################################################################
# ECEn 425 Lab 4b Makefile - Conor Bodily, Daniel Coenen 10/8/2018

lab3.bin: lab4cfinal.s
		nasm lab4cfinal.s -o lab4c.bin -l lab4c.lst

lab4cfinal.s: clib.s isr.s intrpt.s lab4c_app.s yakc.s yaks.s
		cat clib.s isr.s intrpt.s lab4c_app.s yakc.s yaks.s > lab4cfinal.s

yakc.s:	yakc.c
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

intrpt.s: intrpt.c
		cpp intrpt.c intrpt.i
		c86 -g intrpt.i intrpt.s

lab4c_app.s: lab4c_app.c
		cpp lab4b_app.c lab4c_app.i
		c86 -g lab4c_app.i lab4c_app.s

clean:
		rm lab4c.bin lab4c.lst lab4cfinal.s intrpt.s intrpt.i \
		lab4c_app.i lab4c_app.s yakc.s yakc.i 

