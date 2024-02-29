
LINKFLAGS=-lglfw -lGL
# tutorial suggests -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi (but it works without)
COMPFLAGS=-Iinclude -Iexternal/gml -Iexternal/glad/include -Iexternal/single -Wall -Wextra -pedantic -Wno-vla
CPPC=g++ -std=c++20
CC=gcc
GLSLC=glslc
DEBUGGER=gf2
RUNNER=

folders=$(patsubst src%,output%,$(shell find src -type d))
cppobjects=$(patsubst src/%.cpp,output/%.o,$(shell find src -name "*.cpp"))
cppobjects+=output/glad.o # apparently my glfw already comes with bindings for opengl (might need glad only for windows)
shaders=$(patsubst shaders/%.glsl,output/%.spv,$(shell find shaders -name "*.glsl"))

ifndef RELEASE
	COMPFLAGS+= -ggdb -DDEBUG
	LINKFLAGS+= -ggdb
else
	COMPFLAGS+= -O3
	LINKFLAGS+= -O3
endif

# main does not have header file
output/main.o: src/main.cpp
	echo "CPPC  " $<
	$(CPPC) -c $< $(COMPFLAGS) -o $@

# ignoring the warnings specifically for base.cpp because it implements external header only libraries
output/base.o: src/base.cpp include/base.hpp
	echo "CPPC  " $<
	$(CPPC) -c $< $(COMPFLAGS) -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-variable -o $@

output/glad.o: external/glad/src/glad.c
	$(CC) -c -Iexternal/glad/include $< -o $@

output/%.o: src/%.cpp include/%.hpp
	echo "CPPC  " $<
	$(CPPC) -c $< $(COMPFLAGS) -o $@

output/%:
	echo "MKDIR " $@
	mkdir -p $@

output/mcclone: $(folders) $(cppobjects)
	echo "LINK   object files"
	$(CPPC) $(LINKFLAGS) $(cppobjects) $(cobjects) -o output/mcclone

run: output/mcclone
	echo "RUN   " $<
	$(RUNNER) output/mcclone

debug: output/mcclone
	echo "DEBUG  " $<
	$(DEBUGGER) output/mcclone

.SILENT:

todos:
	sh ../todos.sh

# ignoring base.cpp, shaders
clean_o:
	echo 'RM     object files'
	rm `find output -name "*.o" | grep -v base.o` || true

clean_all:
	echo 'RM     output/*'
	rm -rf output/*
