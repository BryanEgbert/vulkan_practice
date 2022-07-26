#include <iostream>
#include <cstdlib>
#include <stdexcept>

#include "triangleEngine.hpp"

int main(int, char**) 
{
    triangle::Engine engine;

    try 
    {
        engine.run();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
