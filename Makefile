CXX = clang++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra
TARGET = game_engine_linux
SRC = $(wildcard *.cpp)
BOX2D_SRC = $(wildcard ./ThirdParty/box2d/src/collision/*.cpp) \
            $(wildcard ./ThirdParty/box2d/src/common/*.cpp) \
            $(wildcard ./ThirdParty/box2d/src/dynamics/*.cpp) \
            $(wildcard ./ThirdParty/box2d/src/rope/*.cpp)
INCLUDES = -I./ThirdParty/glm \
           -I./ThirdParty \
           -I./ThirdParty/SDL2 \
           -I./ThirdParty/SDL2_image \
           -I./ThirdParty/SDL2_mixer \
           -I./ThirdParty/SDL2_ttf \
           -I./ThirdParty/lua \
           -I./ThirdParty/LuaBridge \
           -I./ThirdParty/box2d/ \
           -I./ThirdParty/box2d/src
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -llua5.4
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRC) $(BOX2D_SRC) $(LDFLAGS)
clean:
	rm -f $(TARGET)

