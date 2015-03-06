renderer: main.c jobj.c
	clang -o renderer main.c Nano-BMP/src/nano_bmp.c jobj.c

clean:
	rm output.bmp renderer