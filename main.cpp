#include <iostream>
#include "Engine.h"
#include "ThirdParty/SDL2/SDL.h"
#include "lua.hpp"
#include "LuaBridge.h"
#include <chrono>
#include "box2d/box2d.h"

//linux

int main(int argc, char* argv[]) {
    //auto start = std::chrono::high_resolution_clock::now();
    Engine engine;
    engine.GameLoop();

    //auto end = std::chrono::high_resolution_clock::now();
    //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    //std::cout << "Execution time: " << duration.count() << " ms\n";


    return 0;
}

