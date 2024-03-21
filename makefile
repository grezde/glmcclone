
LINKFLAGS=-lglfw -lGL
# tutorial suggests -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi (but it works without)
COMPFLAGS=-Iinclude -Iexternal/gml -Iexternal/glad/include -Iexternal/single -Wall -Wextra -pedantic -Wno-vla
CPPC=g++ -std=c++20
CC=gcc
GLSLC=glslc
DEBUGGER=gf2
RUNNER=
EXE=mcclone
OUTDIR=output

folders=$(patsubst src%,output%,$(shell find src -type d))
cppobjects=$(patsubst src/%.cpp,output/%.o,$(shell find src -name "*.cpp"))
cppobjects+=output/glad.o
shaders=$(patsubst shaders/%.glsl,output/%.spv,$(shell find shaders -name "*.glsl"))

ifndef RELEASE
	COMPFLAGS+= -ggdb -DDEBUG
	LINKFLAGS+= -ggdb
else
	COMPFLAGS+= -O3
	LINKFLAGS+= -O3
endif

# main does not have header file
$(OUTDIR)/main.o: src/main.cpp
	echo "CPPC  " $<
	$(CPPC) -c $< $(COMPFLAGS) -o $@

# ignoring the warnings specifically for base.cpp because it implements external header only libraries
$(OUTDIR)/base.o: src/base.cpp include/base.hpp
	echo "CPPC  " $<
	$(CPPC) -c $< $(COMPFLAGS) -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-variable -o $@

$(OUTDIR)/glad.o: external/glad/src/glad.c
	$(CC) -c -Iexternal/glad/include $< -o $@

$(OUTDIR)/datatool.o: src2/datatool.cpp
	echo "CPPC  " $<
	$(CPPC) -c $< $(COMPFLAGS) -o $@

$(OUTDIR)/%.o: src/%.cpp include/%.hpp
	echo "CPPC  " $<
	$(CPPC) -c $< $(COMPFLAGS) -o $@

$(OUTDIR)/%: $(OUTDIR)
	echo "MKDIR " $@
	mkdir -p $@

$(OUTDIR):
	echo "MKDIR " $@
	mkdir -p $@


$(OUTDIR)/$(EXE): $(folders) $(cppobjects)
	echo "LINK   $(EXE)"
	$(CPPC) $(LINKFLAGS) $(cppobjects) -o $@

$(OUTDIR)/datatool: $(OUTDIR)/datatool.o $(OUTDIR)/data.o $(OUTDIR)/base.o
	echo "LINK   datatool" 
	$(CPPC) $(LINKFLAGS) $^ -o $@

run: $(OUTDIR)/$(EXE)
	echo "RUN    $(EXE)"
	$(RUNNER) $(OUTDIR)/$(EXE)

debug: $(OUTDIR)/$(EXE)
	echo "DEBUG  " $<
	$(DEBUGGER) $(OUTDIR)/$(EXE)

all: $(OUTDIR)/$(EXE) $(OUTDIR)/datatool


.SILENT:

todos:
	sh ../todos.sh

# ignoring base.cpp, shaders
clean_o:
	echo 'RM     object files'
	rm `find $(OUTDIR) -name "*.o" | grep -v base.o` || true

clean_all:
	echo 'RM     $(OUTDIR)/*'
	rm -rf $(OUTDIR)/*
