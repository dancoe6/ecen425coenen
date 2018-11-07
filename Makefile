#####################################################################
# ECEn 425 Lab 4b Makefile - Conor Bodily, Daniel Coenen 10/8/2018

lab5.bin: lab5final.s
		nasm lab5final.s -o lab5.bin -l lab5.lst

lab5final.s: clib.s isr.s intrpt.s lab5_app.s yakc.s yaks.s
		cat clib.s isr.s intrpt.s lab5_app.s yakc.s yaks.s > lab5final.s

yakc.s:	yakc.c
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

intrpt.s: intrpt.c
		cpp intrpt.c intrpt.i
		c86 -g intrpt.i intrpt.s

lab5_app.s: lab5_app.c
		cpp lab5_app.c lab5_app.i
		c86 -g lab5_app.i lab5_app.s

clean:
		rm lab5.bin lab5.lst lab5final.s intrpt.s intrpt.i \
		lab5_app.i lab5_app.s yakc.s yakc.i
