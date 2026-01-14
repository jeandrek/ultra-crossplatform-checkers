#!/bin/sh
powerpc-apple-macos-as AGL__stub.s -o AGL__stub.o
powerpc-apple-macos-ld --shared --no-check-sections -bexport:syms.exp -o OpenGLLibrary.o  AGL__stub.o
powerpc-apple-macos-ar r libOpenGLLibrary.a OpenGLLibrary.o

powerpc-apple-macos-as glu__stub.s -o glu__stub.o
powerpc-apple-macos-ld --shared --no-check-sections -bexport:syms-glu.exp -o OpenGLUtility.o  glu__stub.o
powerpc-apple-macos-ar r libOpenGLUtility.a OpenGLUtility.o
