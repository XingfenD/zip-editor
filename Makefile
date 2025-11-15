# ZIP Editor Makefile

# settings of compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./utils -I./zip_seg -I./main -MMD -MP
LDFLAGS =

# target name
TARGET = zip_editor.out

SRCS = main.cpp $(wildcard utils/*.cpp) $(wildcard zip_seg/*.cpp) $(wildcard main/*.cpp)

OBJS = $(SRCS:.cpp=.o)
# dependency files
DEPS = $(OBJS:.o=.d)

# default target
all: $(TARGET)

# link target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# compile rules
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean rules
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

# rebuild
rebuild: clean all

# debug mode
debug:
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -g -O0" all

# include automatically generated dependency files
-include $(DEPS)

.PHONY: all clean rebuild debug