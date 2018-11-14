#####################################################################
# ECEn 425 Lab 4b Makefile - Conor Bodily, Daniel Coenen 10/8/2018

lab6.bin: lab6final.s
		nasm lab6final.s -o lab6.bin -l lab6.lst

lab6final.s: clib.s isr.s intrpt.s lab6_app.s yakc.s yaks.s
		cat clib.s isr.s intrpt.s lab6_app.s yakc.s yaks.s > lab6final.s

yakc.s:	yakc.c
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

intrpt.s: intrpt.c
		cpp intrpt.c intrpt.i
		c86 -g intrpt.i intrpt.s

lab6_app.s: lab6_app.c
		cpp lab6_app.c lab6_app.i
		c86 -g lab6_app.i lab6_app.s

clean:
		rm lab6.bin lab6.lst lab6final.s intrpt.s intrpt.i \
		lab6_app.i lab6_app.s yakc.s yakc.i
