time gcc -O3 -o blobules cb_lib/cb_string.c  simple_renderer.c game.c ui.c main.c -I/usr/include/freetype2 -I/usr/include/libpng16 -lfreetype -lglfw -lGL -lGLEW  -lm && ./blobules ; rm blobules
