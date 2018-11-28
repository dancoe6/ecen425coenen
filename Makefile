#####################################################################
# ECEn 425 Lab Makefile - Conor Bodily, Daniel Coenen 10/8/2018

lab7.bin: labfinal.s
		nasm labfinal.s -o lab7.bin -l lab7.lst

labfinal.s: clib.s isr.s intrpt.s lab7_app.s yakc.s yaks.s
		cat clib.s isr.s intrpt.s lab7_app.s yakc.s yaks.s > labfinal.s

yakc.s:	yakc.c
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

intrpt.s: intrpt.c
		cpp intrpt.c intrpt.i
		c86 -g intrpt.i intrpt.s

lab7_app.s: lab7_app.c
		cpp lab7_app.c lab7_app.i
		c86 -g lab7_app.i lab7_app.s

clean:
		rm lab7.bin lab7.lst labfinal.s intrpt.s intrpt.i \
		lab7_app.i lab7_app.s yakc.s yakc.i
