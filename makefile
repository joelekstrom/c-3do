DIR = ./bin
CC = clang
CFLAGS = -Wall -Wextra

$(DIR)/renderer: $(DIR)/main.o $(DIR)/sob.o $(DIR)/geometry.o $(DIR)/nano-bmp.o
	$(CC) $(CFLAGS) -o $(DIR)/renderer $(DIR)/main.o $(DIR)/sob.o $(DIR)/geometry.o $(DIR)/nano-bmp.o

$(DIR)/main.o: main.c $(DIR)
	$(CC) $(CFLAGS) -o $(DIR)/main.o -c main.c

$(DIR)/sob.o: sob.h sob.c $(DIR)
	$(CC) $(CFLAGS) -o $(DIR)/sob.o -c sob.c

$(DIR)/geometry.o: geometry.h geometry.c $(DIR)
	$(CC) $(CFLAGS) -o $(DIR)/geometry.o -c geometry.c

$(DIR)/nano-bmp.o: $(DIR)
	$(CC) $(CFLAGS) -o $(DIR)/nano-bmp.o -c Nano-BMP/src/nano_bmp.c

$(DIR):
	mkdir $(DIR)

clean:
	rm -rf $(DIR)
