BIN = ./bin
SRC = ./src
LIB = ./lib
CC = clang
CFLAGS = -Wall -Wextra

$(BIN)/renderer: $(BIN)/main.o $(BIN)/sob.o $(BIN)/geometry.o $(BIN)/nano-bmp.o $(BIN)/graphics_context.o
	$(CC) $(CFLAGS) -o $(BIN)/renderer $(BIN)/main.o $(BIN)/sob.o $(BIN)/geometry.o $(BIN)/nano-bmp.o $(BIN)/graphics_context.o

$(BIN)/main.o: $(BIN) $(SRC)/main.c $(SRC)/geometry.h $(SRC)/sob.h $(SRC)/graphics_context.h
	$(CC) $(CFLAGS) -o $(BIN)/main.o -c $(SRC)/main.c

$(BIN)/sob.o: $(BIN) $(SRC)/sob.h $(SRC)/sob.c $(SRC)/geometry.h
	$(CC) $(CFLAGS) -o $(BIN)/sob.o -c $(SRC)/sob.c

$(BIN)/geometry.o: $(BIN) $(SRC)/geometry.h $(SRC)/geometry.c
	$(CC) $(CFLAGS) -o $(BIN)/geometry.o -c $(SRC)/geometry.c

$(BIN)/nano-bmp.o: $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/nano-bmp.o -c $(LIB)/nano-bmp/src/nano_bmp.c -I$(LIB)/nano-bmp/include/

$(BIN)/graphics_context.o: $(BIN) $(SRC)/graphics_context.c $(SRC)/graphics_context.h $(SRC)/geometry.h
	$(CC) $(CFLAGS) -o $(BIN)/graphics_context.o -c $(SRC)/graphics_context.c

$(BIN):
	mkdir $(BIN)

clean:
	rm -rf $(BIN)
