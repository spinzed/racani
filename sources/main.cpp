#define EXAMPLE 3

#include "examples/example1.h"
#include "examples/exampleGame.h"
#include "examples/exampleRT.h"

int main(int _, char *argv[]) {
    std::string execDirectory(argv[0], 0, std::string(argv[0]).find_last_of("\\/"));
    //example1(execDirectory);
    #if EXAMPLE == 1
    example1(execDirectory);
    #elif EXAMPLE == 2
    exampleGame(execDirectory);
    #elif EXAMPLE == 3
    exampleRT(execDirectory);
    #endif
}
