pushd src && time gcc -O1 -pg -o blobules cb_lib/cb_string.c  renderer.c game.c ui.c main.c -I/usr/include/SDL2 -lSDL2 -lSDL2_mixer -lglfw -lGL -lGLEW  -lm && ./blobules && rm ./blobules && popd
