renderer: main.c sob.c
	clang -o renderer main.c Nano-BMP/src/nano_bmp.c sob.c

clean:
	rm output.bmp renderer