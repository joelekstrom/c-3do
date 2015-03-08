renderer: main.c sob.c geometry.c
	clang -o renderer main.c Nano-BMP/src/nano_bmp.c sob.c geometry.c

clean:
	rm output.bmp renderer