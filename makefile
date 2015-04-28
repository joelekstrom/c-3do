BIN = ./bin
SRC = ./src
LIB = ./lib
CFLAGS = -Wall -Wextra

ifndef CC
    CC = gcc
endif

$(BIN)/renderer: $(BIN)/main.o $(BIN)/obj.o $(BIN)/geometry.o $(BIN)/nano-bmp.o $(BIN)/graphics_context.o
	$(CC) $(CFLAGS) -o $(BIN)/renderer $(BIN)/main.o $(BIN)/obj.o $(BIN)/geometry.o $(BIN)/nano-bmp.o $(BIN)/graphics_context.o

$(BIN)/main.o: $(BIN) $(SRC)/main.c $(SRC)/geometry.h $(SRC)/obj.h $(SRC)/graphics_context.h
	$(CC) $(CFLAGS) -o $(BIN)/main.o -c $(SRC)/main.c

$(BIN)/obj.o: $(BIN) $(SRC)/obj.h $(SRC)/obj.c $(SRC)/geometry.h
	$(CC) $(CFLAGS) -o $(BIN)/obj.o -c $(SRC)/obj.c

$(BIN)/geometry.o: $(BIN) $(SRC)/geometry.h $(SRC)/geometry.c
	$(CC) $(CFLAGS) -o $(BIN)/geometry.o -c $(SRC)/geometry.c

$(BIN)/nano-bmp.o: $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/nano-bmp.o -c $(LIB)/Nano-BMP/src/nano_bmp.c -I$(LIB)/Nano-BMP/include/

$(BIN)/graphics_context.o: $(BIN) $(SRC)/graphics_context.c $(SRC)/graphics_context.h $(SRC)/geometry.h
	$(CC) $(CFLAGS) -o $(BIN)/graphics_context.o -c $(SRC)/graphics_context.c

$(BIN):
	mkdir $(BIN)

clean:
	rm -rf $(BIN)
