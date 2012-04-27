#//////////////////////////////////////////////////////////////////////////////////
#//// GENERIC  maya plugin make file
#///////////////////////////////////////////////////

#////////
#/// version 1.0
#\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#
# Include platform specific build settings
#
include ./buildconfig

.SUFFIXES: .cpp .cc .o .so .c .bundle

OUTPUT = ./

PLUGIN = curveBrushTool.$(EXT)

COMPILE  = $(CC) -c $(CFLAGS) $(INCLUDES)

.cpp.o:
	$(COMPILE) $< -o $@

plugins: $(PLUGIN)

depend:
	makedepend $(INCLUDES) *.cc *.cpp

clean:
	-rm -f *.o  $(OUTPUT)*.so


##################
# Specific Rules #
##################

SOURCES = 	brushContextCommand.cpp\
		brushContext.cpp\
		brushToolCommand.cpp\
		pluginMain.cpp

OBJS = $(SOURCES:.cpp=.o)

all: $(SOURCES) $(PLUGIN)

$(PLUGIN): $(OBJS)
	-rm -f $(OUTPUT)$@
	$(LD) -o $(OUTPUT)$@ $(OBJS) $(LIBS)

install: $(all)

	cp --dereference -rv scripts/*  /redpawFX/maya/mel/3rdParty/curveBrushTool



