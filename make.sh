time gcc -o blobules cb_lib/cb_string.c  simple_renderer.c game.c main.c -I/usr/include/freetype2 -I/usr/include/libpng16 -lfreetype -lglfw -lGL -lGLEW  && ./blobules ; rm blobules
