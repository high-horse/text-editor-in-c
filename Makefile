CC = gcc
CSTD = c2x
CFLAGS = -std=$(CSTD) -Wall -Wextra -Werror

TARGET = bin/main
SRC = src/main.c src/libs/*
BIN_DIR = bin

all: $(TARGET)

$(TARGET): $(SRC) | $(BIN_DIR)
	$(CC)  $(CFLAGS) -o $(TARGET) $(SRC)  -lraylib -static

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: all
	@./$(TARGET) $(ARGS)

clean:
	rm -f $(TARGET)

	
build:
	gcc -std=c2x -Wall -Wextra -Werror -o bin/main src/main.c src/libs/* -lraylib
	
	
build-static-linux:
	gcc -std=c2x -Wall -Wextra -Werror \
	-o build/linux/text \
	src/main.c src/libs/*.c \
	-Wl,-Bstatic -lraylib -Wl,-Bdynamic \
	-lm -ldl -lpthread -lrt -lX11 -lGL
	
	
build-static-macos:
	clang -std=c2x -Wall -Wextra -Werror \
    -o build/macos/text \
    src/main.c src/libs/*.c \
    -lraylib \
    -framework OpenGL \
    -framework Cocoa \
    -framework IOKit \
    -framework CoreVideo \
    -lm -lpthread
	# gcc -std=c2x -Wall -Wextra -Werror \
	# -o build/macos/text \
	# src/main.c src/libs/*.c \
	# -Wl,-Bstatic -lraylib -Wl,-Bdynamic \
	# -lm -ldl -lpthread -lrt -lX11 -lGL
	
	
	
font2header:
	xxd -i src/resources/Roboto/static/Roboto-Regular.ttf > font_data.h
	
re: clean all

.PHONY: all clean run re