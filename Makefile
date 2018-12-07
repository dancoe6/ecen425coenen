#####################################################################
# ECEn 425 Lab Makefile - Conor Bodily, Daniel Coenen 10/8/2018

lab8.bin: labfinal.s
		nasm labfinal.s -o lab8.bin -l lab8.lst

labfinal.s: clib.s isr.s intrpt.s lab8_app.s simptris.s yakc.s yaks.s
		cat clib.s isr.s intrpt.s lab8_app.s simptris.s yakc.s yaks.s > labfinal.s

yakc.s:	yakc.c
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

intrpt.s: intrpt.c
		cpp intrpt.c intrpt.i
		c86 -g intrpt.i intrpt.s

lab8_app.s: lab8_app.c
		cpp lab8_app.c lab8_app.i
		c86 -g lab8_app.i lab8_app.s

clean:
		rm lab8.bin lab8.lst labfinal.s intrpt.s intrpt.i \
		lab8_app.i lab8_app.s yakc.s yakc.i
