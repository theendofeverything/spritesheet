CFLAGS = -Wall -Wextra -pedantic -std=c11
CFLAGS += `pkg-config --cflags sdl2`
LDLIBS  = `pkg-config --libs sdl2`
CFLAGS += `pkg-config --cflags SDL2_ttf`
LDLIBS += `pkg-config --libs SDL2_ttf`
CFLAGS += `pkg-config --cflags SDL2_image`
LDLIBS += `pkg-config --libs SDL2_image`

SRC = main

.PHONY: show-tags
show-tags: tags
	@echo -e \n\# $(SRC)\n
	@ctags --c-kinds=+l					--sort=no -x $(SRC).c
	@echo -e \n\# LIBS\n
	@ctags --c-kinds=+l -L headers.txt 	--sort=no -x

.PHONY: tags
tags: $(SRC).c parse-headers.exe
	@$(CC) $(CFLAGS) $< -M > headers-M.txt
	@./parse-headers.exe M
	@ctags --c-kinds=+l -L headers.txt $(SRC).c

.PHONY: lib-tags
lib-tags: $(SRC).c parse-headers.exe
	@$(CC) $(CFLAGS) $< -M > headers-M.txt
	@./parse-headers.exe
	@ctags -f lib-tags --c-kinds=+p -L headers.txt

parse-headers.exe: parse-headers.c
	$(CC) -Wall $< -o $@
