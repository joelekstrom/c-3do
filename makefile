DIR = ./bin
CC = clang

$(DIR)/renderer: $(DIR)/main.o $(DIR)/sob.o $(DIR)/geometry.o $(DIR)/nano-bmp.o
	$(CC) -o $(DIR)/renderer $(DIR)/main.o $(DIR)/sob.o $(DIR)/geometry.o $(DIR)/nano-bmp.o

$(DIR)/main.o: main.c $(DIR)
	$(CC) -o $(DIR)/main.o -c main.c

$(DIR)/sob.o: sob.h sob.c $(DIE)
	$(CC) -o $(DIR)/sob.o -c sob.c

$(DIR)/geometry.o: geometry.h geometry.c $(DIR)
	$(CC) -o $(DIR)/geometry.o -c geometry.c

$(DIR)/nano-bmp.o: $(DIR)
	$(CC) -o $(DIR)/nano-bmp.o -c Nano-BMP/src/nano_bmp.c

$(DIR):
	mkdir $(DIR)

clean:
	rm -rf $(DIR)
