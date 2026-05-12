#!/bin/bash
rm -f *.o *.so gltestapp
gcc -c AutumnGL_Eng.c AutumnGL_Math.c -fPIC $(pkg-config --cflags sdl2 freetype2)
gcc -shared -o libautumngl.so AutumnGL_Eng.o AutumnGL_Math.o $(pkg-config --libs sdl2 freetype2) -lGLESv2 -lEGL -lm
gcc gltest.c -o gltestapp -L. -lautumngl $(pkg-config --cflags --libs sdl2 freetype2) -lGLESv2 -lEGL -lm
