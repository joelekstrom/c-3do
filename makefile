renderer:
	clang -o renderer main.c Nano-BMP/src/nano_bmp.c

clean:
	rm output.bmp
	rm renderer