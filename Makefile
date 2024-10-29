#====================================================================
#
# (c) Borna Noureddin
# COMP 8552   British Columbia Institute of Technology
# Assignment 2
# Makefile
#
#====================================================================

ARCH := $(shell uname -s)

ifeq ($(ARCH),Darwin)
  CC = g++
  CXXFLAGS = -g --std=c++17 -isystem . -isystem glfw/include -Wc++11-narrowing
  LINKFLAGS = -L glfw/lib-macos -lglfw3 -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
  CC = g++
  CXXFLAGS = -g --std=c++17 -isystem . -isystem glfw/include
  LINKFLAGS = -L glfw/lib-win -lglfw3dll -lopengl32
endif

TARGET = c8552a2
SRCFILES = c8552qt.cpp glad.cpp

$(TARGET): $(SRCFILES)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(SRCFILES) $(LINKFLAGS)

clean:
ifeq ($(ARCH),Darwin)
	\rm -f $(TARGET)
else
	/bin/rm -f $(TARGET).exe
endif

