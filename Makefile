# Compiler and Compiler Flags
CXX=clang++
CXXFLAGS=-Wall -std=c++17 -Iheaders -Ilibs/glm/ -Ilibs/SDL2/include -Ilibs/SDL2_image/include -Ilibs/SDL2_mixer/include -Ilibs/SDL2_ttf/include -ILua -ILuaBridge -ILuaBridge/detail -O3 
LDFLAGS=-Llibs/SDL2/lib/x64 -lSDL2 -lSDL2main -Llibs/SDL2_image/lib/x64 -lSDL2_image -Llibs/SDL2_ttf/lib/x64 -lSDL2_ttf -Llibs/SDL2_mixer/lib/x64 -lSDL2_mixer -llua5.4 -Wl 
# LDLIBS := -llua5.4

# Target executable name
TARGET=game_engine_linux

# Source files
SRC=my_game_engine.cpp MainHelper.cpp Template.cpp Actor.cpp EngineUtils.cpp Scene.cpp Renderer.cpp TextDB.cpp AudioDB.cpp ImageDB.cpp Scene.cpp Input.cpp Camera.cpp # Add more source files here as needed

# Automatically find all header files in the headers directory
HEADERS=$(wildcard headers/*.h)

# Object files
OBJ=$(SRC:.cpp=.o)

# Default rule
all: $(TARGET)

# Rule for building the final executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $^

# Rule for building object files
# Include header dependencies
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJ) $(TARGET)
